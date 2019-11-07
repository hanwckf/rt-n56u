// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "wireguard_config.h"
#include "wireguard.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <stdarg.h>

class WgFileParser {
public:
  WgFileParser(WireguardProcessor *wg, DnsResolver *resolver) : wg_(wg), dns_resolver_(resolver) {}
  bool ParseFlag(const char *group, const char *key, char *value);
  WireguardProcessor *wg_;
  
  void FinishGroup();
  struct Peer {
    WgPublicKey pub;
    uint8 psk[32];
  };
  Peer pi_;
  WgPeer *peer_ = NULL;
  DnsResolver *dns_resolver_;
  bool had_interface_ = false;
};

static bool ParseBoolean(const char *str, bool *value) {
  if (_stricmp(str, "true") == 0 ||
      _stricmp(str, "yes") == 0 ||
      _stricmp(str, "1") == 0 ||
      _stricmp(str, "on") == 0) {
    *value = true;
    return true;
  }
  if (_stricmp(str, "false") == 0 ||
      _stricmp(str, "no") == 0 ||
      _stricmp(str, "0") == 0 ||
      _stricmp(str, "off") == 0) {
    *value = false;
    return true;
  }
  return false;
}

static int ParseFeature(const char *str) {
  size_t len = strlen(str);
  int what = WG_BOOLEAN_FEATURE_WANTS;
  if (len > 0) {
    if (str[len - 1] == '?')
      what = WG_BOOLEAN_FEATURE_SUPPORTS, len--;
    else if (str[len - 1] == '!')
      what = WG_BOOLEAN_FEATURE_ENFORCES, len--;
  }
  if (WITH_SHORT_MAC && len == 5 && memcmp(str, "mac64", 5) == 0)
    return what + WG_FEATURE_ID_SHORT_MAC * 16;
  if (WITH_PACKET_COMPRESSION && len == 5 && memcmp(str, "ipzip", 5) == 0)
    return what + WG_FEATURE_ID_IPZIP * 16;
  if (len == 10 && memcmp(str, "hybrid_tcp", 10) == 0)
    return what + WG_FEATURE_HYBRID_TCP * 16;
  if (WITH_SHORT_HEADERS && len == 10 && memcmp(str, "skip_keyid", 10) == 0)
    return what + WG_FEATURE_ID_SKIP_KEYID_IN * 16 + 1 * 4;
  if (WITH_SHORT_HEADERS && len == 12 && memcmp(str, "short_header", 12) == 0)
    return what + WG_FEATURE_ID_SHORT_HEADER * 16;
  if (WITH_SHORT_HEADERS && len == 13 && memcmp(str, "skip_keyid_in", 13) == 0)
    return what + WG_FEATURE_ID_SKIP_KEYID_IN * 16;
  if (WITH_SHORT_HEADERS && len == 14 && memcmp(str, "skip_keyid_out", 14) == 0)
    return what + WG_FEATURE_ID_SKIP_KEYID_OUT * 16;
  return -1;
}

static int ParseCipherSuite(const char *cipher) {
  if (!strcmp(cipher, "chacha20-poly1305"))
    return EXT_CIPHER_SUITE_CHACHA20POLY1305;
  if (!strcmp(cipher, "aes128-gcm"))
    return EXT_CIPHER_SUITE_AES128_GCM;
  if (!strcmp(cipher, "aes256-gcm"))
    return EXT_CIPHER_SUITE_AES256_GCM;
  if (!strcmp(cipher, "none"))
    return EXT_CIPHER_SUITE_NONE_POLY1305;
  return -1;
}

void WgFileParser::FinishGroup() {
  if (peer_) {
    peer_->SetPublicKey(pi_.pub);
    peer_ = NULL;
  }
}

