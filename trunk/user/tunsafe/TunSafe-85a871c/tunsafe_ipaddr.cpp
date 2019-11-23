#include "stdafx.h"
#include "tunsafe_ipaddr.h"
#include "tunsafe_dnsresolve.h"

#if defined(OS_WIN)
#include "network_win32_dnsblock.h"
#include <malloc.h>
#endif

#if defined(OS_POSIX)
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif  // defined(OS_POSIX)

#include <stdlib.h>
#include "util.h"

const char *print_ip_prefix(char buf[kSizeOfAddress], int family, const void *ip, int prefixlen) {
  // cast to void* to work on VS2015
  if (!inet_ntop(family, (void*)ip, buf, kSizeOfAddress - 8)) {
    memcpy(buf, "unknown", 8);
  }
  if (prefixlen >= 0)
    snprintf(buf + strlen(buf), 8, "/%d", prefixlen);
  return buf;
}

char *PrintIpAddr(const IpAddr &addr, char buf[kSizeOfAddress]) {
  if (addr.sin.sin_family == AF_INET) {
    print_ip_prefix(buf, addr.sin.sin_family, &addr.sin.sin_addr, -1);
  } else if (addr.sin.sin_family == AF_INET6) {
    print_ip_prefix(buf, addr.sin.sin_family, &addr.sin6.sin6_addr, -1);
  } else {
    buf[0] = 0;
  }
  return buf;
}


char *PrintWgCidrAddr(const WgCidrAddr &addr, char buf[kSizeOfAddress]) {
  if (addr.size == 32) {
    print_ip_prefix(buf, AF_INET, addr.addr, addr.cidr);
  } else if (addr.size == 128) {
    print_ip_prefix(buf, AF_INET6, addr.addr, addr.cidr);
  } else {
    buf[0] = 0;
  }
  return buf;
}

struct Addr {
  byte addr[4];
  uint8 cidr;
};

bool ParseCidrAddr(const char *s, WgCidrAddr *out) {
  const char *slash = strchr(s, '/');
  if (!slash)
    return false;

  size_t len = slash - s;
  char *tmp = (char*)alloca(len + 1);
  tmp[len] = 0;
  memcpy(tmp, s, len);

  int e = atoi(slash + 1);
  if (e < 0) return false;

  if (inet_pton(AF_INET, tmp, out->addr) == 1) {
    if (e > 32) return false;
    out->cidr = e;
    out->size = 32;
    return true;
  }
  if (inet_pton(AF_INET6, tmp, out->addr) == 1) {
    if (e > 128) return false;
    out->cidr = e;
    out->size = 128;
    return true;
  }
  return false;
}

static inline bool CheckFirstNbitsEquals(const byte *a, const byte *b, size_t n) {
  return memcmp(a, b, n >> 3) == 0 && ((n & 7) == 0 || !((a[n >> 3] ^ b[n >> 3]) & (0xff << (8 - (n & 7)))));
}

static bool IsWgCidrAddrSubsetOf(const WgCidrAddr &inner, const WgCidrAddr &outer) {
  return inner.size == outer.size && inner.cidr >= outer.cidr &&
    CheckFirstNbitsEquals(inner.addr, outer.addr, outer.cidr);
}

bool IsWgCidrAddrSubsetOfAny(const WgCidrAddr &inner, const std::vector<WgCidrAddr> &addr) {
  for (auto &a : addr)
    if (IsWgCidrAddrSubsetOf(inner, a))
      return true;
  return false;
}

// Returns nonzero if two endpoints are different.
uint32 CompareIpAddr(const IpAddr *a, const IpAddr *b) {
  uint32 rv = b->sin.sin_family ^ a->sin.sin_family;
  if (b->sin.sin_family != AF_INET6) {
    rv |= b->sin.sin_addr.s_addr ^ a->sin.sin_addr.s_addr;
    rv |= b->sin.sin_port ^ a->sin.sin_port;
  } else {
    uint64 rx = ((uint64*)&b->sin6.sin6_addr)[0] ^ ((uint64*)&a->sin6.sin6_addr)[0];
    rx |= ((uint64*)&b->sin6.sin6_addr)[1] ^ ((uint64*)&a->sin6.sin6_addr)[1];
    rv |= rx | (rx >> 32);
    rv |= b->sin6.sin6_port ^ a->sin6.sin6_port;
  }
  return rv;
}


static Mutex g_dns_mutex;

// This starts a background thread for running DNS resolving.
class DnsResolverThread : private Thread::Runner {
public:
  DnsResolverThread();
  ~DnsResolverThread();

  // Resolve the hostname and store the result in |result|.
  // The function will block until it's resolved. If the cancellation
  // token or becomes signalled, the call will fail.
  bool Resolve(const char *hostname, IpAddr *result, DnsResolverCanceller *token);

private:
  virtual void ThreadMain();
  void StartThread();

  struct Entry {
    enum {
      // Set when it's been posted to the job queue
      POSTED = 0,
      // Set when the thread has finished and original thread should delete
      COMPLETE = 1,
      // Set when the original thread has cancelled and worker thread should delete
      CANCELLED = 2,
    };

