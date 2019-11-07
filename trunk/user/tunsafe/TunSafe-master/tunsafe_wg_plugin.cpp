#include "stdafx.h"
#include "tunsafe_wg_plugin.h"
#include "wireguard.h"
#include "util.h"
#include "crypto/curve25519/curve25519-donna.h"
#include "crypto/sha/sha1.h"
#include "crypto/chacha20poly1305.h"
#include "crypto/siphash/siphash.h"
#include "crypto/blake2s/blake2s.h"
#include "tunsafe_endian.h"
#include <algorithm>

enum {
  WG_SESSION_ID_LEN = 32,
  WG_SESSION_AUTH_LEN = 16,
};

class PluginPeer;
class TunsafePluginImpl;

class ExtFieldWriter {
public:
  ExtFieldWriter(uint8 *target, uint32 target_size) : target_(target), target_size_(target_size), target_pos_(0), fail_flag_(false) { }

  bool WriteField(uint8 code, const uint8 *data, uint32 size);
  void BlockLogin() { fail_flag_ = true; }
  bool fail_flag() { return fail_flag_; }
  uint32 length() {
    return target_pos_;
  }
private:
  uint8 *target_;
  uint32 target_size_;
  uint32 target_pos_;
  bool fail_flag_;
};

bool ExtFieldWriter::WriteField(uint8 code, const uint8 *data, uint32 size) {
  assert(size < 256);
  uint8 *dst = &target_[target_pos_];
  if (target_pos_ + size + 2 > target_size_)
    return false;
  target_pos_ += size + 2;
  dst[0] = code;
  dst[1] = size;
  memcpy(dst + 2, data, size);
  return true;
}

enum {
  // The other peer has no way of identifying a specific instance of
  // a connection. There's no way to distinguish a periodic handshake from
  // a new client connection. Add a session ID to the Peer to solve this. 
  // We don't send the actual session id, instead we send:
  // Hash(plaintext ephemeral public key, session id)
  kExtensionType_SessionIDAuth = 0x20,
  kExtensionType_SetSessionID = 0x21,

  // This is sent by the server to request an additional token to allow
  // login, for example a TOTP token, or a password.
  // By cleverly using session ids, the server can avoid having to request
  // this for every new handshake, even when roaming.
  kExtensionType_TokenRequest = 0x22,
  // This holds the token reply.
  kExtensionType_TokenReply = 0x23,
};

class TokenClientHandler {
public:
  TokenClientHandler(PluginPeer *pp);
  ~TokenClientHandler();

  void SetSessionId(const uint8 id[WG_SESSION_ID_LEN]);
  void SetToken(const uint8 *token, size_t token_size);
  void OnHandshakeCreate(WgPeer *peer, ExtFieldWriter &writer, const uint8 salt[WG_PUBLIC_KEY_LEN]);
  void OnTokenRequest(const uint8 *data, uint32 data_size);
  void OnHandshakeComplete();

  bool waiting_for_token() { return waiting_for_token_; }
  uint32 token_request() { return token_request_type_; }

  bool WantHandshake() { return !waiting_for_token_; }
  void WriteSessionId(ExtFieldWriter &writer, const uint8 salt[WG_PUBLIC_KEY_LEN]);
private:
  PluginPeer *pp_;

  // Set to true if we're waiting for the UI to set the TOTP-token, so login can continue.
  bool waiting_for_token_;
  uint8 token_size_;
  bool has_session_id_;

  uint32 token_request_type_;

  // The session id
  uint8 session_id_[WG_SESSION_ID_LEN];
  
  // Crypto key for tokens
  uint8 token_crypto_key_[WG_SYMMETRIC_KEY_LEN];

  // This is set to the token given by the UI
  uint8 token_[TunsafePlugin::kMaxTokenLen];
};

class TotpTokenAuthenticator {
public:
  TotpTokenAuthenticator() : secret_size_(0), window_size_(30), block_reuse_(false), digits_(6), precision_(0), next_allowed_code_(0) {}
  bool Initialize(const char *config);
  bool Authenticate(const uint8 *data, size_t size, uint64 *last_code);
  bool configured() { return secret_size_ != 0; }
  uint8 digits() { return digits_; }
private:
  uint32 GetValueForTimestamp(uint64 now);
  uint16 window_size_;
  uint16 precision_;
  bool block_reuse_;
  uint8 digits_;
  uint8 secret_size_;
  uint64 next_allowed_code_;
  uint8 secret_[64];
};

