#include "stdafx.h"
#include "tunsafe_types.h"
#include "netapi.h"
#include "crypto/curve25519/curve25519-donna.h"
#include "util.h"
#include "wireguard_proto.h"
#include <string.h>
#include <algorithm>

#if defined(OS_WIN)
#include "util_win32.h"
#include "service_pipe_win32.h"
#include "service_win32_constants.h"
#endif  // defined(OS_WIN)

#if defined(OS_POSIX)
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#endif  // defined(OS_WIN)


#pragma comment(lib, "ws2_32.lib")

#define ANSI_RESET "\x1b[0m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_FG_BLACK "\x1b[30m"
#define ANSI_FG_RED "\x1b[31m"
#define ANSI_FG_GREEN "\x1b[32m"
#define ANSI_FG_YELLOW "\x1b[33m"
#define ANSI_FG_BLUE "\x1b[34m"
#define ANSI_FG_MAGENTA "\x1b[35m"
#define ANSI_FG_CYAN "\x1b[36m"
#define ANSI_FG_WHITE "\x1b[37m"

#if defined(OS_WIN)
#define EXENAME "tunsafe"

static bool SendMessageToService(HANDLE pipe, int message, const void *data, size_t data_size) {
  uint8 *temp = new uint8[data_size + 5];
  *(uint32*)temp = (uint32)(data_size + 1);
  temp[4] = (uint8)message;
  memcpy(temp + 5, data, data_size);
  // Write the whole thing
  DWORD pos = 0, bytes_to_write = (DWORD)(data_size + 5), bytes_written;
  do {
    if (!WriteFile(pipe, temp + pos, bytes_to_write, &bytes_written, NULL)) {
      fprintf(stderr, "Error writing to service pipe, error = %d\n", GetLastError());
      break;
    }
    pos += bytes_written;
    bytes_to_write -= bytes_written;
  } while (bytes_to_write != 0);
  delete[] temp;
  return (bytes_to_write == 0);
}

static bool ReadExactBytesFromPipe(HANDLE pipe, const void *data, DWORD bytes_to_read) {
  DWORD pos = 0, n;
  do {
    if (!ReadFile(pipe, (uint8*)data + pos, bytes_to_read, &n, NULL))
      return false;
    if (n == 0)
      return false; // premature eof..
    pos += n;
    bytes_to_read -= n;
  } while (bytes_to_read != 0);
  return true;
}

static bool ReadMessageFromService(HANDLE pipe, int *message, std::string *data) {
  uint8 header[5];
  uint32 message_size;

  if (!ReadExactBytesFromPipe(pipe, header, 5) || (message_size = *(uint32*)header) == 0) {
    fprintf(stderr, "Error reading from service pipe, error = %d\n", GetLastError());
    return false;
  }
  *message = header[4];
  data->resize(message_size - 1);
  if (message_size - 1 != 0 && !ReadExactBytesFromPipe(pipe, data->data(), message_size - 1)) {
    fprintf(stderr, "Error reading from service pipe, error = %d\n", GetLastError());
    return false;
  }
  return true;
}

struct ServiceLoginMessage {
  uint64 version;
  char interfac[kTsMaxDevnameSize];
  bool want_state_updates;
  bool want_create_interface;
};

static std::vector<GuidAndDevName> g_tap_adapters;
static bool g_did_get_adapters;
static const std::vector<GuidAndDevName> &GetTapAdapterInfo() {
  if (!g_did_get_adapters) {
    g_did_get_adapters = true;
    GetTapAdapterInfo(&g_tap_adapters);
  }
  return g_tap_adapters;
}

static const char *GetGuidFromInterfaceName(const char *name) {
  for (const GuidAndDevName &e : GetTapAdapterInfo()) 
    if (strcmp(e.name, name) == 0)
      return e.guid;
  return NULL;
}

static const char *GetInterfaceNameFromGuid(const char *guid) {
  for (const GuidAndDevName &e : GetTapAdapterInfo())
    if (strcmp(e.guid, guid) == 0)
      return e.name;
  return NULL;
}


