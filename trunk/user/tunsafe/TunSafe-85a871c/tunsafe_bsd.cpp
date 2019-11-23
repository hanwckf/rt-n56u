// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "tunsafe_bsd.h"
#include "tunsafe_endian.h"
#include "tunsafe_wg_plugin.h"
#include "util.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>

#include <sys/socket.h>
#include <net/route.h>
#include <sys/time.h>

#include <pthread.h>

#if defined(OS_MACOSX)
#include <sys/kern_control.h>
#include <net/if_utun.h>
#include <sys/sys_domain.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <net/if_dl.h>
#elif defined(OS_FREEBSD)
#include <net/if_tun.h>
#include <net/if_dl.h>
#elif defined(OS_LINUX)
//#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/prctl.h>
#include <linux/rtnetlink.h>
#include <sys/inotify.h>
#include <limits.h>
#endif

static bool g_daemon_mode;

#if defined(OS_MACOSX) || defined(OS_FREEBSD)
struct MyRouteMsg {
  struct rt_msghdr hdr;
  uint32 pad;
  struct sockaddr_in target;
  struct sockaddr_in netmask;
};

struct MyRouteReply {
  struct rt_msghdr hdr;
  uint8 buf[512];
};

// Zero gets rounded up
#if defined(OS_MACOSX)
#define RTMSG_ROUNDUP(a) ((a) ? ((((a) - 1) | (sizeof(uint32_t) - 1)) + 1) : sizeof(uint32_t))
#else
#define RTMSG_ROUNDUP(a) ((a) ? ((((a) - 1) | (sizeof(long) - 1)) + 1) : sizeof(long))
#endif


static bool GetDefaultRoute(char *iface, size_t iface_size, uint32 *gw_addr) {
  int fd, pid, len;

  union {
    MyRouteMsg rt;
    MyRouteReply rep;
  };

  fd = socket(PF_ROUTE, SOCK_RAW, AF_INET);
  if (fd < 0)
    return false;

  memset(&rt, 0, sizeof(rt));

  rt.hdr.rtm_type = RTM_GET;
  rt.hdr.rtm_flags = RTF_UP | RTF_GATEWAY;
  rt.hdr.rtm_version = RTM_VERSION;
  rt.hdr.rtm_seq = 0;
  rt.hdr.rtm_addrs = RTA_DST | RTA_NETMASK | RTA_IFP;

  rt.target.sin_family = AF_INET;
  rt.netmask.sin_family = AF_INET;

  rt.target.sin_len = sizeof(struct sockaddr_in);
  rt.netmask.sin_len = sizeof(struct sockaddr_in);

  rt.hdr.rtm_msglen = sizeof(rt);

  if (write(fd, (char*)&rt, sizeof(rt)) != sizeof(rt)) {
    RERROR("PF_ROUTE write failed.");
    close(fd);
    return false;
  }

  pid = getpid();
  do {
    len = read(fd, (char *)&rep, sizeof(rep));
    if (len <= 0) {
      RERROR("PF_ROUTE read failed.");
      close(fd);
      return false;
    }
  } while (rep.hdr.rtm_seq != 0 || rep.hdr.rtm_pid != pid);
  close(fd);

  const struct sockaddr_dl *ifp = NULL;
  const struct sockaddr_in *gw = NULL;

  uint8 *pos = rep.buf;
  for (int i = 1; i && i < rep.hdr.rtm_addrs; i <<= 1) {
    if (rep.hdr.rtm_addrs & i) {
      if (1 > rep.buf + 512 - pos)
        break; // invalid
      size_t len = RTMSG_ROUNDUP(((struct sockaddr*)pos)->sa_len);
      if (len > rep.buf + 512 - pos)
        break; // invalid
               //      RINFO("rtm %d %d", i, ((struct sockaddr*)pos)->sa_len);
      if (i == RTA_IFP && ((struct sockaddr*)pos)->sa_len >= sizeof(struct sockaddr_dl)) {
        ifp = (struct sockaddr_dl *)pos;
      } else if (i == RTA_GATEWAY && ((struct sockaddr*)pos)->sa_len >= sizeof(struct sockaddr_in)) {
        gw = (struct sockaddr_in *)pos;

      }
      pos += len;
    }
  }

  if (ifp && ifp->sdl_nlen && ifp->sdl_nlen < iface_size) {
    iface[ifp->sdl_nlen] = 0;
    memcpy(iface, ifp->sdl_data, ifp->sdl_nlen);
    if (gw && gw->sin_family == AF_INET) {
      *gw_addr = ReadBE32(&gw->sin_addr);
      return true;
    }

  }
  //  RINFO("Read %d %d %d", len, rep.hdr.rtm_addrs, (int)sizeof(struct rt_msghdr ));
  return false;
}
#endif  // defined(OS_MACOSX) || defined(OS_FREEBSD)