bool WgFileParser::ParseFlag(const char *group, const char *key, char *value) {
  uint8 binkey[32];
  WgCidrAddr addr;
  IpAddr sin;
  std::vector<char*> ss;
  bool ciphermode = false;
  
  if (strcmp(group, "[Interface]") == 0) {
    if (key == NULL) return true;
    if (wg_->dev().plugin() && wg_->dev().plugin()->OnUnknownInterfaceSetting(key, value)) {
      // nothing here
    } else if (strcmp(key, "PrivateKey") == 0) {
      if (!ParseBase64Key(value, binkey))
        return false;
      had_interface_ = true;
      wg_->dev().SetPrivateKey(binkey);
    } else if (strcmp(key, "ListenPort") == 0) {
      wg_->SetListenPort(atoi(value));
    } else if (strcmp(key, "ListenPortTCP") == 0) {
      wg_->SetListenPortTcp(atoi(value));
    } else if (strcmp(key, "Address") == 0) {
      SplitString(value, ',', &ss);
      for (size_t i = 0; i < ss.size(); i++) {
        if (!ParseCidrAddr(ss[i], &addr))
          return false;
        if (!wg_->SetTunAddress(addr)) {
          RERROR("Multiple Address not allowed");
          return false;
        }
      }
    } else if (strcmp(key, "MTU") == 0) {
      wg_->SetMtu(atoi(value));
    } else if (strcmp(key, "Table") == 0) {
      bool mode;
      if (!strcmp(value, "off")) {
        mode = false;
      } else if (!strcmp(value, "auto")) {
        mode = true;
      } else {
        goto err;
      }
      wg_->SetAddRoutesMode(mode);
    } else if (strcmp(key, "DNS") == 0) {
      SplitString(value, ',', &ss);
      for (size_t i = 0; i < ss.size(); i++) {
        if (!ParseSockaddrInWithoutPort(ss[i], &sin, dns_resolver_, kParseSockaddrDontDoNAT64))
          return false;
        wg_->AddDnsServer(sin);
      }
    } else if (strcmp(key, "BlockDNS") == 0) {
      bool v;
      if (!ParseBoolean(value, &v))
        goto err;
      wg_->SetDnsBlocking(v);
    } else if (strcmp(key, "BlockInternet") == 0) {
      uint8 v = kBlockInternet_Default;
      SplitString(value, ',', &ss);
      for (size_t i = 0; i < ss.size(); i++) {
        const char *s = ss[i];
        if (strcmp(s, "route") == 0) {
          if (v & 128) v = 0;
          v |= kBlockInternet_Route;
        } else if (strcmp(s, "firewall") == 0 || strcmp(s, "on") == 0) {
          if (v & 128) v = 0;
          v |= kBlockInternet_Firewall;
        } else if (strcmp(s, "off") == 0) {
          v = 0;
        } else if (strcmp(s, "default") == 0) {
          v = kBlockInternet_Default;
        } else if (strcmp(s, "persist") == 0) {
          if (v & 128) v = 0;
          v |= kBlockInternet_BlockOnDisconnect;
        } else if (strcmp(s, "allow_local") == 0) {
          if (v & 128) v = 0;
          v |= kBlockInternet_AllowLocalNetworks;
        } else {
          RERROR("Unknown mode in BlockInternet: %s", ss[i]);
        }
      }
      
      wg_->SetInternetBlocking((InternetBlockState)v);
    } else if (WITH_HEADER_OBFUSCATION && strcmp(key, "ObfuscateKey") == 0) {
      wg_->dev().packet_obfuscator().SetKey((uint8*)value, strlen(value));
    } else if (WITH_HEADER_OBFUSCATION && strcmp(key, "ObfuscateTCP") == 0) {
      bool flag;
      int v = 1;
      if (ParseBoolean(value, &flag)) {
        v = flag;
      } else if (strcmp(value, "tls-firefox") == 0) {
        v = 2;
      } else if (strcmp(value, "tls-chrome") == 0) {
        v = 3;
      } else if (*value != 0) {
        RERROR("Unknown mode in ObfuscateTCP: %s", value);
      }
      wg_->dev().packet_obfuscator().set_obfuscate_tcp(v);

    } else if (strcmp(key, "PostUp") == 0) {
      wg_->prepost().post_up.emplace_back(value);
    } else if (strcmp(key, "PostDown") == 0) {
      wg_->prepost().post_down.emplace_back(value);
    } else if (strcmp(key, "PreUp") == 0) {
      wg_->prepost().pre_up.emplace_back(value);
    } else if (strcmp(key, "PreDown") == 0) {
      wg_->prepost().pre_down.emplace_back(value);
    } else if (strcmp(key, "ExcludedIPs") == 0) {
      SplitString(value, ',', &ss);
      for (size_t i = 0; i < ss.size(); i++) {
        if (!ParseCidrAddr(ss[i], &addr))
          return false;
        wg_->AddExcludedIp(addr);
      }
    } else {
      goto err;
    }
  } else if (strcmp(group, "[Peer]") == 0) {
    if (key == NULL) {
      if (!had_interface_) {
        RERROR("Missing [Interface].PrivateKey.");
        return false;
      }
      FinishGroup();
      peer_ = wg_->dev().AddPeer();
      memset(&pi_, 0, sizeof(pi_));
      return true;
    }

    if (wg_->dev().plugin() && wg_->dev().plugin()->OnUnknownPeerSetting(peer_, key, value)) {
      // nothing here
    } else if (strcmp(key, "PublicKey") == 0) {
      if (!ParseBase64Key(value, pi_.pub.bytes))
        return false;
    } else if (strcmp(key, "PresharedKey") == 0) {
      if (!ParseBase64Key(value, pi_.psk))
        return false;
      peer_->SetPresharedKey(pi_.psk);
    } else if (strcmp(key, "AllowedIPs") == 0) {
      SplitString(value, ',', &ss);
      for (size_t i = 0; i < ss.size(); i++) {
        if (!ParseCidrAddr(ss[i], &addr))
          return false;
        if (!peer_->AddIp(addr))
          return false;
      }
    } else if (strcmp(key, "Endpoint") == 0) {
      int proto = kPacketProtocolUdp;
      if (strncmp(value, "tcp://", 6) == 0) {
        value += 6;
        proto = kPacketProtocolTcp;
      }
      if (!ParseSockaddrInWithPort(value, &sin, dns_resolver_))
        return false;
      peer_->SetEndpoint(proto, sin);
    } else if (strcmp(key, "PersistentKeepalive") == 0) {
      if (!peer_->SetPersistentKeepalive(atoi(value)))
        return false;
    } else if (strcmp(key, "AllowMulticast") == 0) {
      bool b;
      if (!ParseBoolean(value, &b))
        return false;
      peer_->SetAllowMulticast(b);
    } else if (strcmp(key, "Features") == 0) {
      SplitString(value, ',', &ss);
      for (size_t i = 0; i < ss.size(); i++) {
        int v = ParseFeature(ss[i]);
        if (v < 0)
          return false;
        for (;; v += 12) {
          peer_->SetFeature(v >> 4, v & 3);
          if (!(v & 12))
            break;
        }
      }
    } else if (strcmp(key, "Ciphers") == 0 || (ciphermode = true, strcmp(key, "Ciphers!") == 0)) {
      SplitString(value, ',', &ss);
      peer_->SetCipherPrio(ciphermode);
      for (size_t i = 0; i < ss.size(); i++) {
        int v = ParseCipherSuite(ss[i]);
        if (v < 0 || !peer_->AddCipher(v))
          return false;
      }
    } else {
      goto err;
    }
  } else {
  err:
    return false;
  }
  return true;
}