static HANDLE ConnectToService(const char *devname, bool want_updates, bool want_create = false) {
  ServiceLoginMessage msg = {0};
  msg.version = TUNSAFE_SERVICE_PROTOCOL_VERSION;
  msg.want_state_updates = want_updates;
  msg.want_create_interface = want_create;

  // Rename devname to a guid
  if (devname) {
    const char *guid = (devname[0] == '{' || devname[0] == 0) ? devname : GetGuidFromInterfaceName(devname);
    if (!guid) {
      fprintf(stderr, "Interface '%s' not found\n", devname);
      return NULL;
    }
    my_strlcpy(msg.interfac, sizeof(msg.interfac), guid);
  }
  
  for (;;) {
    HANDLE pipe = CreateFile(TUNSAFE_PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                      OPEN_EXISTING, 0, NULL);
    if (pipe != INVALID_HANDLE_VALUE) {
      if (!SendMessageToService(pipe, TS_SERVICE_REQ_LOGIN, &msg, sizeof(msg))) {
        CloseHandle(pipe);
        pipe = NULL;
      }
      return pipe;
    }
    DWORD error = GetLastError();
    if (error != ERROR_PIPE_BUSY) {
      fprintf(stderr, "Error connecting to TunSafe service, error = %d\n", error);
      if (error == ERROR_FILE_NOT_FOUND)
        fprintf(stderr, "Please check that the TunSafe service is started\n");
      return NULL;
    }
    if (!WaitNamedPipe(TUNSAFE_PIPE_NAME, 10000)) {
      fprintf(stderr, "Error connecting to TunSafe service, timed out.\n");
      return NULL;
    }
  }
}

static bool CommunicateWithService(const char *devname, const std::string &query, std::string *reply) {
  HANDLE pipe = ConnectToService(devname, false);
  int message_code;
  bool rv = false;

  if (pipe != NULL &&
      SendMessageToService(pipe, TS_SERVICE_REQ_TEXT_PROTOCOL, query.data(), query.size()) &&
      ReadMessageFromService(pipe, &message_code, reply)) {
    if (message_code == TS_SERVICE_REQ_TEXT_PROTOCOL_REPLY) {
      rv = true;
    } else {
      if (message_code == TS_SERVICE_MSG_ERROR_REPLY) {
        fprintf(stderr, "Error: %s\n", reply->c_str());
      } else {
        fprintf(stderr, "Unknown reply (%d) from TunSafe service.\n", message_code);
      }
    }
  }
  CloseHandle(pipe);
  return rv;
}

static bool GetInterfaceList(std::string *result) {
  HANDLE pipe = ConnectToService(NULL, false);
  int message_code;
  bool rv = false;

  if (pipe != NULL &&
      SendMessageToService(pipe, TS_SERVICE_REQ_GETINTERFACES, NULL, 0) &&
      ReadMessageFromService(pipe, &message_code, result)) {
    if (message_code == TS_SERVICE_REQ_GETINTERFACES_REPLY) {
      rv = true;
    } else {
      fprintf(stderr, "GetInterfaceList: bad reply\n");
    }
  }
  CloseHandle(pipe);
  return rv;
}

// Supports stripping ansi colors
static bool g_supports_ansi_color;
static void ansi_printf(const char *s, ...) {
  va_list va;
  va_start(va, s);
  if (g_supports_ansi_color) {
    vprintf(s, va);
  } else {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), s, va);
    char *s = buf, *d = s, c;
    for (; (c = *s) != 0;) {
      if (c == '\x1b' && s[1] == '[') {
        s += 2;
        while ((c = *s) >= '0' && c <= '9')
          s++;
        if (c == 'm')
          s++;
      } else {
        *d++ = c;
        s++;
      }
    }
    *d = 0;
    fputs(buf, stdout);
  }
  va_end(va);
}

#endif  // defined(OS_WIN)

#if defined(OS_POSIX)
#define EXENAME "tunsafe"
#define ansi_printf printf

static const char *GetGuidFromInterfaceName(const char *name) {
  return name;
}

static const char *GetInterfaceNameFromGuid(const char *guid) {
  return guid;
}