#if defined(OS_LINUX)
struct LinuxParsedRoute {
  int has;
  struct in_addr dst, gateway;
  char ifname[IF_NAMESIZE];
};

static bool ParseLinuxRoutes(struct nlmsghdr *nl, struct LinuxParsedRoute *result) {
  struct rtmsg *rt = (struct rtmsg *)NLMSG_DATA(nl);
  if (rt->rtm_family != AF_INET || rt->rtm_table != RT_TABLE_MAIN)
    return false;

  struct rtattr *attr = (struct rtattr *)RTM_RTA(rt);
  int len = RTM_PAYLOAD(nl);
  int has = 0;
  for(; RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
    switch(attr->rta_type) {
    case RTA_OIF:
      has |= 1;
      if_indextoname(*(int *)RTA_DATA(attr), result->ifname);
      break;
    case RTA_GATEWAY:
      has |= 2;
      memcpy(&result->gateway, RTA_DATA(attr), sizeof(result->gateway));
      break;
    case RTA_DST:
      has |= 4;
      memcpy(&result->dst, RTA_DATA(attr), sizeof(result->dst));
      break;
    }
  }
  result->has = has;
  return true;
}

static bool GetDefaultRoute(char *iface, size_t iface_size, uint32 *gw_addr) {
  enum {BUFSIZE = 8192};
  struct nlmsghdr *nl;
  struct rtmsg *rt;
  struct LinuxParsedRoute parsed_route;
  char buffer[BUFSIZE];
  int fd, len, pid = getpid();
  bool result = false;

  if ((fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
    return false;

  size_t msg_size = NLMSG_SPACE(sizeof(struct rtmsg));
  memset(buffer, 0, msg_size);
  nl = (struct nlmsghdr *)buffer;
  nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  nl->nlmsg_type = RTM_GETROUTE;
  nl->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
  nl->nlmsg_seq = 1;
  nl->nlmsg_pid = pid;
  rt = (struct rtmsg *)NLMSG_DATA(nl);
  rt->rtm_family = AF_INET;
  rt->rtm_table = RT_TABLE_MAIN;
  if (send(fd, nl, msg_size, 0) != msg_size) {
    RERROR("write to route socket failed");
    goto done;
  }
  do {
    if ((len = recv(fd, buffer, BUFSIZE, 0)) < 0) {
      RERROR("read from route socket failed");
      goto done;
    }
    for (nl = (struct nlmsghdr *)buffer; NLMSG_OK(nl, len); nl = NLMSG_NEXT(nl, len)) {
      if (nl->nlmsg_seq != 1 || nl->nlmsg_pid != pid)
        continue;
      if (nl->nlmsg_type == NLMSG_DONE)
        goto done;
      if (nl->nlmsg_type == NLMSG_ERROR) {
        RERROR("Error in recieved packet");
        goto done;
      }
      if (ParseLinuxRoutes(nl, &parsed_route) && (parsed_route.has & (1+2+4)) == (1+2)) {
        size_t l = strlen(parsed_route.ifname);
        if (l < iface_size) {
          *gw_addr = ReadBE32(&parsed_route.gateway);
          memcpy(iface, parsed_route.ifname, l + 1);
          result = true;
        }
      }
    }
  } while ((nl->nlmsg_flags & NLM_F_MULTI) != 0);
done:
  close(fd);
  return result;
}
#endif  // defined(OS_LINUX)

#if defined(OS_MACOSX)
int open_tun(char *devname, size_t devname_size) {
  struct sockaddr_ctl sc;
  struct ctl_info ctlinfo = {0};
  int fd;

  memcpy(ctlinfo.ctl_name, UTUN_CONTROL_NAME, sizeof(UTUN_CONTROL_NAME));

  for(int i = 0; i < 256; i++) {
    fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    if (fd < 0) {
      RERROR("socket(SYSPROTO_CONTROL) failed");
      return -1;
    }

    if (ioctl(fd, CTLIOCGINFO, &ctlinfo) == -1) {
      RERROR("ioctl(CTLIOCGINFO) failed: %d", errno);
      close(fd);
      return -1;
    }
    sc.sc_id = ctlinfo.ctl_id;
    sc.sc_len = sizeof(sc);
    sc.sc_family = AF_SYSTEM;
    sc.ss_sysaddr = AF_SYS_CONTROL;
    sc.sc_unit = i + 1;
    if (connect(fd, (struct sockaddr *)&sc, sizeof(sc)) == 0) {
      socklen_t devname_size2 = devname_size;
      if (getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, devname, &devname_size2)) {
        RERROR("getsockopt(UTUN_OPT_IFNAME) failed");
        close(fd);
        return -1;
      }


      return fd;
    }
    close(fd);
  }
  return -1;  
}

#elif defined(OS_FREEBSD)
int open_tun(char *devname, size_t devname_size) {
  char buf[32];
  int tun_fd;
  // First open an existing tun device
  for(int i = 0; i < 256; i++) {
    sprintf(buf, "/dev/tun%d", i);
    tun_fd = open(buf, O_RDWR);
    if (tun_fd >= 0) goto did_open;
  }
  tun_fd = open("/dev/tun", O_RDWR);
  if (tun_fd < 0)
    return tun_fd;
did_open:
  if (!fdevname_r(tun_fd, devname, devname_size)) {
    RERROR("Unable to get name of tun device");
    close(tun_fd);
    return -1;
  }
  int flags = IFF_POINTOPOINT | IFF_MULTICAST;
  if (ioctl(tun_fd, TUNSIFMODE, &flags) < 0) {
    RERROR("ioctl(TUNSIFMODE) failed");
    close(tun_fd);
    return -1;

  }
  flags = 1;
  if (ioctl(tun_fd, TUNSIFHEAD, &flags) < 0) {
    RERROR("ioctl(TUNSIFHEAD) failed");
    close(tun_fd);
    return -1;
  }
  return tun_fd;
}

#elif defined(OS_LINUX)
int open_tun(char *devname, size_t devname_size) {
  int fd, err;
  struct ifreq ifr;

  fd = open("/dev/net/tun", O_RDWR);
  if (fd < 0)
    return fd;

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  my_strlcpy(ifr.ifr_name, sizeof(ifr.ifr_name), devname);
  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
    close(fd);
    return err;
  }
  my_strlcpy(devname, devname_size, ifr.ifr_name);
  return fd;
}
#endif