static bool ContainsNonAsciiCharacter(const char *buf, size_t size) {
  for (size_t i = 0; i < size; i++) {
    uint8 c = buf[i];
    if (c < 32 && ((1 << c) & (1 << '\n' | 1 << '\r' | 1 << '\t')) == 0)
      return true;
  }
  return false;
}

bool ParseWireGuardConfigString(WireguardProcessor *wg, const char *bufin, size_t buf_size, DnsResolver *dns_resolver) {
  char group[32] = {0};
  std::string buf2 = bufin;
  char *buf = &buf2[0];
  
  WgFileParser file_parser(wg, dns_resolver);
  
  const char *buf_end = buf + buf_size;
  for (;;) {
    char *nl = (char*)memchr(buf, '\n', buf_end - buf);
    if (nl)
      *nl = 0;
    
    size_t len = (nl ? nl : buf_end) - buf;	
    
    if (ContainsNonAsciiCharacter(buf, len)) {
      RERROR("File is not a config file");
      return false;
    }
    
    char *comment = (char*)memchr(buf, '#', len);
    if (comment) {
      len = comment - buf;
      *comment = '\0';
    }
    while (len && is_space(buf[len - 1]))
      buf[--len] = 0;
    
    if (buf[0] == '[') {
      if (len < sizeof(group)) {
        memcpy(group, buf, len + 1);
        if (!file_parser.ParseFlag(group, NULL, NULL)) {
          RERROR("Error parsing %s", group);
          return false;
        }
      }
    } else if (buf[0] != '\0') {
      char *sep = strchr(buf, '=');
      if (!sep) {
        RERROR("Missing = on line: %s", buf);
        return false;
      }
      char *sepe = sep;
      while (sepe > buf && is_space(sepe[-1]))
        sepe--;
      *sepe = 0;
      
      // trim space after =
      do sep++; while (is_space(*sep));
      
      if (!file_parser.ParseFlag(group, buf, sep)) {
        RERROR("Error parsing %s.%s = %s", group, buf, sep);
        return false;
      }
    }
    if (!nl)
      break;
    buf = nl + 1;
  }
  file_parser.FinishGroup();

  // Let plugin do any final processing
  if (wg->dev().plugin() && !wg->dev().plugin()->OnAfterSettingsParsed())
    return false;

  return true;
}