class TokenServerHandler {
public:
  TokenServerHandler();
  ~TokenServerHandler();
  bool OnHandshake(uint8 *token_reply, int token_reply_size, bool has_valid_session_id, ExtFieldWriter &writer, const siphash_key_t *siphash_key);
  bool OnHandshake2(bool has_valid_session_id);
  bool OnUnknownPeerSetting(const char *key, const char *value);
  bool WantHandshake() { return !stop_reconnects_; }
  bool VerifySessionId(const uint8 session_id_auth[WG_SESSION_AUTH_LEN], const uint8 salt[WG_PUBLIC_KEY_LEN]);

private:
  bool has_session_id_;
  bool is_session_id_authed_;
  bool stop_reconnects_;
  uint8 num_failures_;
  uint8 reset_recovery_counter_;
  uint8 token_bucket_;
  uint8 authentication_type_;
  uint8 last_login_status_;
  uint64 last_attempt_;
  uint64 reset_recovery_last_code_;
  uint64 last_cksum_;
  uint64 cksum_equal_timestamp_;
  enum {
    // Allow one token attempt every 30 seconds
    kTokenBucketCost = 30,

    // And the bucket size is 8, so you can perform 8 attempts
    // in a row.
    kTokenBucketFull = 240,

    // Failed attempts until lockout. When locked out, you need to
    // perform 3 successful 2fa attempts in a row before it's unlocked
    // for 3 different codes.
    kAttemptsUntilLockout = 10,

    kAttemptsUntilLockoutRemoved = 3,
  };

  TotpTokenAuthenticator token_authenticator_;
  uint8 session_id_[WG_SESSION_ID_LEN];
  uint8 token_crypto_key_[WG_SYMMETRIC_KEY_LEN];
};

class PluginPeer : public WgPeerExtraData {
public:
  PluginPeer(TunsafePluginImpl *plugin, WgPeer *peer) : plugin(plugin), peer(peer), token_client_handler(this) {}
  ~PluginPeer();

  virtual void OnPeerDestroy() override {
    delete this;
  }

  WgPeer *peer;
  TunsafePluginImpl *plugin;
  TokenClientHandler token_client_handler;
  TokenServerHandler token_server_handler;
};

// Toplevel wireguard plugin 
class TunsafePluginImpl : public TunsafePlugin {
  friend class PluginPeer;
public:
  TunsafePluginImpl(PluginDelegate *del, WireguardProcessor *proc) {
    delegate_ = del;
    proc_ = proc;
    peer_doing_2fa_ = NULL;
    OsGetRandomBytes((uint8*)&siphash_key_, sizeof(siphash_key_));
  }

  void DeletingPeer(PluginPeer *peer) {
    if (peer_doing_2fa_ == peer)
      peer_doing_2fa_ = NULL;
  }

  PluginDelegate *delegate() { return delegate_; }

  void OnTokenRequest(PluginPeer *peer);

private:
  virtual bool HandleUnknownPeerId(uint8 public_key[WG_PUBLIC_KEY_LEN], Packet *packet) override { return false; }
  virtual bool OnUnknownInterfaceSetting(const char *key, const char *value) override;
  virtual bool OnUnknownPeerSetting(WgPeer *peer, const char *key, const char *value) override;
  virtual bool WantHandshake(WgPeer *peer) override;
  virtual uint32 OnHandshake0(WgPeer *peer, uint8 *extout, uint32 extout_size, const uint8 salt[WG_PUBLIC_KEY_LEN]) override;
  virtual uint32 OnHandshake1(WgPeer *peer, const uint8 *ext, uint32 ext_size, const uint8 salt_in[WG_PUBLIC_KEY_LEN], uint8 *extout, uint32 extout_size, const uint8 salt_out[WG_PUBLIC_KEY_LEN]) override;
  virtual uint32 OnHandshake2(WgPeer *peer, const uint8 *ext, uint32 ext_size, const uint8 salt[WG_PUBLIC_KEY_LEN]) override;
  virtual bool OnAfterSettingsParsed() override;
  virtual void OnOutgoingHandshakePacket(WgPeer *peer, Packet *packet) override;

