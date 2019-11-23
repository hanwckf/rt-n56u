// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#ifndef TUNSAFE_NETWORK_BSD_COMMON_H_
#define TUNSAFE_NETWORK_BSD_COMMON_H_

#include "netapi.h"
#include "wireguard.h"
#include "wireguard_config.h"
#include <string>
#include <signal.h>

struct RouteInfo {
  uint8 family;
  uint8 cidr;
  uint8 ip[16];
  uint8 gw[16];
  std::string dev;
};

class SignalCatcher {
public:
  SignalCatcher(bool *exit_flag, bool *sigalarm_flag);
  ~SignalCatcher();

  sigset_t orig_signal_mask_;
private:
  static void SigAlrm(int sig);
  static void SigInt(int sig);
  bool *exit_flag_;
  bool *sigalarm_flag_;
};

class TunsafeBackendBsd : public TunInterface, public UdpInterface {
public:
  TunsafeBackendBsd();
  virtual ~TunsafeBackendBsd();

  void CleanupRoutes();

  void SetTunDeviceName(const char *name);

  // -- from TunInterface
  virtual bool Configure(const TunConfig &&config, TunConfigOut *out) override;

protected:
  virtual bool InitializeTun(char devname[16]) = 0;

  void AddRoute(uint32 ip, uint32 cidr, uint32 gw, const char *dev);
  void DelRoute(const RouteInfo &cd);
  bool AddRoute(int family, const void *dest, int dest_prefix, const void *gateway, const char *dev);
  bool RunPrePostCommand(const std::vector<std::string> &vec);

  std::vector<RouteInfo> cleanup_commands_;
  std::vector<std::string> pre_down_, post_down_;
  std::vector<WgCidrAddr> addresses_to_remove_;
  char devname_[16];
  bool tun_interface_gone_;
};

#endif  // TUNSAFE_NETWORK_BSD_COMMON_H_