static int OpenUserspaceInterface(const char *iface) {
  struct stat st;
  struct sockaddr_un un = { 0 };
  int fd = -1, rv;

  if (strchr(iface, '/') != NULL) {
    fprintf(stderr, "Unable to open usermode socket: No such device\n");
    goto getout;
  }

  snprintf(un.sun_path, sizeof(un.sun_path), "/var/run/wireguard/%s.sock", iface);
  if (stat(un.sun_path, &st) < 0) {
    perror("Unable to open usermode socket");
    goto getout;
  }

  if (!S_ISSOCK(st.st_mode)) {
    fprintf(stderr, "Unable to open usermode socket: No such device\n");
    goto getout;
  }

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0)
    goto getout;

  un.sun_family = AF_UNIX;
  if (connect(fd, (struct sockaddr *)&un, sizeof(un)) < 0) {
    if (errno == ECONNREFUSED)
      unlink(un.sun_path);
    else
      perror("Error opening wireguard usermode interface socket");
    goto getout;
  }
  return fd;

getout:
  if (fd >= 0)
    close(fd);
  return -1;
}


static bool GetInterfaceList(std::string *result) {
  struct dirent *dent;

  DIR *dir = opendir("/var/run/wireguard/");
  if (!dir)
    return errno == ENOENT;

  while ((dent = readdir(dir)) != NULL) {
    size_t len = strlen(dent->d_name);
    static const char kSuffix[6] = ".sock";
    if (len >= sizeof(kSuffix) - 1 &&
        memcmp(&dent->d_name[len - (sizeof(kSuffix) - 1)], kSuffix, sizeof(kSuffix) - 1) == 0) {
      dent->d_name[len - (sizeof(kSuffix) - 1)] = '\n';
      result->append(dent->d_name, len - (sizeof(kSuffix) - 1) + 1);
    }
  }
  closedir(dir);
  return true;
}

static bool CommunicateWithService(const char *devname, const std::string &query, std::string *reply) {
  ssize_t n;
  char buf[4096];
  bool rv = false;

  reply->clear();

  int fd = OpenUserspaceInterface(devname);
  if (fd == -1)
    return false;  

  for(size_t pos = 0; query.size() - pos; pos += n) {
    n = write(fd, query.data() + pos, query.size() - pos);
    if (n <= 0) {
      perror("Error writing to service pipe");
      goto getout;
    }
  }

  for(;;) {
    n = read(fd, buf, sizeof(buf));
    if (n <= 0) {
      if (n == 0) {
        // ensure that it ends with \n\n
        if (reply->size() >= 2 && (*reply)[reply->size() - 1] == '\n' &&  (*reply)[reply->size() - 2] == '\n') {
          rv = true;
        } else {
          fprintf(stderr, "Bad reply from service pipe\n");  
        }
      } else {
        perror("Error reading from service pipe");
      }
      break;
    }
    reply->append(buf, n);
  }

getout:
  close(fd);
  return rv;
}

static int HandleStopCommand(int argc, char **argv) {
  if (argc != 1) {
    fprintf(stderr, "Usage: " EXENAME " stop <interface>\n");
    return 1;
  }
  struct sockaddr_un un;
  struct stat st;
  const char *iface = argv[0];
  if (strchr(iface, '/')) {
    fprintf(stderr, "No such interface\n");
    return 1;
  }
  snprintf(un.sun_path, sizeof(un.sun_path), "/var/run/wireguard/%s.sock", iface);
  if (unlink(un.sun_path) == -1) {
    perror("unlink");
    return 1;
  }
  return 0;
}

#endif  // defined(OS_POSIX)

void ShowHelp() {
  fprintf(stderr,
    "Usage: " EXENAME " <cmd> [<args>]\n\n"
#if defined(OS_POSIX)
    "       " EXENAME " filename.conf\n\n"
#endif  // defined(OS_POSIX)
    "Available subcommands:\n"
    "  show: Shows the configuration and status of the interfaces\n"
    "  set: Change the configuration or the peer list\n"
    "  start: Start TunSafe on an interface\n"
    "  stop: Stop TunSafe on an interface\n"
#if defined(OS_WIN)
    "  log: Display recent log entries\n"
#endif  // defined(OS_WIN)
    "  genkey: Writes a new private key to stdout\n"
    "  genpsk: Writes a new preshared key to stdout\n"
    "  pubkey: Reads a private key from stdin and writes its public key to stdout\n"
    "To see more help about a subcommand, pass --help to it\n");
}