TunsafeBackendBsd::TunsafeBackendBsd() {
  devname_[0] = 0;
  tun_interface_gone_ = false;
}

TunsafeBackendBsd::~TunsafeBackendBsd() {
}

static uint32 CidrToNetmaskV4(int cidr) {
  return cidr == 32 ? 0xffffffff : 0xffffffff << (32 - cidr);
}

void TunsafeBackendBsd::AddRoute(uint32 ip, uint32 cidr, uint32 gw, const char *dev) {
  uint32 ip_be, gw_be;
  WriteBE32(&ip_be, ip);
  WriteBE32(&gw_be, gw);
  AddRoute(AF_INET, &ip_be, cidr, &gw_be, dev);
}

static void AddOrRemoveRoute(const RouteInfo &cd, bool remove) {
  char buf1[kSizeOfAddress], buf2[kSizeOfAddress];

  print_ip_prefix(buf1, cd.family, cd.ip, cd.cidr);
  print_ip_prefix(buf2, cd.family, cd.gw, -1);

#if defined(OS_LINUX)
  const char *cmd = remove ? "del" : "add";
  const char *proto = (cd.family == AF_INET) ? NULL : "-6";
  if (cd.dev.empty()) {
    RunCommand("/bin/ip %s route %s %s via %s", proto, cmd, buf1, buf2);
  } else {
    RunCommand("/bin/ip %s route %s %s dev %s", proto, cmd, buf1, cd.dev.c_str());
  }
#elif defined(OS_MACOSX) || defined(OS_FREEBSD)
  const char *cmd = remove ? "delete" : "add";
  if (cd.family == AF_INET) {
    RunCommand("/bin/route -q %s %s %s", cmd, buf1, buf2);
  } else {
    RunCommand("/bin/route -q %s -inet6 %s %s", cmd, buf1, buf2);
  }
#endif
}

