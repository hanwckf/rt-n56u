// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "tunsafe_types.h"
#include "network_win32_dnsblock.h"
#include <fwpmu.h>
#include <fwpmtypes.h>
#include <string.h>

#pragma comment (lib, "Fwpuclnt.lib")

static const GUID TUNSAFE_DNS_SUBLAYER = {0x1ce6cce2, 0xcc8f, 0x4175, { 0xac, 0x7b, 0x95, 0xfd, 0xe8, 0x95, 0x80, 0x92}};
static const GUID TUNSAFE_GLOBAL_BLOCK_SUBLAYER = {0x1ce6cce2, 0xcc8f, 0x4175,{0xac, 0x7b, 0x95, 0xfd, 0xe8, 0x95, 0x80, 0x93}};

static bool GetFwpmAppIdFromCurrentProcess(FWP_BYTE_BLOB **appid) {
  wchar_t module_filename[MAX_PATH];
  DWORD err = GetModuleFileNameW(NULL, module_filename, ARRAYSIZE(module_filename));
  if (err == 0 || err == ARRAYSIZE(module_filename))
    return false;
  err = FwpmGetAppIdFromFileName0(module_filename, appid);
  if (err != 0)
    return false;
  return true;
}

static uint8 internet_fw_blocking_state;

static inline bool FwpmFilterAddCheckedAleConnect(HANDLE handle, FWPM_FILTER0 *filter, bool also_ipv6, int idx) {
  DWORD err;
  UINT64 dummy;

  filter->layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
  err = FwpmFilterAdd0(handle, filter, NULL, &dummy);
  if (err != 0) {
    RERROR("FwpmFilterAdd0 #%d failed (%s): %d", idx, "ipv4", err);
    return false;
  }

  if (also_ipv6) {
    filter->layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V6;
    err = FwpmFilterAdd0(handle, filter, NULL, &dummy);
    if (err != 0) {
      RERROR("FwpmFilterAdd0 #%d failed (%s): %d", idx, "ipv6", err);
      return false;
    }
  }
  return true;
}

DnsBlocker::DnsBlocker() {
  also_ipv6_ = false;
  handle_ = NULL;
}

DnsBlocker::~DnsBlocker() {
  RestoreDns();
}

bool DnsBlocker::BlockDnsExceptOnAdapter(const NET_LUID &luid, bool also_ipv6) {
  FWPM_SUBLAYER0 *sublayer = NULL;
  FWP_BYTE_BLOB *fwp_appid = NULL;
  
  FWPM_FILTER0 filter;
  FWPM_FILTER_CONDITION0 filter_condition[2];
  DWORD err;
  HANDLE handle = NULL;

  // Check if it already matches
  if (handle_ != NULL) {
    if (memcmp(&luid, &luid_, sizeof(luid)) == 0 && also_ipv6_)
      return true;
    FwpmEngineClose0(handle_);
    handle_ = NULL;
  }

  {
    FWPM_SESSION0 session = {0};
    session.flags = FWPM_SESSION_FLAG_DYNAMIC;
    err = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &handle);
    if (err != 0) {
      RERROR("FwpmEngineOpen0 failed: %d", err);
      goto getout;
    }
  }

  {
    FWPM_SUBLAYER0 sublayer = {0};
    sublayer.subLayerKey = TUNSAFE_DNS_SUBLAYER;
    sublayer.displayData.name = L"TunSafe DNS Block";
    sublayer.weight = 0x100;
    err = FwpmSubLayerAdd0(handle, &sublayer, NULL);
    if (err != 0) {
      RERROR("FwpmSubLayerAdd0 failed: %d", err);
      goto getout;
    }
  }

  if (!GetFwpmAppIdFromCurrentProcess(&fwp_appid)) {
    RERROR("GetFwpmAppIdFromCurrentProcess failed");
    goto getout;
  }

  // Allow all queries to port 53 from our process
  memset(&filter, 0, sizeof(filter));
  filter_condition[0].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
  filter_condition[0].matchType = FWP_MATCH_EQUAL;
  filter_condition[0].conditionValue.type = FWP_UINT16;
  filter_condition[0].conditionValue.uint16 = 53;
  filter_condition[1].fieldKey = FWPM_CONDITION_ALE_APP_ID;
  filter_condition[1].matchType = FWP_MATCH_EQUAL;
  filter_condition[1].conditionValue.type = FWP_BYTE_BLOB_TYPE;
  filter_condition[1].conditionValue.byteBlob = fwp_appid;
  filter.filterCondition = filter_condition;
  filter.numFilterConditions = 2;
  filter.subLayerKey = TUNSAFE_DNS_SUBLAYER;
  filter.displayData.name = L"TunSafe DNS Block";
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = 15;
  filter.action.type = FWP_ACTION_PERMIT;
  if (!FwpmFilterAddCheckedAleConnect(handle, &filter, also_ipv6, 1))
    goto getout;

  // Allow DNS queries from TAP
  filter_condition[1].fieldKey = FWPM_CONDITION_IP_LOCAL_INTERFACE;
  filter_condition[1].conditionValue.type = FWP_UINT64;
  filter_condition[1].conditionValue.uint64 = (uint64*)&luid.Value;
  filter.weight.uint8 = 14;
  if (!FwpmFilterAddCheckedAleConnect(handle, &filter, also_ipv6, 2))
    goto getout;

  // Block all IPv4 and IPv6
  filter.numFilterConditions = 1;
  filter.weight.type = FWP_EMPTY;
  filter.action.type = FWP_ACTION_BLOCK;
  if (!FwpmFilterAddCheckedAleConnect(handle, &filter, also_ipv6, 3))
    goto getout;

  goto success;