static bool ParseHexKeyToBase64(const char *key, char base64key[WG_PUBLIC_KEY_LEN_BASE64 + 1]) {
  uint8 keybuf[32];
  if (!ParseHexString(key, keybuf, 32))
    return false;
  return base64_encode(keybuf, 32, base64key, WG_PUBLIC_KEY_LEN_BASE64 + 1, NULL) != NULL;
}

static char *FormatTransferPart(char *buf, size_t bufsize, uint64 n) {
  if (n < 1024)
    snprintf(buf, bufsize, "%u " ANSI_FG_CYAN "B" ANSI_RESET, (unsigned)n);
  else if (n < 1024 * 1024)
    snprintf(buf, bufsize, "%.2f " ANSI_FG_CYAN "KiB" ANSI_RESET, (double)n * (1.0 / 1024));
  else if (n < 1024 * 1024 * 1024)
    snprintf(buf, bufsize, "%.2f " ANSI_FG_CYAN "MiB" ANSI_RESET, (double)n * (1.0 / 1024 / 1024));
  else if (n < 1024ull * 1024 * 1024 * 1024)
    snprintf(buf, bufsize, "%.2f " ANSI_FG_CYAN "GiB" ANSI_RESET, (double)n * (1.0 / 1024 / 1024 / 1024));
  else
    snprintf(buf, bufsize, "%.2f " ANSI_FG_CYAN "TiB" ANSI_RESET, (double)n * (1.0 / 1024 / 1024 / 1024 / 1024));
  return buf;
}

static size_t PrintTime(char *buf, size_t bufsize, uint64 n) {
  size_t pos = 0;
  uint64 years = n / (365 * 24 * 60 * 60);
  uint32 n32 = n % (365 * 24 * 60 * 60);
  if (years)
    pos += snprintf(buf + pos, bufsize - pos, "%llu " ANSI_FG_CYAN "year%s" ANSI_RESET ", ", (unsigned long long)years, (years == 1) ? "" : "s");
  uint32 days = n32 / (24 * 60 * 60);
  n32 %= (24 * 60 * 60);
  if (days)
    pos += snprintf(buf + pos, bufsize - pos, "%u " ANSI_FG_CYAN "day%s" ANSI_RESET ", ", days, (days == 1) ? "" : "s");
  uint32 hours = n32 / (60 * 60);
  n32 %= (60 * 60);
  if (hours)
    pos += snprintf(buf + pos, bufsize - pos, "%u " ANSI_FG_CYAN "hour%s" ANSI_RESET ", ", hours, (hours == 1) ? "" : "s");
  uint32 minutes = n32 / 60;
  if (minutes)
    pos += snprintf(buf + pos, bufsize - pos, "%u " ANSI_FG_CYAN "minute%s" ANSI_RESET ", ", minutes, (minutes == 1) ? "" : "s");
  uint32 seconds = n32 % 60;
  if (seconds)
    pos += snprintf(buf + pos, bufsize - pos, "%u " ANSI_FG_CYAN "second%s" ANSI_RESET ", ", seconds, (seconds == 1) ? "" : "s");
  if (pos)
    buf[pos -= 2] = '\0';
  return pos;
}

static char *PrintHandshake(char *buf, size_t bufsize, uint64 secs) {
  time_t now = time(NULL);
  if (now == secs) {
    snprintf(buf, bufsize, "Now");
  } else if (now < (int64)secs) {
    snprintf(buf, bufsize, ANSI_FG_RED "System clock going backwards" ANSI_RESET);
  } else {
    size_t pos = PrintTime(buf, bufsize - 4, now - secs);
    memcpy(buf + pos, " ago", 5);
  }
  return buf;
}

static void AppendIpToString(const char *value, std::string *result) {
  if (!result->empty())
    (*result) += ", ";
  const char *slash = strchr(value, '/');
  if (slash) {
    result->append(value, slash - value);
    result->append(ANSI_FG_CYAN "/" ANSI_RESET);
    result->append(slash + 1);
  } else {
    result->append(value);
  }
}