bool TunsafeBackendBsd::AddRoute(int family, const void *dest, int dest_prefix, const void *gateway, const char *dev) {
  RouteInfo c;

  c.dev = dev ? dev : "";
  c.family = family;
  size_t len = (family == AF_INET) ? 4 : 16;
  memcpy(c.ip, dest, len);
  memcpy(c.gw, gateway, len);
  c.cidr = dest_prefix;
  cleanup_commands_.push_back(c);
  AddOrRemoveRoute(c, false);
  return true;
}

void TunsafeBackendBsd::DelRoute(const RouteInfo &cd) {
  AddOrRemoveRoute(cd, true);
}

static bool IsIpv6AddressSet(const void *p) {
  return (ReadLE64(p) | ReadLE64((char*)p + 8)) != 0;
}
 
// Called to initialize tun
bool TunsafeBackendBsd::Configure(const TunConfig &&config, TunConfigOut *out) override {
  char buf[kSizeOfAddress];
  char buf2[kSizeOfAddress];

  if (!RunPrePostCommand(config.pre_post_commands.pre_up)) {
    RERROR("Pre command failed!");
    return false;
  }

  out->enable_neighbor_discovery_spoofing = false;

  if (!InitializeTun(devname_))
    return false;

  const WgCidrAddr *ipv4_addr = NULL;
  const WgCidrAddr *ipv6_addr = NULL;
  for (auto it = config.addresses.begin(); it != config.addresses.end(); ++it) {
    if (it->size == 32 && ipv4_addr == NULL)
      ipv4_addr = &*it;
    else if (it->size == 128 && ipv6_addr == NULL)
      ipv6_addr = &*it;
  }
  if (ipv4_addr == NULL) {
    RERROR("The TUN adapter requires an IPv4 address");
    return false;
  }
  uint32 ipv4_netmask = CidrToNetmaskV4(ipv4_addr->cidr);
  uint32 ipv4_ip = ReadBE32(ipv4_addr->addr);

  addresses_to_remove_ = config.addresses;

#if defined(OS_LINUX)
  RunCommand("/bin/ip address flush dev %s scope global", devname_);
  for(const WgCidrAddr &a : config.addresses)
    RunCommand("/bin/ip address add dev %s %s", devname_, print_ip_prefix(buf, a.size == 32 ? AF_INET : AF_INET6, a.addr, a.cidr));
  RunCommand("/bin/ip link set dev %s mtu %d up", devname_, config.mtu);
#else
  for(const WgCidrAddr &a : config.addresses) {
    if (a.size == 32) {
      RunCommand("/bin/ifconfig %s inet %s %s add", devname_, print_ip_prefix(buf, AF_INET, a.addr, a.cidr), print_ip_prefix(buf2, AF_INET, a.addr, -1));
    } else {
      RunCommand("/bin/ifconfig %s inet6 %s add", devname_, print_ip_prefix(buf, AF_INET6, a.addr, a.cidr));
    }
  }
  RunCommand("/sbin/ifconfig %s mtu %d up", devname_, config.mtu);
#endif


  char default_iface[16];
  uint32 ipv4_default_gw;
  bool found_ipv4_route = GetDefaultRoute(default_iface, sizeof(default_iface), &ipv4_default_gw);
  for (auto it = config.excluded_routes.begin(); it != config.excluded_routes.end(); ++it) {
    if (it->size == 32) {
      if (!found_ipv4_route) {
        RERROR("Unable to determine default interface.");
        return false;
      }
      AddRoute(ReadBE32(it->addr), it->cidr, ipv4_default_gw, NULL);
    } else if (it->size == 128) {
      RERROR("default_route_endpoint_v6 not supported");
      return false;
    }
  }

  // Add all the extra routes
  for (auto it = config.included_routes.begin(); it != config.included_routes.end(); ++it) {
    if (it->cidr == 0) {
      if (it->size == 32) {
        AddRoute(0x00000000, 1, ipv4_ip, devname_);
        AddRoute(0x80000000, 1, ipv4_ip, devname_);
      } else if (it->size == 128 && ipv6_addr) {
        static const uint8 matchall_1_route[17] = {0x80, 0, 0, 0};
        AddRoute(AF_INET6, matchall_1_route + 1, 1, ipv6_addr->addr, devname_);
        AddRoute(AF_INET6, matchall_1_route + 0, 1, ipv6_addr->addr, devname_);
      }
      continue;
    }

    // On linux, don't add a route that equals one of the addresses
#if defined(OS_LINUX)
    if (IsWgCidrAddrSubsetOfAny(*it, config.addresses))
      continue;
#endif

    if (it->size == 32) {
      AddRoute(ReadBE32(it->addr), it->cidr, ipv4_ip, devname_);
    } else if (it->size == 128 && ipv6_addr) {
      AddRoute(AF_INET6, it->addr, it->cidr, ipv6_addr->addr, devname_);
    }
  }

  RunPrePostCommand(config.pre_post_commands.post_up);

  pre_down_ = std::move(config.pre_post_commands.pre_down);
  post_down_ = std::move(config.pre_post_commands.post_down);

  return true;
}