getout:
  if (handle != NULL) {
    FwpmEngineClose0(handle);
    handle = NULL;
  }
success:
  if (fwp_appid)
    FwpmFreeMemory0((void **)&fwp_appid);

  handle_ = handle;
  also_ipv6_ = also_ipv6;
  luid_ = luid;
  return handle != NULL;
}

void DnsBlocker::RestoreDns() {
  HANDLE h = handle_;
  if (h) {
    handle_ = NULL;
    FwpmEngineClose0(h);
  }
}

static bool RemovePersistentInternetBlockingInner(HANDLE handle, bool destroy_sublayer) {
  FWPM_FILTER_ENUM_TEMPLATE0 enum_template = {0};
  HANDLE enum_handle = NULL;
  DWORD err;
  UINT32 num_returned;
  FWPM_FILTER0 **filter = NULL;

  for (int iptype = 0; iptype < 2; iptype++) {
    enum_template.layerKey = iptype == 0 ? FWPM_LAYER_ALE_AUTH_CONNECT_V4 : FWPM_LAYER_ALE_AUTH_CONNECT_V6;
    enum_template.actionMask = 0xffffffff;

    err = FwpmFilterCreateEnumHandle0(handle, &enum_template, &enum_handle);
    if (err != 0) {
      RERROR("FwpmFilterCreateEnumHandle0 failed: %d", err);
      goto getout;
    }

    do {
      err = FwpmFilterEnum0(handle, enum_handle, 256, &filter, &num_returned);
      if (err != 0) {
        RERROR("FwpmFilterEnum0 failed: %d", err);
        goto getout;
      }
      for (UINT32 i = 0; i < num_returned; i++) {
        FWPM_FILTER0 *cur_filter = filter[i];
        if (memcmp(&cur_filter->subLayerKey, &TUNSAFE_GLOBAL_BLOCK_SUBLAYER, sizeof(GUID)) == 0 && (destroy_sublayer || cur_filter->numFilterConditions != 0)) {
          err = FwpmFilterDeleteById0(handle, cur_filter->filterId);
          if (err != 0)
            RERROR("FwpmFilterDeleteById0 failed: %d", err);
        }
      }
      FwpmFreeMemory0((void**)&filter);
    } while (num_returned == 256);

    FwpmFilterDestroyEnumHandle0(handle, enum_handle);
    enum_handle = NULL;
  }

  if (destroy_sublayer) {
    err = FwpmSubLayerDeleteByKey0(handle, &TUNSAFE_GLOBAL_BLOCK_SUBLAYER);
    if (err != 0 && err != FWP_E_SUBLAYER_NOT_FOUND) {
      RERROR("FwpmSubLayerDeleteByKey0 failed: %d", err);
      goto getout;
    }
  }

getout:
  if (enum_handle != NULL) {
    FwpmFilterDestroyEnumHandle0(handle, enum_handle);
  }
  return false;
}

