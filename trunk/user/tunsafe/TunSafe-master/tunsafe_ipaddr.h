#ifndef TUNSAFE_IPADDR_H_
#define TUNSAFE_IPADDR_H_

#include "tunsafe_types.h"
#include <vector>
#if !defined(OS_WIN)
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

union IpAddr {
  short int sin_family;
  sockaddr_in sin;
  sockaddr_in6 sin6;
};

struct WgCidrAddr {
  uint8 addr[16];
  uint8 size;
  uint8 cidr;
};

class DnsResolver;

#define kSizeOfAddress 64
const char *print_ip_prefix(char buf[kSizeOfAddress], int family, const void *ip, int prefixlen);
char *PrintIpAddr(const IpAddr &addr, char buf[kSizeOfAddress]);
char *PrintWgCidrAddr(const WgCidrAddr &addr, char buf[kSizeOfAddress]);
bool ParseCidrAddr(const char *s, WgCidrAddr *out);

bool IsWgCidrAddrSubsetOfAny(const WgCidrAddr &inner, const std::vector<WgCidrAddr> &addr);

enum {
  kParseSockaddrDontDoNAT64 = 1,
};
bool ParseSockaddrInWithPort(const char *s, IpAddr *sin, DnsResolver *resolver, int flags = 0);
bool ParseSockaddrInWithoutPort(char *s, IpAddr *sin, DnsResolver *resolver, int flags = 0);

// Returns nonzero if two endpoints are different.
uint32 CompareIpAddr(const IpAddr *a, const IpAddr *b);

#endif  // TUNSAFE_IPADDR_H_