void TunsafeBackendBsd::CleanupRoutes() {
  char buf[kSizeOfAddress];

  RunPrePostCommand(pre_down_);

  for(auto it = cleanup_commands_.begin(); it != cleanup_commands_.end(); ++it) {
    if (!tun_interface_gone_ || strcmp(it->dev.c_str(), devname_) != 0)
      DelRoute(*it);
  }

#if defined(OS_LINUX)
  for(const WgCidrAddr &a : addresses_to_remove_)
    RunCommand("/bin/ip address del dev %s %s", devname_, print_ip_prefix(buf, a.size == 32 ? AF_INET : AF_INET6, a.addr, a.cidr));
#else
  for(const WgCidrAddr &a : addresses_to_remove_) {
    if (a.size == 32) {
      RunCommand("/sbin/ifconfig %s inet %s -alias", devname_, print_ip_prefix(buf, AF_INET, a.addr, -1));
    } else {
      RunCommand("/sbin/ifconfig %s inet6 %s -alias", devname_, print_ip_prefix(buf, AF_INET6, a.addr, -1));
    }
  }
#endif

  cleanup_commands_.clear();
  addresses_to_remove_.clear();

  RunPrePostCommand(post_down_);

  pre_down_.clear();
  post_down_.clear();
}

void TunsafeBackendBsd::SetTunDeviceName(const char *name) {
  my_strlcpy(devname_, sizeof(devname_), name);
}