static int ShowUserFriendlyForDevice(char *devname) {
  std::string reply;
  std::vector<std::pair<char*, char*>> kv;
  std::string ips;

  if (!CommunicateWithService(devname, "get=1\n\n", &reply))
    return 1;

  if (!ParseConfigKeyValue(&reply[0], &kv)) {
getout_fail:
    fprintf(stderr, "Unable to parse response");
    return 1;
  }

  size_t i = 0;
  char base64key[WG_PUBLIC_KEY_LEN_BASE64 + 1];
  char base64psk[WG_PUBLIC_KEY_LEN_BASE64 + 1];
  int listen_port = 0;
  base64key[0] = 0;

  // Parse all interface level keys
  for (; i < kv.size(); i++) {
    char *key = kv[i].first, *value = kv[i].second;
    if (strcmp(key, "private_key") == 0) {
      uint8 binkey[32];
      if (!ParseHexString(value, binkey, sizeof(binkey)))
        goto getout_fail;
      if (!IsOnlyZeros(binkey, 32)) {
        curve25519_donna(binkey, binkey, kCurve25519Basepoint);
        base64_encode(binkey, sizeof(binkey), base64key, sizeof(base64key), NULL);
      }
    } else if (strcmp(key, "address") == 0) {
      AppendIpToString(value, &ips);
    } else if (strcmp(key, "listen_port") == 0) {
      listen_port = atoi(value);
    } else if (strcmp(key, "public_key") == 0) {
      break;
    }
  }

  const char *interfacename = (devname[0] == '{') ? GetInterfaceNameFromGuid(devname) : devname;

  ansi_printf(ANSI_RESET ANSI_FG_GREEN ANSI_BOLD "interface" ANSI_RESET ": " ANSI_FG_GREEN "%s" ANSI_RESET "\n",
         interfacename);
  if (base64key[0]) {
    ansi_printf("  " ANSI_BOLD "public key" ANSI_RESET ": %s\n"
           "  " ANSI_BOLD "private key" ANSI_RESET ": (hidden)\n", base64key);
  }
  if (listen_port)
    ansi_printf("  " ANSI_BOLD "listening port" ANSI_RESET ": %d\n", listen_port);
  if (ips.size())
    ansi_printf("  " ANSI_BOLD "address" ANSI_RESET ": %s\n", ips.c_str());

  const char *endpoint = NULL;
  uint64 rx_bytes, tx_bytes, last_handshake_time_sec;
  int persistent_keepalive;
  char text[256];
  bool clear_state = true;

  // Parse peer level keys
  for (; i < kv.size(); i++) {
    char *key = kv[i].first, *value = kv[i].second;

    if (clear_state) {
      base64key[0] = base64psk[0] = 0;
      endpoint = NULL;
      ips.clear();
      persistent_keepalive = 0;
      last_handshake_time_sec = tx_bytes = rx_bytes = 0;
      clear_state = false;
    }
    if (strcmp(key, "public_key") == 0) {
      if (!ParseHexKeyToBase64(value, base64key))
        goto getout_fail;
    } else if (strcmp(key, "preshared_key") == 0) {
      if (!ParseHexKeyToBase64(value, base64psk))
        goto getout_fail;
    } else if (strcmp(key, "tx_bytes") == 0) {
      tx_bytes = strtoull(value, NULL, 0);
    } else if (strcmp(key, "rx_bytes") == 0) {
      rx_bytes = strtoull(value, NULL, 0);
    } else if (strcmp(key, "allowed_ip") == 0) {
      AppendIpToString(value, &ips);
    } else if (strcmp(key, "persistent_keepalive_interval") == 0) {
      persistent_keepalive = atoi(value);
    } else if (strcmp(key, "endpoint") == 0) {
      endpoint = value;
    } else if (strcmp(key, "last_handshake_time_sec") == 0) {
      last_handshake_time_sec = strtoull(value, NULL, 0);
    }
    if (i == kv.size() - 1 || strcmp(kv[i + 1].first, "public_key") == 0) {
      if (!base64key[0])
        goto getout_fail;
      ansi_printf("\n" ANSI_FG_YELLOW ANSI_BOLD "peer" ANSI_RESET ": " ANSI_FG_YELLOW "%s" ANSI_RESET "\n", base64key);
      if (base64psk[0])
        ansi_printf("  " ANSI_BOLD "preshared key" ANSI_RESET ": (hidden)\n");
      if (endpoint)
        ansi_printf("  " ANSI_BOLD "endpoint" ANSI_RESET ": %s\n", endpoint);
      ansi_printf("  " ANSI_BOLD "allowed ips" ANSI_RESET ": %s\n", ips.size() ? ips.c_str() : "(none)");
      if (last_handshake_time_sec)
        ansi_printf("  " ANSI_BOLD "latest handshake" ANSI_RESET ": %s\n", PrintHandshake(text, sizeof(text), last_handshake_time_sec));
      if (tx_bytes | rx_bytes) {
        ansi_printf("  " ANSI_BOLD "transfer" ANSI_RESET ": %s received, ", FormatTransferPart(text, sizeof(text), rx_bytes));
        ansi_printf("%s sent\n", FormatTransferPart(text, sizeof(text), tx_bytes));
      }
      if (persistent_keepalive) {
        PrintTime(text, sizeof(text), persistent_keepalive);
        ansi_printf("  " ANSI_BOLD "persistent keepalive" ANSI_RESET ": every %s\n", text);
      }
      clear_state = true;
    }
  }
  return 0;
}