  PluginPeer *GetPluginPeer(WgPeer *peer);

  virtual void SubmitToken(const uint8 *text, size_t text_len) override;

  WireguardProcessor *proc_;
  PluginPeer *peer_doing_2fa_;
  PluginDelegate *delegate_;

  siphash_key_t siphash_key_;

};

PluginPeer::~PluginPeer() {
  plugin->DeletingPeer(this);
}


TokenClientHandler::TokenClientHandler(PluginPeer *pp) {
  pp_ = pp;
  waiting_for_token_ = false;
  token_size_ = false;
  has_session_id_ = false;
  token_request_type_ = 0;
  memset(token_crypto_key_, 0, sizeof(token_crypto_key_));
}

TokenClientHandler::~TokenClientHandler() {

}

void TokenClientHandler::SetSessionId(const uint8 id[WG_SESSION_ID_LEN]) {
  has_session_id_ = true;
  memcpy(session_id_, id, WG_SESSION_ID_LEN);
}

void TokenClientHandler::SetToken(const uint8 *token, size_t token_size) {
  if (token_size > TunsafePlugin::kMaxTokenLen || !waiting_for_token_)
    return;
  waiting_for_token_ = false;
  token_size_ = (uint8)token_size;
  memcpy(token_, token, token_size);
}

void TokenClientHandler::WriteSessionId(ExtFieldWriter &writer, const uint8 salt[WG_PUBLIC_KEY_LEN]) {
  if (has_session_id_) {
    uint8 buf[WG_SESSION_AUTH_LEN];
    blake2s(buf, WG_SESSION_AUTH_LEN, salt, WG_PUBLIC_KEY_LEN, session_id_, sizeof(session_id_));
    writer.WriteField(kExtensionType_SessionIDAuth, buf, WG_SESSION_AUTH_LEN);
  }
}

// This is called to include a token (if the server has set one) in outgoing handshakes.
void TokenClientHandler::OnHandshakeCreate(WgPeer *peer, ExtFieldWriter &writer, const uint8 salt[WG_PUBLIC_KEY_LEN]) {
  WriteSessionId(writer, salt);
  
  if (token_size_ && has_session_id_) {
    // Encrypt and include the token in the response
    // NOTE: Must not reuse the key to send different tokens, but we send
    // only one token as a reply to TokenRequest so that's fine.
    uint8 buf[TunsafePlugin::kMaxTokenLen + 16];
    chacha20poly1305_encrypt(buf, token_, token_size_, NULL, 0, 0, token_crypto_key_);
    writer.WriteField(kExtensionType_TokenReply, buf, 16 + token_size_);

    static const uint8 kPadding[16] = {0};
    writer.WriteField(kExtensionType_Padding, kPadding, -token_size_ & 0xF);
  }
}

// This runs on the initiator, after the handshake has been parsed
void TokenClientHandler::OnHandshakeComplete() {
  // Forget an old token, we'll request it again if needed.
  token_size_ = 0;
  memset(token_crypto_key_, 0, sizeof(token_crypto_key_));
}

// This runs when backend requests a token, ask the user for the token
// and then call SetToken.
void TokenClientHandler::OnTokenRequest(const uint8 *data, uint32 data_size) {
  if (data_size >= WG_SYMMETRIC_KEY_LEN + 2 && !waiting_for_token_) {
    memcpy(token_crypto_key_, data, WG_SYMMETRIC_KEY_LEN);
    token_request_type_ = ReadLE16(data + WG_SYMMETRIC_KEY_LEN);
    if (token_size_ && (token_request_type_ & kTokenRequestStatus_Mask) == kTokenRequestStatus_None)
      token_request_type_ |= kTokenRequestStatus_NotAccepted;
    waiting_for_token_ = true;
    pp_->plugin->OnTokenRequest(pp_);
  }
}