static bool RunOneCommand(const std::string &cmd) {
  RINFO("Run: %s", cmd.c_str());
  int exit_code = system(cmd.c_str());
  if (exit_code) {
    RERROR("Run Failed (%d) : %s", exit_code, cmd.c_str());
    return false;
  }
  return true;
}

bool TunsafeBackendBsd::RunPrePostCommand(const std::vector<std::string> &vec) {
  bool success = true;
  for (auto it = vec.begin(); it != vec.end(); ++it) {
    success &= RunOneCommand(*it);
  }
  return success;
}


static SignalCatcher *g_signal_catcher;
static bool did_ctrlc;

void SignalCatcher::SigAlrm(int sig) {
  if (g_signal_catcher)
    *g_signal_catcher->sigalarm_flag_ = true;
}

void SignalCatcher::SigInt(int sig) {
  if (did_ctrlc)
    exit(1);
  did_ctrlc = true;
  write(1, "Ctrl-C detected. Exiting. Press again to force quit.\n", sizeof("Ctrl-C detected. Exiting. Press again to force quit.\n") - 1);
  // todo: fix signal safety?
  if (g_signal_catcher)
    *g_signal_catcher->exit_flag_ = true;
}

SignalCatcher::SignalCatcher(bool *exit_flag, bool *sigalarm_flag) {
  assert(g_signal_catcher == NULL);
  exit_flag_ = exit_flag;
  sigalarm_flag_ = sigalarm_flag;
  g_signal_catcher = this;

  sigset_t mask;

  // We want an alarm signal every second.
  {
    struct sigaction act = {0};
    act.sa_handler = SigAlrm;
    if (sigaction(SIGALRM, &act, NULL) < 0) {
      RERROR("Unable to install SIGALRM handler.");
      return;
    }
  }

  {
    struct sigaction act = {0};
    act.sa_handler = SigInt;
    if (sigaction(SIGINT, &act, NULL) < 0) {
      RERROR("Unable to install SIGINT handler.");
      return;
    }
  }
#if defined(OS_LINUX) || defined(OS_FREEBSD)
  sigemptyset(&mask);
  sigaddset(&mask, SIGALRM);
  if (sigprocmask(SIG_BLOCK, &mask, &orig_signal_mask_) < 0) {
    perror("sigprocmask");
    return;
  }

  {
    struct itimerspec tv = {0};
    struct sigevent sev;
    timer_t timer_id;

    tv.it_interval.tv_sec = 1;
    tv.it_value.tv_sec = 1;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = NULL;

    if (timer_create(CLOCK_MONOTONIC, &sev, &timer_id) < 0) {
      RERROR("timer_create failed");
      return;
    }

    if (timer_settime(timer_id, 0, &tv, NULL) < 0) {
      RERROR("timer_settime failed");
      return;
    }
  }
#elif defined(OS_MACOSX)
  ualarm(1000000, 1000000);
#endif
}

SignalCatcher::~SignalCatcher() {
  g_signal_catcher = NULL;
}

void InitCpuFeatures();
void Benchmark();

const char *print_ip(char buf[kSizeOfAddress], in_addr_t ip) {
  snprintf(buf, kSizeOfAddress, "%d.%d.%d.%d", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, (ip >> 0) & 0xff);
  return buf;
}

class TunsafeBackendBsdImpl : public TunsafeBackendBsd, public NetworkBsd::NetworkBsdDelegate, public ProcessorDelegate, public PluginDelegate {
public:
  TunsafeBackendBsdImpl();
  virtual ~TunsafeBackendBsdImpl();

  void RunLoop();
  virtual bool InitializeTun(char devname[16]) override;