static int HandleShowCommand(int argc, char **argv) {
  if (argc != 0 && strcmp(argv[0], "--help") == 0) {
    fprintf(stderr, "Usage: ts show { <interface> | all | interfaces }\n");
    return 0;
  }

  std::vector<char*> interfaces;
  std::string interfaces_str;

  if (argc == 0 || strcmp(argv[0], "all") == 0) {
    if (!GetInterfaceList(&interfaces_str))
      return 1;
    SplitString(&interfaces_str[0], '\n', &interfaces);

    bool want_newline = false;
    for (char *interfac : interfaces) {
      if (want_newline)
        ansi_printf("\n");
      want_newline = true;
      if (ShowUserFriendlyForDevice(interfac))
        return 1;
    }
  } else if (strcmp(argv[0], "interfaces") == 0) {
    if (!GetInterfaceList(&interfaces_str))
      return 1;
    SplitString(&interfaces_str[0], '\n', &interfaces);

    for (char *interfac : interfaces) {
      const char *name = GetInterfaceNameFromGuid(interfac);
      if (name)
        ansi_printf("%s\n", name);
    }
  } else {
    return ShowUserFriendlyForDevice(argv[0]);
  }
  return 0;
}

static void AppendCommand(std::string *result, const char *tag, const char *value) {
  result->append(tag);
  result->append("=");
  result->append(value);
  result->append("\n");
}

static bool ConvertBase64KeyToHex(const char *s, char key[65]) {
  uint8 tmp[32];
  size_t size = 32;
  if (!base64_decode((uint8*)s, strlen(s), tmp, &size) || size != 32)
    return false;
  PrintHexString(tmp, 32, key);
  return true;
}

static int HandleSetCommand(int argc, char **argv) {
  std::string command, reply;
  std::vector<char*> ss;
  char hexkey[65];

  if (argc == 0) {
    fprintf(stderr, "Usage: " EXENAME " set <interface> [address <address>] [listen-port <port>] [private-key <file path>] "
            "[peer <base64 public key> [remove] [preshared-key <file path>] [endpoint <ip>:<port>] "
            "[persistent-keepalive <interval seconds>] [allowed-ips <ip1>/<cidr1>[,<ip2>/<cidr2>]] ]");
    return 1;
  }
  char **argv_end = argv + argc;
  const char *interfc = *argv++;
  
  command = "set=1\n";

  bool in_interface_section = true;
  bool in_peer_section = false;
  bool did_clear_allowed_ips = false;

  while (argv != argv_end) {
    const char *key = *argv++;
    
    if (argv != argv_end) {
      if (in_interface_section) {
        if (strcmp(key, "listen-port") == 0) {
          AppendCommand(&command, "listen_port", *argv++);
          continue;
        } else if (strcmp(key, "address") == 0) {
          AppendCommand(&command, "address", *argv++);
          continue;
        } else if (strcmp(key, "private-key") == 0) {
          if (!ConvertBase64KeyToHex(*argv++, hexkey))
            goto invalid_key_format;
          AppendCommand(&command, "private_key", hexkey);
          continue;
        }
      }
      if (strcmp(key, "peer") == 0) {
        in_interface_section = false;
        in_peer_section = true;
        did_clear_allowed_ips = false;
        if (!ConvertBase64KeyToHex(*argv++, hexkey))
          goto invalid_key_format;
        AppendCommand(&command, "public_key", hexkey);
        
        continue;
      }
      if (in_peer_section) {
        if (strcmp(key, "preshared-key") == 0) {
          if (!ConvertBase64KeyToHex(*argv++, hexkey))
            goto invalid_key_format;
          AppendCommand(&command, "preshared_key", hexkey);
          continue;
        } else if (strcmp(key, "endpoint") == 0) {
          AppendCommand(&command, "endpoint", *argv++);
          continue;
        } else if (strcmp(key, "persistent-keepalive") == 0) {
          AppendCommand(&command, "persistent_keepalive_interval", *argv++);
          continue;
        } else if (strcmp(key, "allowed-ips") == 0) {
          if (!did_clear_allowed_ips) {
            AppendCommand(&command, "replace_allowed_ips", "true");
            did_clear_allowed_ips = true;
          }
          SplitString(*argv++, ',', &ss);
          for (char *x : ss)
            AppendCommand(&command, "allowed_ip", x);
          continue;
        }
      }
    }
    if (in_peer_section) {
      if (strcmp(key, "remove") == 0) {
        in_peer_section = false;
        AppendCommand(&command, "remove", "true");
        continue;
      }
    }

    fprintf(stderr, "Invalid argument: %s\n", key);
    return 1;

invalid_key_format:
    fprintf(stderr, "Key is not in the correct format: '%s'\n", argv[-1]);
    return 1;
  }

  command.append("\n");

  if (!CommunicateWithService(interfc, command, &reply))
    return 1;

  return 0;
}

