// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once


class DnsBlocker {
public:
  DnsBlocker();
  ~DnsBlocker();
  
  bool BlockDnsExceptOnAdapter(const NET_LUID &luid, bool also_ipv6);
  void RestoreDns();
  bool IsActive() { return handle_ != NULL; }

  // Current state
  NET_LUID luid_;
  HANDLE handle_;
  bool also_ipv6_;
};

bool AddKillSwitchFirewall(const NET_LUID &luid_to_allow, bool also_ipv6, bool allow_local_networks);
void RemoveKillSwitchFirewall();
bool GetKillSwitchFirewallActive();