  // -- from TunInterface
  virtual void WriteTunPacket(Packet *packet) override;

  // -- from UdpInterface
  virtual bool Configure(int listen_port_udp, int listen_port_tcp) override;
  virtual void WriteUdpPacket(Packet *packet) override;

  // -- from NetworkBsdDelegate
  virtual void OnSecondLoop(uint64 now) override;
  virtual void RunAllMainThreadScheduled() override;

  // -- from ProcessorDelegate
  virtual void OnConnected() override;
  virtual void OnConnectionRetry(uint32 attempts) override;

  // -- from PluginDelegate
  virtual void OnRequestToken(WgPeer *peer, uint32 type) override;

  WireguardProcessor *processor() { return &processor_; }

private:
  // Close all TCP connections that are not pointed to by any of the peer endpoint.
  void CloseOrphanTcpConnections();

  bool is_connected_;
  uint8 close_orphan_counter_;
  TunsafePlugin *plugin_;
  WireguardProcessor processor_;
  NetworkBsd network_;
  TunSocketBsd tun_;
  UdpSocketBsd udp_;
  UnixDomainSocketListenerBsd unix_socket_listener_;
  TcpSocketListenerBsd tcp_socket_listener_;
};

TunsafeBackendBsdImpl::TunsafeBackendBsdImpl() 
    : is_connected_(false),
      close_orphan_counter_(0),
      plugin_(CreateTunsafePlugin(this, &processor_)),
      processor_(this, this, this),
      network_(this, 1000),
      tun_(&network_, &processor_), 
      udp_(&network_, &processor_),
      unix_socket_listener_(&network_, &processor_),
      tcp_socket_listener_(&network_, &processor_) {
  processor_.dev().SetPlugin(plugin_);
}

TunsafeBackendBsdImpl::~TunsafeBackendBsdImpl() {
  delete plugin_;
}

bool TunsafeBackendBsdImpl::InitializeTun(char devname[16]) {
  int tun_fd = open_tun(devname, 16);
  if (tun_fd < 0) { RERROR("Error opening tun device"); return false; }
  if (!tun_.Initialize(tun_fd)) {
    close(tun_fd);
    return false;
  }
  unix_socket_listener_.Initialize(devname);
  return true;  
}

void TunsafeBackendBsdImpl::WriteTunPacket(Packet *packet) {
  tun_.WritePacket(packet);
}

// Called to initialize udp
bool TunsafeBackendBsdImpl::Configure(int listen_port, int listen_port_tcp) {
  return udp_.Initialize(listen_port) && 
         (listen_port_tcp == 0 || tcp_socket_listener_.Initialize(listen_port_tcp));
}

void TunsafeBackendBsdImpl::WriteUdpPacket(Packet *packet) {
  assert((packet->protocol & 0x7F) <= 2);
  if (packet->protocol & kPacketProtocolTcp) {
    TcpSocketBsd::WriteTcpPacket(&network_, &processor_, packet);
  } else {
    udp_.WritePacket(packet);
  }
}

void TunsafeBackendBsdImpl::RunLoop() {
  if (!unix_socket_listener_.Start(network_.exit_flag()))
    return;

  SignalCatcher signal_catcher(network_.exit_flag(), network_.sigalarm_flag());
  network_.RunLoop(&signal_catcher.orig_signal_mask_);
  unix_socket_listener_.Stop();

  tun_interface_gone_ = tun_.tun_interface_gone();
}

void TunsafeBackendBsdImpl::OnSecondLoop(uint64 now) {
  if (!(close_orphan_counter_++ & 0xF))
    CloseOrphanTcpConnections();
  processor_.SecondLoop();
}

void TunsafeBackendBsdImpl::RunAllMainThreadScheduled() {
  processor_.RunAllMainThreadScheduled();
}