#if defined(OS_WIN)
static int HandleLogCommand() {
  HANDLE pipe = ConnectToService(NULL, true);

  int message_code;
  std::string reply;

  while (pipe != NULL && ReadMessageFromService(pipe, &message_code, &reply) && message_code == TS_SERVICE_MSG_LOGLINE)
    ansi_printf("%s\n", reply.c_str());

  CloseHandle(pipe);
  return 0;
}

static int HandleStartCommand(int argc, char **argv) {
  if (argc < 1 || argc > 2 || strcmp(argv[0], "--help") == 0) {
    fprintf(stderr, "Usage: " EXENAME " start <interface> [<filename>]\n");
    return 1;
  }

  const char *devname = argv[0];
  HANDLE pipe = ConnectToService(devname, false, true);
  int message_code;
  std::string reply;

  const char *path = (argc == 1) ? "" : argv[1];

  // Tell the server to startup a new interface
  if (pipe == NULL ||
      !SendMessageToService(pipe, TS_SERVICE_REQ_START, path, strlen(path) + 1) || 
      !ReadMessageFromService(pipe, &message_code, &reply))
    return 1;

  if (message_code == TS_SERVICE_MSG_ERROR_REPLY) {
    fprintf(stderr, "%s\n", reply.c_str());
    return 1;
  }

  return 0;
}

static int HandleStopCommand(int argc, char **argv) {
  if (argc != 1) {
    fprintf(stderr, "Usage: " EXENAME " stop <interface>\n");
    return 1;
  }

  const char *devname = argv[0];
  HANDLE pipe = ConnectToService(devname, false);

  // Tell the server to stop the interface
  if (pipe == NULL ||
      !SendMessageToService(pipe, TS_SERVICE_REQ_STOP, NULL, 0))
    return 1;
  return 0;
}
#endif  // defined(OS_WIN)