    Entry() : hostname(NULL) {}
    ~Entry() { free(hostname); }

    char *hostname;
    IpAddr *result;
    Entry *next;
    uint32 state;
    ConditionVariable *condvar;
  };
  Entry *entry_;
  Thread thread_;
  bool thread_active_;
};

DnsResolverThread::DnsResolverThread() {
  thread_active_ = false;
  entry_ = NULL;
}

DnsResolverThread::~DnsResolverThread() {
  assert(entry_ == NULL);
  thread_.StopThread();
}

void DnsResolverCanceller::Cancel() {
  g_dns_mutex.Acquire();
  cancel_ = true;
  condvar_.Wake();
  g_dns_mutex.Release();
}

void DnsResolverCanceller::CancelSleepOnce() {
  g_dns_mutex.Acquire();
  cancel_sleep_once_ = true;
  condvar_.Wake();
  g_dns_mutex.Release();
}



bool DnsResolverThread::Resolve(const char *hostname, IpAddr *result, DnsResolverCanceller *token) {
  if (token->cancel_)
    return false;

  Entry *e = new Entry;
  e->hostname = _strdup(hostname);
  e->result = result;
  e->next = NULL;
  e->state = Entry::POSTED;
  e->condvar = &token->condvar_;
  result->sin.sin_family = 0;

  // Push it to the queue and start thread
  g_dns_mutex.Acquire();
  Entry **p = &entry_;
  while (*p) p = &(*p)->next;
  *p = e;
  if (!thread_active_)
    StartThread();
  // Wait for something to happen with it.
  while (!token->cancel_ && e->state == Entry::POSTED)
    token->condvar_.Wait(&g_dns_mutex);
  if (e->state == Entry::COMPLETE) {
    delete e;
  } else {
    e->state = Entry::CANCELLED;
  }
  g_dns_mutex.Release();
  return result->sin.sin_family != 0;
}

void DnsResolverThread::StartThread() {
  thread_.StopThread();
  thread_active_ = true;
  thread_.StartThread(this);
}

void DnsResolverThread::ThreadMain() {
  Entry *e;
  struct addrinfo *ai;
  g_dns_mutex.Acquire();
  while ((e = entry_) != NULL) {
    entry_ = e->next;
    g_dns_mutex.Release();

    struct addrinfo hints = {0};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    // AI_V4MAPPED doesn't work on Android?!
#if defined(OS_IOS)
    hints.ai_flags = AI_DEFAULT;
#else
    hints.ai_flags = AI_ADDRCONFIG ;
#endif
    ai = NULL;
    int r =  getaddrinfo(e->hostname, NULL, &hints, &ai);
    if (r != 0)
      ai = NULL;
//    RINFO("r=%d errno=%d, %s", r, errno, gai_strerror(r));
    //    he = gethostbyname(e->hostname);
    g_dns_mutex.Acquire();
    if (e->state == Entry::CANCELLED) {
      delete e;
    } else {
//      RINFO("ai=%p, family=%d", ai, ai ? ai->ai_family : -1);
      if (ai) {
        e->result->sin.sin_family = ai->ai_family;
        e->result->sin.sin_port = 0;
        if (ai->ai_family == AF_INET)
          memcpy(&e->result->sin.sin_addr, &((sockaddr_in*)ai->ai_addr)->sin_addr, 4);
        else
          memcpy(&e->result->sin6.sin6_addr, &((sockaddr_in6*)ai->ai_addr)->sin6_addr, 16);
      }
/*      if (he) {
        e->result->sin.sin_family = AF_INET;
        e->result->sin.sin_port = 0;
        memcpy(&e->result->sin.sin_addr, he->h_addr_list[0], 4);
      }*/
      
      e->state = Entry::COMPLETE;
      e->condvar->Wake();
    }
    
    if (ai)
      freeaddrinfo(ai);
  }
  thread_active_ = false;
  g_dns_mutex.Release();
}

static DnsResolverThread g_dnsresolver_thread;

static bool InterruptibleSleep(int delay, DnsResolverCanceller *token) {
  g_dns_mutex.Acquire();
  uint32 time_at_start = (uint32)OsGetMilliseconds();
  while (delay > 0 && !token->cancel_) {
    if (token->cancel_sleep_once_) {
      token->cancel_sleep_once_ = false;
      delay = 0;
      break;
    }
    token->condvar_.WaitTimed(&g_dns_mutex, delay);
    uint32 now = (uint32)OsGetMilliseconds();
    delay -= (now - time_at_start);
    time_at_start = now;
  }
  g_dns_mutex.Release();
  return (delay <= 0);
}

DnsResolver::DnsResolver(DnsBlocker *dns_blocker) {
  dns_blocker_ = dns_blocker;
}

DnsResolver::~DnsResolver() {
}

void DnsResolver::ClearCache() {
  cache_.clear();
}