static bool LoadFileWithMaximumSize(const char *filename, std::string *result, size_t max_size) {
  FILE *f = fopen(filename, "rb");
  if (!f) return false;
  fseek(f, 0, SEEK_END);
  size_t n;
  long x = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (x < 0 || x > max_size) goto error;
  result->resize((size_t)x);
  n = fread(&(*result)[0], 1, x, f);
  if (n != x) goto error;
  fclose(f);
  return true;
error:
  fclose(f);
  return false;
}


bool ParseWireGuardConfigFile(WireguardProcessor *wg, const char *filename, DnsResolver *dns_resolver) {
  std::string temp;
  RINFO("Loading file: %s", filename);
  if (!LoadFileWithMaximumSize(filename, &temp, 1024 * 1024)) {
    RERROR("Unable to open: %s", filename);
    return false;
  }
  return ParseWireGuardConfigString(wg, &temp[0], temp.size(), dns_resolver);
}



static void CmsgAppendFmt(std::string *result, const char *fmt, ...) {
  va_list va;
  char buf[256];
  va_start(va, fmt);
  vsnprintf(buf, sizeof(buf), fmt, va);
  (*result) += buf;
  (*result) += '\n';
  va_end(va);
}

static void CmsgAppendHex(std::string *result, const char *key, const void *data, size_t data_size) {
  char *tmp = (char*)alloca(data_size * 2 + 2);
  PrintHexString(data, data_size, tmp + 1);
  tmp[0] = '=';
  tmp[data_size * 2 + 1] = '\n';
  (*result) += key;
  result->append(tmp, data_size * 2 + 2);
}

void WgConfig::HandleConfigurationProtocolGet(WireguardProcessor *proc, std::string *result) {
  char buf[kSizeOfAddress];
  
  CmsgAppendHex(result, "private_key", proc->dev_.s_priv_, sizeof(proc->dev_.s_priv_));
  if (proc->listen_port_)
    CmsgAppendFmt(result, "listen_port=%d", proc->listen_port_);
  for(const WgCidrAddr &x : proc->addresses_)
    CmsgAppendFmt(result, "address=%s", PrintWgCidrAddr(x, buf));
  
  for (WgPeer *peer = proc->dev_.peers_; peer; peer = peer->next_peer_) {
    WG_SCOPED_LOCK(peer->mutex_);
    
    CmsgAppendHex(result, "public_key", peer->s_remote_.bytes, sizeof(peer->s_remote_));
    if (!IsOnlyZeros(peer->preshared_key_, sizeof(peer->preshared_key_)))
      CmsgAppendHex(result, "preshared_key", peer->preshared_key_, sizeof(peer->preshared_key_));
    if (peer->tx_bytes_ | peer->rx_bytes_)
      CmsgAppendFmt(result, "tx_bytes=%lld\nrx_bytes=%lld", peer->tx_bytes_, peer->rx_bytes_);
    for (auto it = peer->allowed_ips_.begin(); it != peer->allowed_ips_.end(); ++it)
      CmsgAppendFmt(result, "allowed_ip=%s", PrintWgCidrAddr(*it, buf));
    if (peer->persistent_keepalive_ms_)
      CmsgAppendFmt(result, "persistent_keepalive_interval=%d", peer->persistent_keepalive_ms_ / 1000);
    if (peer->endpoint_.sin.sin_family == AF_INET)
      CmsgAppendFmt(result, "endpoint=%s:%d", PrintIpAddr(peer->endpoint_, buf), htons(peer->endpoint_.sin.sin_port));
    else if (peer->endpoint_.sin.sin_family == AF_INET6)
      CmsgAppendFmt(result, "endpoint=[%s]:%d", PrintIpAddr(peer->endpoint_, buf), htons(peer->endpoint_.sin6.sin6_port));
    
    if (peer->last_complete_handskake_timestamp_) {
      uint64 millis_since = OsGetMilliseconds() - peer->last_complete_handskake_timestamp_;
      uint64 when = time(NULL) - millis_since / 1000;
      CmsgAppendFmt(result, "last_handshake_time_sec=%lld", when);
    }
  }
  CmsgAppendFmt(result, "protocol_version=1");
}