// Returns -1 on invalid subcommand
int HandleCommandLine(int argc, char **argv, CommandLineOutput *output) {
  uint8 key[32];
  char base64buf[WG_PUBLIC_KEY_LEN_BASE64 + 1];

  if (argc == 1) {
    ShowHelp();
    return 1;
  }

  const char *subcommand = argv[1];
  argv += 2;
  argc -= 2;

  if (!strcmp(subcommand, "show")) {
    return HandleShowCommand(argc, argv);

  } else if (!strcmp(subcommand, "set")) {
    return HandleSetCommand(argc, argv);

#if defined(OS_WIN)
  } else if (!strcmp(subcommand, "log")) {
    if (argc != 0) {
      fprintf(stderr, "Usage: " EXENAME " log\n");
      return 1;
    }
    return HandleLogCommand();

  } else if (!strcmp(subcommand, "start")) {
    return HandleStartCommand(argc, argv);

#else
  } else if (!strcmp(subcommand, "start") && output) {
    if (argc != 0 && !strcmp(argv[0], "--help")) {
start_usage:
      fprintf(stderr, "Usage: " EXENAME " start [-d/--daemon] [-n <interface-name>] [<filename>]\n");
      return 0;
    }
    for (; argc; argc--, argv++) {
      char *arg = argv[0];
      if (strcmp(arg, "-d") == 0 || strcmp(arg, "--daemon") == 0) {
        output->daemon = true;
        continue;
      }
      if (strcmp(arg, "-n") == 0) {
        if (argc < 2) goto start_usage;
        output->interface_name = argv[1];
        argc--,argv++;
        continue;
      }
      break;
    }
    if (argc > 1) goto start_usage;
    output->filename_to_load = (argc == 0) ? "" : argv[0];
    return 0;
#endif  // defined(OS_WIN)
  } else if (!strcmp(subcommand, "stop")) {
    return HandleStopCommand(argc, argv);
  } else if(!strcmp(subcommand, "genkey")) {
    if (argc != 0) {
      fprintf(stderr, "Usage: " EXENAME " genkey\n");
      return 1;
    }
    OsGetRandomBytes(key, 32);
    curve25519_normalize(key);
    ansi_printf("%s\n", base64_encode(key, 32, base64buf, sizeof(base64buf), NULL));

  } else if (!strcmp(subcommand, "genpsk")) {
    if (argc != 0) {
      fprintf(stderr, "Usage: " EXENAME " genpsk\n");
      return 1;
    }
    OsGetRandomBytes(key, 32);
    ansi_printf("%s\n", base64_encode(key, 32, base64buf, sizeof(base64buf), NULL));
  } else if (!strcmp(subcommand, "pubkey")) {
    char base64[WG_PUBLIC_KEY_LEN_BASE64 + 2];
    size_t n = fread(base64, 1, sizeof(base64), stdin);
    if (n < sizeof(base64) - 2 || n >= sizeof(base64) || 
        (n == sizeof(base64) - 1 && (base64[WG_PUBLIC_KEY_LEN_BASE64] != ' ' && base64[WG_PUBLIC_KEY_LEN_BASE64] != '\n'))) {
      fprintf(stderr, EXENAME ": Incorrect key format\n");
      return 1;
    }
    size_t size = 32;
    if (!base64_decode((uint8*)base64, n, key, &size) || size != 32) {
      fprintf(stderr, EXENAME ": Incorrect key format\n");
      return 1;
    }
    curve25519_donna(key, key, kCurve25519Basepoint);
    ansi_printf("%s\n", base64_encode(key, 32, base64buf, sizeof(base64buf), NULL));
  } else if (!strcmp(subcommand, "--help")) {
    ShowHelp();
  } else if (!strcmp(subcommand, "--version")) {
    ansi_printf("%s\n", TUNSAFE_VERSION_STRING);
  } else {
    if (argc == 0) {
      if (output)
        output->filename_to_load = subcommand;
    } else {
      ShowHelp();
    }
    return -1;
  }
  return 0;
}

#if defined(OS_WIN)

// This is ugly but all 3rd party terminals I found hide cmd.exe
static bool ConsoleSupportsColorCodes() {
  HWND wnd = GetConsoleWindow();
  return wnd && !(GetWindowLong(wnd, GWL_STYLE) & WS_VISIBLE);
}

// This is integrated into the main tunsafe binary on posix systems
int main(int argc, char **argv) {

  // Enable color codes on Windows 10+
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;
  GetConsoleMode(hOut, &dwMode);
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, dwMode);

  // Use colors depending on TUNSAFE_COLOR
  const char *env = getenv("TUNSAFE_COLOR");
  if (env) {
    g_supports_ansi_color = atoi(env) != 0;
  } else {
    g_supports_ansi_color = GetConsoleMode(hOut, &dwMode) && (dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) || ConsoleSupportsColorCodes();
  }

  int rv = HandleCommandLine(argc, argv, NULL);
  if (rv == -1) {
    fprintf(stderr, "Invalid subcommand '%s'\n", argv[1]);
    ShowHelp();
    return 1;
  }
  return rv;
}
#endif  // defined(OS_WIN)