bool DnsResolver::Resolve(const char *hostname, IpAddr *result) {
  static const uint8 retry_delays[] = {1, 2, 3, 5, 10, 20, 40, 60, 120, 180, 255};
  char buf[kSizeOfAddress];

  retry_attempt_ = 0;

  memset(result, 0, sizeof(IpAddr));

  // First check cache
  for (auto it = cache_.begin(); it != cache_.end(); ++it) {
    if (it->name == hostname) {

      *result = it->ip;
      RINFO("Resolved %s to %s%s", hostname, PrintIpAddr(*result, buf), " (cached)");
      return true;
    }
  }

#if defined(OS_WIN)
  // Then disable dns blocker (otherwise the windows dns client service can't resolve)
  if (dns_blocker_ && dns_blocker_->IsActive()) {
    RINFO("Disabling DNS blocker to resolve %s", hostname);
    dns_blocker_->RestoreDns();
  }
#endif  // defined(OS_WIN)

  for (;;) {
    if (g_dnsresolver_thread.Resolve(hostname, result, &token_)) {
      // add to cache
      cache_.emplace_back(hostname, *result);
      RINFO("Resolved %s to %s%s", hostname, PrintIpAddr(*result, buf), "");
      return true;
    }
    if (token_.is_cancelled())
      return false;

    RINFO("Unable to resolve %s. Trying again in %d second(s)", hostname, retry_delays[retry_attempt_]);
    if (!InterruptibleSleep(retry_delays[retry_attempt_] * 1000, &token_))
      return false;

    if (retry_attempt_ != ARRAY_SIZE(retry_delays) - 1)
      retry_attempt_++;
  }
}

void DnsResolver::RetryNow() {
  retry_attempt_ = 0;
  token_.CancelSleepOnce();
}

// Parse an IPV4 address into sin, doing NAT64 translation if applicable (on IOS)
static bool ParseIpv4WithNAT64Translation(const char *s, IpAddr *sin, int flags) {
  // First verify it's actually a valid ipv4 address to prevent getaddrinfo from doing a slow resolve
  if (inet_pton(AF_INET, s, &sin->sin.sin_addr) != 1)
    return false;
  sin->sin.sin_family = AF_INET;

#if defined(OS_IOS)
  if (!(flags & kParseSockaddrDontDoNAT64)) {
    struct addrinfo hints = {0};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_DEFAULT;
    struct addrinfo* ai = NULL;
    // When NAT64 is enabled, I don't get an IPv4 address back.
    if (getaddrinfo(s, NULL, &hints, &ai) == 0) {
      // check so we have an AF_INET6 and no AF_INET
      struct sockaddr_in6 *found = NULL;
      for(struct addrinfo *t = ai; t; t = t->ai_next) {
        if (t->ai_family == AF_INET) { found = NULL; break; }
        if (t->ai_family == AF_INET6) found = (sockaddr_in6*)t->ai_addr;
      }
      if (found) {
        memset(&sin->sin.sin_addr, 0, 4); // clear out ipv4 address already written
        memcpy(&sin->sin6.sin6_addr, &found->sin6_addr, 16);
        sin->sin.sin_family = AF_INET6;
      }
      freeaddrinfo(ai);
    }
  }
#endif  // defined(OS_IOS)
  return true;
}

bool ParseSockaddrInWithPort(const char *si, IpAddr *sin, DnsResolver *resolver, int flags) {
  size_t len = strlen(si) + 1;
  char *s = (char*)alloca(len);
  memcpy(s, si, len);

  memset(sin, 0, sizeof(IpAddr));
  if (*s == '[') {
    char *end = strchr(s, ']');
    if (end == NULL)
      return false;
    *end = 0;
    if (inet_pton(AF_INET6, s + 1, &sin->sin6.sin6_addr) != 1)
      return false;
    char *x = strchr(end + 1, ':');
    if (!x)
      return false;
    sin->sin6.sin6_family = AF_INET6;
    sin->sin6.sin6_port = htons(atoi(x + 1));
    return true;
  }
  char *x = strchr(s, ':');
  if (!x) return false;
  *x = 0;

  if (!ParseIpv4WithNAT64Translation(s, sin, flags)) {
    if (!resolver) {
      return false;
    } else if (!resolver->Resolve(s, sin)) {
      RERROR("Unable to resolve %s", s);
      return false;
    }
  }
  sin->sin.sin_port = htons(atoi(x + 1));
  return true;
}

bool ParseSockaddrInWithoutPort(char *s, IpAddr *sin, DnsResolver *resolver, int flags) {
  memset(sin, 0, sizeof(IpAddr));

  if (inet_pton(AF_INET6, s, &sin->sin6.sin6_addr) == 1) {
    sin->sin.sin_family = AF_INET6;
    return true;
  } else if (ParseIpv4WithNAT64Translation(s, sin, flags)) {
    return true;
  } else if (!resolver) {
    return false;
  } else if (!resolver->Resolve(s, sin)) {
    RERROR("Unable to resolve %s", s);
    return false;
  }
  return true;
}