struct LastKillswitchSettings {
  NET_LUID luid_to_allow;
  bool also_ipv6;
  bool allow_local_networks;
};

static LastKillswitchSettings last_killswitch_settings;


bool AddKillSwitchFirewall(const NET_LUID &luid_to_allow, bool also_ipv6, bool allow_local_networks) {
  FWPM_SUBLAYER0 *sublayer_p = NULL;
  FWP_BYTE_BLOB *fwp_appid = NULL;
  FWPM_FILTER0 filter;
  FWPM_FILTER_CONDITION0 filter_condition[3];
  DWORD err;
  HANDLE handle = NULL;
  bool success = false;

  LastKillswitchSettings new_settings;
  new_settings.luid_to_allow = luid_to_allow;
  new_settings.also_ipv6 = also_ipv6;
  new_settings.allow_local_networks = allow_local_networks;
  
  {
    FWPM_SESSION0 session = {0};
    err = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &handle);
    if (err != 0) {
      RERROR("FwpmEngineOpen0 failed: %d", err);
      goto getout;
    }
  }

  if (FwpmSubLayerGetByKey0(handle, &TUNSAFE_GLOBAL_BLOCK_SUBLAYER, &sublayer_p) == 0) {
    // The sublayer already exists
    FwpmFreeMemory0((void **)&sublayer_p);

    if (memcmp(&last_killswitch_settings, &new_settings, sizeof(new_settings)) != 0)
      RemovePersistentInternetBlockingInner(handle, false);
  } else {
    // Add new sublayer
    FWPM_SUBLAYER0 sublayer = {0};
    sublayer.subLayerKey = TUNSAFE_GLOBAL_BLOCK_SUBLAYER;
    sublayer.displayData.name = L"TunSafe Global Block";
    sublayer.weight = 0x101;
    err = FwpmSubLayerAdd0(handle, &sublayer, NULL);
    if (err != 0) {
      RERROR("FwpmSubLayerAdd0 failed: %d", err);
      goto getout;
    }
  }

  if (!GetFwpmAppIdFromCurrentProcess(&fwp_appid)) {
    RERROR("GetFwpmAppIdFromCurrentProcess failed");
    goto getout;
  }

  // Allow all outgoing queries from our process
  memset(&filter, 0, sizeof(filter));
  filter_condition[0].fieldKey = FWPM_CONDITION_ALE_APP_ID;
  filter_condition[0].matchType = FWP_MATCH_EQUAL;
  filter_condition[0].conditionValue.type = FWP_BYTE_BLOB_TYPE;
  filter_condition[0].conditionValue.byteBlob = fwp_appid;
  filter.numFilterConditions = 1;
  filter.filterCondition = filter_condition;
  filter.subLayerKey = TUNSAFE_GLOBAL_BLOCK_SUBLAYER;
  filter.displayData.name = L"TunSafe Global Block";
  filter.weight.type = FWP_UINT8;
  filter.weight.uint8 = 15;
  filter.action.type = FWP_ACTION_PERMIT;
  if (!FwpmFilterAddCheckedAleConnect(handle, &filter, also_ipv6, 1))
    goto getout;

  // Permit all queries going out on TUN
  filter_condition[0].fieldKey = FWPM_CONDITION_IP_LOCAL_INTERFACE;
  filter_condition[0].conditionValue.type = FWP_UINT64;
  filter_condition[0].conditionValue.uint64 = (uint64*)&luid_to_allow.Value;
  filter_condition[0].matchType = FWP_MATCH_EQUAL;
  filter.weight.uint8 = 14;
  if (!FwpmFilterAddCheckedAleConnect(handle, &filter, also_ipv6, 2))
    goto getout;

  // Permit everything that's loopback
  filter_condition[0].fieldKey = FWPM_CONDITION_INTERFACE_TYPE;
  filter_condition[0].conditionValue.type = FWP_UINT32;
  filter_condition[0].conditionValue.uint32 = 24;
  filter_condition[0].matchType = FWP_MATCH_EQUAL;
  filter.weight.uint8 = 13;
  if (!FwpmFilterAddCheckedAleConnect(handle, &filter, also_ipv6, 3))
    goto getout;

  // Permit everything going out to local networks
  //  '10.0.0.0/8', '172.16.0.0/12', '192.168.0.0/16'
  if (allow_local_networks) {
    static const FWP_V4_ADDR_AND_MASK kLocalNetworkAddrMask[3] = {
      {0x0A000000, 0xff000000}, // 10.0.0.0/8
      {0xAC100000, 0xfff00000}, // 172.16.0.0/12
      {0xC0A80000, 0xffff0000}, // 192.168.0.0/16
    };
    for (int i = 0; i < 3; i++) {
      FWP_V4_ADDR_AND_MASK addr_mask = kLocalNetworkAddrMask[i];
      filter.numFilterConditions = 1;
      filter_condition[0].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
      filter_condition[0].conditionValue.type = FWP_V4_ADDR_MASK;
      filter_condition[0].conditionValue.v4AddrMask = &addr_mask;
      filter_condition[0].matchType = FWP_MATCH_EQUAL;
      filter.weight.uint8 = 12;
      if (!FwpmFilterAddCheckedAleConnect(handle, &filter, false, 4))
        goto getout;
    }
  }

  // Permit all queries on the DHCP port (It uses 68 on the local side and 67 on the remote side)
  filter_condition[2].fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
  filter_condition[2].matchType = FWP_MATCH_EQUAL;
  filter_condition[2].conditionValue.type = FWP_UINT16;
  filter_condition[2].conditionValue.uint16 = 68;
  filter_condition[1].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
  filter_condition[1].matchType = FWP_MATCH_EQUAL;
  filter_condition[1].conditionValue.type = FWP_UINT16;
  filter_condition[1].conditionValue.uint16 = 67;
  filter.numFilterConditions = 3;
  filter_condition[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
  filter_condition[0].conditionValue.type = FWP_UINT8;
  filter_condition[0].conditionValue.uint8 = 17; // UDP
  filter_condition[0].matchType = FWP_MATCH_EQUAL;
  filter.weight.uint8 = 12;
  if (!FwpmFilterAddCheckedAleConnect(handle, &filter, also_ipv6, 5))
    goto getout;

  // Block the rest
  filter.numFilterConditions = 0;
  filter.weight.type = FWP_EMPTY;
  filter.action.type = FWP_ACTION_BLOCK;
  if (!FwpmFilterAddCheckedAleConnect(handle, &filter, also_ipv6, 6))
    goto getout;

  success = true;
  last_killswitch_settings = new_settings;

getout:
  if (!success)
    memset(&last_killswitch_settings, 0, sizeof(last_killswitch_settings));

  if (handle != NULL) {
    if (!success)
      RemovePersistentInternetBlockingInner(handle, true);
    FwpmEngineClose0(handle);
    handle = NULL;
  }
  if (fwp_appid)
    FwpmFreeMemory0((void **)&fwp_appid);
  return success;
}