// Decode a base32 string, skip whitespace and =.
// returns 0 on failure.
static size_t DecodeBase32String(const char *string, size_t string_len, uint8 *output, size_t output_len) {
  //  static const char kBase32Charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
  size_t n = 0;
  uint32 bitbuff = 0, nbits = 0, v;
  for (size_t i = 0; i < string_len; i++) {
    uint8 c = string[i];
    if (c >= '2' && c <= '7') {
      v = c - '2' + 26;
    } else if ((c | 32) >= 'a' && (c | 32) <= 'z') {
      v = (c | 32) - 'a';
    } else if (c == ' ' || c == '=' || c == '\t') {
      continue;
    } else {
      return 0;
    }
    bitbuff = bitbuff * 32 + v;
    nbits += 5;
    if (nbits >= 8) {
      nbits -= 8;
      if (n == output_len)
        return 0;
      output[n++] = (uint8)(bitbuff >> nbits);
    }
  }
  return n;
}

// RequireToken=totp-sha1:ALPHABETAGAMMAOSCAR,digits=6,period=30,precision=15
bool TotpTokenAuthenticator::Initialize(const char *config) {
  const char *end = config + strlen(config);
  const char *comma = strchr(config, ',');
  size_t rv = DecodeBase32String(config, (comma ? comma : end) - config, secret_, sizeof(secret_));
  if (!rv)
    return false;
  secret_size_ = (uint8)rv;

  bool has_precision = false;
  while (comma) {
    comma += 1;
    while (*comma == ' ') comma++;
    if (strncmp(comma, "digits=", 7) == 0) {
      int v = atoi(comma + 7);
      if (v < 6 || v > 8)
        return false;
      digits_ = (uint8)v;
    } else if (strncmp(comma, "period=", 7) == 0) {
      int v = atoi(comma + 7);
      if (v < 1 || v > 3600)
        return false;
      window_size_ = (uint16)v;
    } else if (strncmp(comma, "precision=", 10) == 0) {
      int v = atoi(comma + 10);
      if (v < 0 || v > 3600)
        return false;
      has_precision = true;
      precision_ = (uint16)v;
    } else if (strncmp(comma, "reuse=0", 7) == 0) {
      block_reuse_ = true;
    } else {
      return false;
    }
    comma = strchr(comma, ',');
  }
  if (!has_precision)
    precision_ = window_size_ >> 1;

  return true;
}

extern int memcmp_crypto(const uint8 *a, const uint8 *b, size_t n);

uint32 TotpTokenAuthenticator::GetValueForTimestamp(uint64 now) {
  uint8 hmacbuf[20];
  SHA1HmacContext hmac;
  SHA1HmacReset(&hmac, secret_, secret_size_);
  uint8 timebuf[8];
  WriteBE64(timebuf, now);
  SHA1HmacInput(&hmac, timebuf, 8);
  SHA1HmacFinish(&hmac, hmacbuf);
  uint32 tmp;
  memcpy(&tmp, hmacbuf + (hmacbuf[19] & 0xF), 4);
  uint32 value = ReadBE32(&tmp) & 0x7FFFFFFF;
  switch (digits_) {
  case 6: value %= 1000000; break;
  case 7: value %= 10000000; break;
  case 8: value %= 100000000; break;
  }
  return value;
}