void TunsafeBackendBsdImpl::OnConnected() {
  if (!is_connected_) {
    const WgCidrAddr *ipv4_addr = NULL;
    for (const WgCidrAddr &x : processor_.addr()) {
      if (x.size == 32) { ipv4_addr = &x; break; }
    }
    uint32 ipv4_ip = ipv4_addr ? ReadBE32(ipv4_addr->addr) : 0;
    char buf[kSizeOfAddress];
    RINFO("Connection established. IP %s", ipv4_ip ? print_ip(buf, ipv4_ip) : "(none)");
    is_connected_ = true;
  }
}

void TunsafeBackendBsdImpl::OnConnectionRetry(uint32 attempts) {
  if (is_connected_ && attempts >= 3) {
    is_connected_ = false;
    RINFO("Reconnecting...");
  }
}

void TunsafeBackendBsdImpl::OnRequestToken(WgPeer *peer, uint32 type) {
  if (!g_daemon_mode) {
    fprintf(stderr, "A two factor token is required to login. Please enter the value from your authenticator.\nToken: ");
    char buf[100], *rv;
    while (!(rv = fgets(buf, 100, stdin)) && errno == EINTR) {}
    if (rv) {
      size_t len = strlen(buf);
      while (len && buf[len-1] == '\n') buf[--len] = 0;
      plugin_->SubmitToken((const uint8*)buf, strlen(buf));
    }
  }
}

void TunsafeBackendBsdImpl::CloseOrphanTcpConnections() {
  // Add all incoming tcp connections into a lookup table
  WG_HASHTABLE_IMPL<WgAddrEntry::IpPort, void*, WgAddrEntry::IpPortHasher> lookup;
  for (TcpSocketBsd *tcp = network_.tcp_sockets(); tcp; tcp = tcp->next()) {
    if (tcp->endpoint_protocol() == (kPacketProtocolTcp | kPacketProtocolIncomingConnection)) {
      // Avoid deleting tcp sockets that were just born.
      if (tcp->age == 0) {
        tcp->age = 1;
      } else {
        lookup[ConvertIpAddrToAddrX(tcp->endpoint())] = tcp;
      }
    }
  }
  if (lookup.empty())
    return;
  // For each peer, check if it has an endpoint that matches 
  // an entry in the lookup table, and delete it from the lookup
  // table.
  for(WgPeer *peer = processor_.dev().first_peer(); peer; peer = peer->next_peer()) {
    if (peer->endpoint_protocol() == (kPacketProtocolTcp | kPacketProtocolIncomingConnection))
      lookup.erase(ConvertIpAddrToAddrX(peer->endpoint()));
  }
  // The tcp connections that are still in the hashtable can be deleted
  for(const auto &it : lookup)
    delete (TcpSocketBsd *)it.second;
}
int main(int argc, char **argv) {
  CommandLineOutput cmd = {0};

  InitCpuFeatures();

  if (argc == 2 && strcmp(argv[1], "--benchmark") == 0) {
    Benchmark();
    return 0;
  }

  int rv = HandleCommandLine(argc, argv, &cmd);
  if (!cmd.filename_to_load)
    return rv;
  
#if defined(OS_MACOSX)
  InitOsxGetMilliseconds();
#endif

  SetThreadName("tunsafe-m");

  TunsafeBackendBsdImpl backend;
  if (cmd.interface_name)
    backend.SetTunDeviceName(cmd.interface_name);

  DnsResolver dns_resolver(NULL);
  if (*cmd.filename_to_load && !ParseWireGuardConfigFile(backend.processor(), cmd.filename_to_load, &dns_resolver))
    return 1;
  if (!backend.processor()->Start())
    return 1;

  if (cmd.daemon) {
    g_daemon_mode = true;

    fprintf(stderr, "Switching to daemon mode...\n");
    if (daemon(0, 0) == -1)
      perror("daemon() failed");
  }

  backend.RunLoop();
  backend.CleanupRoutes();

  return 0;
}