void RemoveKillSwitchFirewall() {
  DWORD err;
  HANDLE handle = NULL;
  FWPM_SUBLAYER0 *sublayer_p = NULL;

  {
    FWPM_SESSION0 session = {0};
    err = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &handle);
    if (err != 0) {
      RERROR("FwpmEngineOpen0 failed: %d", err);
      goto getout;
    }
  }

  if (FwpmSubLayerGetByKey0(handle, &TUNSAFE_GLOBAL_BLOCK_SUBLAYER, &sublayer_p) == 0) {
    // The sublayer exists
    FwpmFreeMemory0((void **)&sublayer_p);
  } else {
    goto getout;
  }
  
  RemovePersistentInternetBlockingInner(handle, true);

getout:
  if (handle != NULL) {
    FwpmEngineClose0(handle);
    handle = NULL;
  }
}

bool GetKillSwitchFirewallActive() {
  DWORD err;
  HANDLE handle = NULL;
  FWPM_SUBLAYER0 *sublayer_p = NULL;
  bool result;

  {
    FWPM_SESSION0 session = {0};
    err = FwpmEngineOpen0(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &handle);
    if (err != 0) {
      RERROR("FwpmEngineOpen0 failed: %d", err);
      goto getout;
    }
  }

  if (FwpmSubLayerGetByKey0(handle, &TUNSAFE_GLOBAL_BLOCK_SUBLAYER, &sublayer_p) == 0) {
    // The sublayer already exists
    FwpmFreeMemory0((void **)&sublayer_p);
    result = true;
  } else {
    result = false;
  }

getout:
  if (handle != NULL) {
    FwpmEngineClose0(handle);
    handle = NULL;
  }
  return result;
}