bool TotpTokenAuthenticator::Authenticate(const uint8 *data, size_t size, uint64 *code_out) {
  uint64 now = time(NULL);
  uint64 first_period = (now - precision_) / window_size_;
  uint64 last_period = (now + precision_) / window_size_;
  for (; first_period <= last_period; first_period++) {
    char buf[16];
    int r = snprintf(buf, sizeof(buf), "%.*u", digits_, GetValueForTimestamp(first_period));
//    RINFO("Checking if %.*s equals %s", (int)size, data, buf);
    if (r == size && memcmp_crypto((uint8*)buf, data, size) == 0) {
      // Disable code reuse if requested.
      if (block_reuse_ && first_period < next_allowed_code_)
        return false;

      next_allowed_code_ = first_period + 1;
      *code_out = first_period;
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TokenServerHandler::TokenServerHandler() {
  num_failures_ = 0;
  has_session_id_ = false;
  is_session_id_authed_ = false;
  stop_reconnects_ = false;
  last_attempt_ = 0;
  token_bucket_ = kTokenBucketFull;
  reset_recovery_counter_ = 0;
  reset_recovery_last_code_ = 0;
  last_cksum_ = 0;
  cksum_equal_timestamp_ = 0;
  last_login_status_ = 0;
}

TokenServerHandler::~TokenServerHandler() {
}

bool TokenServerHandler::OnHandshake(uint8 *token_reply, int token_reply_size, bool has_valid_session_id, ExtFieldWriter &writer, const siphash_key_t *siphash_key) {
  // Tokens not required for this peer?
  if (!token_authenticator_.configured())
    return true;
  // check that the client has a valid token session, otherwise block login.
  if (!has_valid_session_id) {
    OsGetRandomBytes(session_id_, sizeof(session_id_));
    writer.WriteField(kExtensionType_SetSessionID, session_id_, WG_SESSION_ID_LEN);
    is_session_id_authed_ = false;
    has_session_id_ = true;
request_token:
    // TODO: Make it optional to reveal last_login_status_
    uint8 data[WG_SYMMETRIC_KEY_LEN + 2];
    OsGetRandomBytes(token_crypto_key_, sizeof(token_crypto_key_));
    data[WG_SYMMETRIC_KEY_LEN] = authentication_type_;
    data[WG_SYMMETRIC_KEY_LEN + 1] = last_login_status_;
    memcpy(data, token_crypto_key_, WG_SYMMETRIC_KEY_LEN);
    writer.WriteField(kExtensionType_TokenRequest, data, sizeof(data));
    writer.BlockLogin();
    return true;
  }

  if (!is_session_id_authed_ && token_reply && token_reply_size >= 16) {
    // allow only so many attempts per second
    uint64 now = OsGetMilliseconds(), code_out;
    uint64 secs = (now - last_attempt_) >> 10;
    last_attempt_ += secs << 10;
    token_bucket_ = (uint8)std::min<uint64>(token_bucket_ + secs, kTokenBucketFull);

    bool authenticated = false;
    // Decrypt and verify the supplied key. If this fails, we're likely using an old key.
    if (chacha20poly1305_decrypt(token_reply, token_reply, token_reply_size, NULL, 0, 0, token_crypto_key_)) {
      authenticated = token_authenticator_.Authenticate(token_reply, token_reply_size - 16, &code_out);

      // Account is locked after 10 failed attempts. To unlock the account, you need to login 3 times successfully
      // in a row, with distincts, increasing codes, with no failed attempts in between.
      if (num_failures_ >= kAttemptsUntilLockout) {
        if (authenticated && code_out > reset_recovery_last_code_) {
          reset_recovery_last_code_ = code_out;
          if (reset_recovery_counter_++ == kAttemptsUntilLockoutRemoved - 1) {
            RINFO("Account unlocked.");
            num_failures_ = 0;
            reset_recovery_counter_ = 0;
            token_bucket_ = kTokenBucketFull;
          } else {
            authenticated = false;
          }
        } else {
          reset_recovery_counter_ = 0;
          authenticated = false;
        }
      } else {
        // Check if the password is the same as the previous attempt, this could indicate a retransmission of the packet.
        // Don't increase num_failures_ based on this, but after a minute, force authenticated to false.
        uint64 cksum = siphash(token_reply, token_reply_size - 16, siphash_key);
        if (cksum == last_cksum_) {
          if (now >= cksum_equal_timestamp_ + 60000) {
            authenticated = false;
          } else {
            if (authenticated)
              num_failures_ = 0;
          }
        } else {
          cksum_equal_timestamp_ = now;
          last_cksum_ = cksum;
          num_failures_ = authenticated ? 0 : num_failures_ + 1;
          if (num_failures_ == kAttemptsUntilLockout)
            RINFO("Account locked because of %d failed login attempts.", num_failures_);
        }
      }
    }
    last_login_status_ = (num_failures_ >= kAttemptsUntilLockout) ? (kTokenRequestStatus_Locked >> 8) : 
                         (token_bucket_ >= kTokenBucketCost) ?      (kTokenRequestStatus_Wrong >> 8) : 
                                                                    (kTokenRequestStatus_Ratelimit >> 8);
    // Fail when toket bucket is exceeded
    if (token_bucket_ >= kTokenBucketCost) {
      token_bucket_ -= kTokenBucketCost;
    } else {
      authenticated = false;
    }
    is_session_id_authed_ = authenticated;
  }

  if (!is_session_id_authed_)
    goto request_token;

  last_login_status_ = 0;
  stop_reconnects_ = false;
  return true;
}

bool TokenServerHandler::OnHandshake2(bool has_valid_session_id) {
  // Tokens not required for this peer?
  if (!token_authenticator_.configured())
    return true;
  // Only allow configuration in this direction if a valid session key was provided.
  if (has_valid_session_id && is_session_id_authed_)
    return true;

  // Stop further reconnections to this peer until further notice.
  stop_reconnects_ = true;
  return false;
}


bool TokenServerHandler::OnUnknownPeerSetting(const char *key, const char *value) {
  if (strcmp(key, "RequireToken") != 0)
    return false;

  if (strncmp(value, "totp-sha1:", 10) != 0)
    return false;

  if (!token_authenticator_.Initialize(value + 10))
    return false;
  authentication_type_ = kTokenRequestType_6digits + (token_authenticator_.digits() - 6);
  return true;
}

bool TokenServerHandler::VerifySessionId(const uint8 session_id_auth[WG_SESSION_AUTH_LEN], const uint8 salt[WG_PUBLIC_KEY_LEN]) {
  if (!has_session_id_)
    return false;
  uint8 buf[WG_SESSION_AUTH_LEN];
  blake2s(buf, WG_SESSION_AUTH_LEN, salt, WG_PUBLIC_KEY_LEN, session_id_, sizeof(session_id_));
  return memcmp_crypto(buf, session_id_auth, WG_SESSION_AUTH_LEN) == 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////


PluginPeer *TunsafePluginImpl::GetPluginPeer(WgPeer *peer) {
  PluginPeer *rv = (PluginPeer *)peer->extradata();
  if (!rv) {
    rv = new PluginPeer(this, peer);
    peer->SetExtradata(rv);
  }
  return rv;
}

bool TunsafePluginImpl::WantHandshake(WgPeer *peer) {
  PluginPeer *pp = GetPluginPeer(peer);
  if (WITH_TWO_FACTOR_AUTHENTICATION) {
    return pp->token_server_handler.WantHandshake() &&
           pp->token_client_handler.WantHandshake();
  } else {
    return true;
  }
}

// This runs on client and appends data
uint32 TunsafePluginImpl::OnHandshake0(WgPeer *peer, uint8 *extout, uint32 extout_size, const uint8 salt[WG_PUBLIC_KEY_LEN]) {
  PluginPeer *pp = GetPluginPeer(peer);
  ExtFieldWriter writer(extout, extout_size);
  if (WITH_TWO_FACTOR_AUTHENTICATION)
    pp->token_client_handler.OnHandshakeCreate(peer, writer, salt);
  return writer.length();
}

// This runs on the server to parse init and send response
uint32 TunsafePluginImpl::OnHandshake1(WgPeer *peer, const uint8 *ext, uint32 ext_size, const uint8 salt_in[WG_PUBLIC_KEY_LEN], uint8 *extout, uint32 extout_size, const uint8 salt_out[WG_PUBLIC_KEY_LEN]) {
  PluginPeer *pp = GetPluginPeer(peer);
  ExtFieldWriter writer(extout, extout_size);

  // Skip the version 
  if (ext_size >= 4)
    ext += 4, ext_size -= 4;

  bool has_valid_session_id = false;
  uint8 *token_reply = NULL;
  uint8 token_reply_size = 0;

  while (ext_size >= 2) {
    uint8 type = ext[0], size = ext[1];
    ext += 2, ext_size -= 2;
    if (size > ext_size)
      return false;
    if (WITH_TWO_FACTOR_AUTHENTICATION) {
      switch (type) {
      case kExtensionType_SessionIDAuth:
        if (size == WG_SESSION_AUTH_LEN)
          has_valid_session_id = pp->token_server_handler.VerifySessionId(ext, salt_in);
        break;

      case kExtensionType_TokenReply:
        token_reply = (uint8*)ext;
        token_reply_size = size;
        break;
      }
    }
    ext += size, ext_size -= size;
  }
  if (ext_size != 0)
    return kHandshakeResponseDrop;

  if (WITH_TWO_FACTOR_AUTHENTICATION) {
    // If this is a handshake in the other direction, also include session id.
    pp->token_client_handler.WriteSessionId(writer, salt_out);

    if (!pp->token_server_handler.OnHandshake(token_reply, token_reply_size, has_valid_session_id, writer, &siphash_key_))
      return kHandshakeResponseDrop;
  }

  return writer.length() + writer.fail_flag() * WgPlugin::kHandshakeResponseFail;
}

// This runs on client and parses response
uint32 TunsafePluginImpl::OnHandshake2(WgPeer *peer, const uint8 *ext, uint32 ext_size, const uint8 salt[WG_PUBLIC_KEY_LEN]) {
  PluginPeer *pp = GetPluginPeer(peer);

  // Skip the version
  if (ext_size >= 4)
    ext += 4, ext_size -= 4;

  bool has_valid_session_id = false;

  while (ext_size >= 2) {
    uint8 type = ext[0], size = ext[1];
    ext += 2, ext_size -= 2;
    if (size > ext_size)
      return false;

    if (WITH_TWO_FACTOR_AUTHENTICATION) {
      switch (type) {
      case kExtensionType_SessionIDAuth:
        if (size == WG_SESSION_AUTH_LEN)
          has_valid_session_id = pp->token_server_handler.VerifySessionId(ext, salt);
        break;
        // All token requests mean that handshake has failed.  
      case kExtensionType_TokenRequest:
        pp->token_client_handler.OnTokenRequest(ext, size);
        return kHandshakeResponseDrop;
      case kExtensionType_SetSessionID:
        if (size == WG_SESSION_ID_LEN)
          pp->token_client_handler.SetSessionId(ext);
        break;
      }
    }
    ext += size, ext_size -= size;
  }
  if (ext_size != 0)
    return kHandshakeResponseDrop;

  if (WITH_TWO_FACTOR_AUTHENTICATION) {
    // Stop outgoing handshakes if client didn't supply a valid session id.
    if (!pp->token_server_handler.OnHandshake2(has_valid_session_id))
      return kHandshakeResponseDrop;
    pp->token_client_handler.OnHandshakeComplete();
  }
  return 0;
}

bool TunsafePluginImpl::OnUnknownInterfaceSetting(const char *key, const char *value) {
  return false;
}

bool TunsafePluginImpl::OnUnknownPeerSetting(WgPeer *peer, const char *key, const char *value) {
  PluginPeer *pp = GetPluginPeer(peer);
  if (WITH_TWO_FACTOR_AUTHENTICATION && pp->token_server_handler.OnUnknownPeerSetting(key, value))
    return true;

  return false;
}

void TunsafePluginImpl::OnTokenRequest(PluginPeer *peer) {
  if (!WITH_TWO_FACTOR_AUTHENTICATION || peer_doing_2fa_ != NULL)
    return;
  peer_doing_2fa_ = peer;
  delegate_->OnRequestToken(peer->peer, peer->token_client_handler.token_request());
}

void TunsafePluginImpl::SubmitToken(const uint8 *text, size_t text_len) {
  if (!WITH_TWO_FACTOR_AUTHENTICATION || peer_doing_2fa_ == NULL)
    return;
  assert(peer_doing_2fa_->peer->dev()->IsMainThread());
  peer_doing_2fa_->token_client_handler.SetToken(text, text_len);
  proc_->ForceSendHandshakeInitiation(peer_doing_2fa_->peer);
  peer_doing_2fa_ = NULL;

  // Find the next peer requiring a token
  for (WgPeer *peer = proc_->dev().first_peer(); peer; peer = peer->next_peer()) {
    PluginPeer *pp = (PluginPeer *)peer->extradata();
    if (!pp)
      continue;
    if (pp->token_client_handler.waiting_for_token()) {
      OnTokenRequest(pp);
      return;
    }
  }
}

bool TunsafePluginImpl::OnAfterSettingsParsed() {
  return true;
}

void TunsafePluginImpl::OnOutgoingHandshakePacket(WgPeer *peer, Packet *packet) {
}

TunsafePlugin *CreateTunsafePlugin(PluginDelegate *delegate, WireguardProcessor *wgp) {
  return new TunsafePluginImpl(delegate, wgp);
}