bool WgConfig::HandleConfigurationProtocolMessage(WireguardProcessor *proc, const std::string &&message, std::string *result) {
  std::string message_copy(std::move(message));
  std::vector<std::pair<char *, char*>> kv;
  bool is_set = false;
  bool did_set_address = false;
  WgPeer *peer = NULL;
  WgCidrAddr cidr_addr;
  IpAddr sin;
  uint8 buf32[32];
  assert(proc->dev().IsMainThread());
  
  result->clear();
  
  if (!ParseConfigKeyValue(&message_copy[0], &kv))
    return false;
  
  for (auto it : kv) {
    char *key = it.first, *value = it.second;
    if (strcmp(key, "get") == 0) {
      if (strcmp(value, "1") != 0)
        goto getout_fail;
      HandleConfigurationProtocolGet(proc, result);
      break;
    } else if (strcmp(key, "set") == 0) {
      if (strcmp(value, "1") != 0)
        goto getout_fail;
      is_set = true;
    } else if (is_set) {
      if (strcmp(key, "private_key") == 0) {
        if (!ParseHexString(value, buf32, 32)) goto getout_fail;
        proc->dev_.SetPrivateKey(buf32);
      } else if (strcmp(key, "listen_port") == 0) {
        int new_port = atoi(value);
        proc->SetListenPort(new_port);
      } else if (strcmp(key, "replace_peers") == 0) {
        if (strcmp(value, "true") != 0) goto getout_fail;
        proc->dev_.RemoveAllPeers();
      } else if (strcmp(key, "address") == 0) {
        if (!ParseCidrAddr(value, &cidr_addr)) goto getout_fail;
        if (!did_set_address) {
          did_set_address = true;
          proc->ClearTunAddress();
        }
        if (!proc->SetTunAddress(cidr_addr)) goto getout_fail;
      } else if (strcmp(key, "public_key") == 0) {
        WgPublicKey pubkey;
        if (!ParseHexString(value, pubkey.bytes, 32)) goto getout_fail;
        peer = proc->dev_.GetPeerFromPublicKey(pubkey);
        if (!peer) {
          peer = proc->dev_.AddPeer();
          peer->SetPublicKey(pubkey);
        }
      } else if (peer != NULL) {
        if (strcmp(key, "remove") == 0) {
          if (strcmp(value, "true") != 0) goto getout_fail;
          peer->RemovePeer();
          peer = NULL;
        } else if (strcmp(key, "preshared_key") == 0) {
          if (!ParseHexString(value, buf32, 32)) goto getout_fail;
          peer->SetPresharedKey(buf32);
        } else if (strcmp(key, "endpoint") == 0) {
          if (!ParseSockaddrInWithPort(value, &sin, NULL)) goto getout_fail;
          peer->SetEndpoint(kPacketProtocolUdp, sin);
        } else if (strcmp(key, "persistent_keepalive_interval") == 0) {
          if (!peer->SetPersistentKeepalive(atoi(value)))
            goto getout_fail;
        } else if (strcmp(key, "replace_allowed_ips") == 0) {
          if (strcmp(value, "true") != 0) goto getout_fail;
          peer->RemoveAllIps();
        } else if (strcmp(key, "allowed_ip") == 0) {
          if (!ParseCidrAddr(value, &cidr_addr)) goto getout_fail;
          peer->AddIp(cidr_addr);
        }
      }
    } else {
      goto getout_fail;
    }
  }
  
  // reconfigure the tun interface?
  if (did_set_address) {
    proc->ConfigureTun();
  }
  
  result->append("errno=0\n\n");
  return true;
  
getout_fail:
  (*result) = "errno=1\n\n";
  return false;
}


