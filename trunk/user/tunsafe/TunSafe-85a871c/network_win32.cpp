// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "network_win32.h"
#include "wireguard_config.h"
#include "netapi.h"
#include <Iphlpapi.h>
#include <Mswsock.h>
#include <ws2ipdef.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include <vector>
#include <Iphlpapi.h>
#include <assert.h>
#include <exdisp.h>
#include "tunsafe_endian.h"
#include "wireguard.h"
#include "util.h"
#include <algorithm>
#include "network_win32_dnsblock.h"
#include "util_win32.h"
#include "tunsafe_wg_plugin.h"

enum {
  HARD_MAXIMUM_QUEUE_SIZE = 102400,
  MAX_BYTES_IN_UDP_OUT_QUEUE = 256 * 1024,
  MAX_BYTES_IN_UDP_OUT_QUEUE_SMALL = (256 + 64) * 1024,

  // On Windows 7 with NDIS6 sometimes the tun queue blows up.
  HARD_MAXIMUM_TUN_QUEUE_SIZE = 16384,
};

enum {
  kMetricNone = -1,
  kMetricAutomatic = 0,
};

static uint8 internet_route_blocking_state;
static SLIST_HEADER freelist_head;
static HKEY g_hklm_reg_key;
static uint8 g_killswitch_curr, g_killswitch_want, g_killswitch_currconn;

bool g_allow_pre_post;
static volatile bool g_fail_malloc_flag;

static void DeactivateKillSwitch(uint32 want);

Packet *AllocPacket() {
  Packet *packet = (Packet*)InterlockedPopEntrySList(&freelist_head);
  if (packet == NULL) {
    while ((packet = (Packet *)_aligned_malloc(kPacketAllocSize, 16)) == NULL) {
      if (g_fail_malloc_flag)
        return NULL;
      Sleep(1000);
    }
  }
  packet->Reset();
  return packet;
}

void FreePacket(Packet *packet) {
  InterlockedPushEntrySList(&freelist_head, &packet->list_entry);
}

static bool IsIpv6AddressSet(const void *p) {
  return (ReadLE64(p) | ReadLE64((char*)p + 8)) != 0;
}

extern "C"
PSLIST_ENTRY __fastcall InterlockedPushListSList(
  IN PSLIST_HEADER ListHead,
  IN PSLIST_ENTRY List,
  IN PSLIST_ENTRY ListEnd,
  IN ULONG Count
);

void FreePackets(Packet *packet, Packet **end, int count) {
  InterlockedPushListSList(&freelist_head, &packet->list_entry, (PSLIST_ENTRY)end, count);
}

void FreeAllPackets() {
  Packet *p;
  p = (Packet*)InterlockedFlushSList(&freelist_head);
  while (Packet *r = p) {
    p = Packet_NEXT(p);
    _aligned_free(r);
  }
}


void InitPacketMutexes() {
  static bool mutex_inited;
  if (!mutex_inited) {
    mutex_inited = true;
    InitializeSListHead(&freelist_head);
  }
}

#define kConcurrentReadTap 16
#define kConcurrentWriteTap 16

#define kAdapterKeyName "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"
#define kNetworkConnectionsKeyName "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}"
#define kTapComponentId "tap0901"

#define TAP_CONTROL_CODE(request,method) \
  CTL_CODE (FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)

#define TAP_IOCTL_GET_MAC               TAP_CONTROL_CODE(1, METHOD_BUFFERED)
#define TAP_IOCTL_GET_VERSION           TAP_CONTROL_CODE(2, METHOD_BUFFERED)
#define TAP_IOCTL_GET_MTU               TAP_CONTROL_CODE(3, METHOD_BUFFERED)
#define TAP_IOCTL_GET_INFO              TAP_CONTROL_CODE(4, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_POINT_TO_POINT TAP_CONTROL_CODE(5, METHOD_BUFFERED)
#define TAP_IOCTL_SET_MEDIA_STATUS      TAP_CONTROL_CODE(6, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_DHCP_MASQ      TAP_CONTROL_CODE(7, METHOD_BUFFERED)
#define TAP_IOCTL_GET_LOG_LINE          TAP_CONTROL_CODE(8, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_DHCP_SET_OPT   TAP_CONTROL_CODE(9, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_TUN            TAP_CONTROL_CODE(10, METHOD_BUFFERED)

static bool RunNetsh(const char *cmdline) {
  wchar_t path[MAX_PATH + 20];
  size_t size = GetSystemDirectoryW(path, MAX_PATH);
  bool result = false;
  if (!size) {
    RERROR("GetSystemDirectory failed");
    return false;
  }
  memcpy(path + size, L"\\netsh.exe", 11 * sizeof(path[0]));

  size_t cmdline_size = strlen(cmdline);
  wchar_t *cmdlinew = new wchar_t[cmdline_size + 1];
  for (size_t i = 0; i <= cmdline_size; i++)
    cmdlinew[i] = cmdline[i];

  STARTUPINFOW si = {0};
  PROCESS_INFORMATION pi = {0};

  GetStartupInfoW(&si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;
   if (CreateProcessW(path, cmdlinew, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
    DWORD exit_code = -1;
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &exit_code);
    if (exit_code != 0)
      RERROR("Netsh failed (%d) : %s", exit_code, cmdline);
    else {
      RINFO("Run: %s", cmdline);
      result = true;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  } else {
    RERROR("CreateProcess failed: %s", cmdline);
  }
  delete[]cmdlinew;
  return result;
}

// Open the TAP adapter, either a random one or a specific one
// On return, the adapter is locked in |TunAdaptersInUse|.
static HANDLE OpenTunAdapter(char guid[ADAPTER_GUID_SIZE], TunsafeRunner *runner, DWORD open_flags) {
  char path[128];
  HANDLE h;
  int retries = 0;
  std::vector<GuidAndDevName> adapters;
  
  // When guid is empty, we try all adapters, otherwise
  // just try the specific adapter
  if (guid[0] == 0) {
    GetTapAdapterInfo(&adapters);
    if (adapters.empty()) {
      RERROR("Unable to find any TAP adapters");
      RERROR("  Please ensure that TunSafe-TAP is properly installed.");
      return NULL;
    }
  } else {
    adapters.emplace_back();
    memcpy(adapters.back().guid, guid, ADAPTER_GUID_SIZE);
    adapters.back().name[0] = 0;
  }
  TunAdaptersInUse *tun_adapters_in_use = TunAdaptersInUse::GetInstance();

RETRY:
  bool did_try_adapter = false;
  int error_code = 0;
  for (GuidAndDevName &x : adapters) {
    snprintf(path, sizeof(path), "\\\\.\\Global\\%s.tap", x.guid);
    if (tun_adapters_in_use->Acquire(x.guid, static_cast<TunsafeBackend*>(runner->backend()))) {
      h = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | open_flags, 0);
      if (h != INVALID_HANDLE_VALUE) {
        memcpy(guid, x.guid, ADAPTER_GUID_SIZE);
        return h;
      }
      did_try_adapter = true;
      error_code = GetLastError();
      tun_adapters_in_use->Release(static_cast<TunsafeBackend*>(runner->backend()));
    }
  }
  if (!did_try_adapter) {
    RERROR("All TAP adapters are currently in use");
    return NULL;
  }
  
  // Sometimes if you close the device right before, it will fail to open with errorcode 31.
  // When resuming from sleep in my VM, the error code is ERROR_FILE_NOT_FOUND
  if ((error_code == ERROR_FILE_NOT_FOUND || error_code == ERROR_GEN_FAILURE) && !runner->exit_code()) {
    if (retries <= 10) {
      RERROR("OpenTapAdapter: CreateFile failed: 0x%X... retrying%s", error_code, retries == 10 ? " (last notice)" : "");
      if (retries == 10) {
        if (error_code == ERROR_FILE_NOT_FOUND) {
          RERROR("  Please ensure that TunSafe-TAP is properly installed.");
        } else if (error_code == ERROR_GEN_FAILURE) {
          RERROR("  Please ensure that the TAP device is not in use.");
        }
        runner->backend()->SetStatus(TunsafeBackend::kStatusTunRetrying);
      }
    }
    int sleep_amount = 250 * std::min(++retries, 40);
    for (;;) {
      if (runner->exit_code())
        return NULL;
      if (sleep_amount == 0)
        break;
      Sleep(125);
      sleep_amount -= 125;
    }
    goto RETRY;
  }

  RERROR("OpenTapAdapter: CreateFile failed: 0x%X", error_code);
  return NULL;
}

static bool AddRoute(int family,
                     const void *dest, int dest_prefix,
                     const void *gateway, const NET_LUID *interface_luid,
                     std::vector<MIB_IPFORWARD_ROW2> *undo_array) {
  MIB_IPFORWARD_ROW2 row = {0};
  char buf1[kSizeOfAddress], buf2[kSizeOfAddress];

  row.InterfaceLuid = *interface_luid;
  row.DestinationPrefix.PrefixLength = dest_prefix;
  row.DestinationPrefix.Prefix.si_family = family;
  row.NextHop.si_family = family;
  if (family == AF_INET) {
    memcpy(&row.DestinationPrefix.Prefix.Ipv4.sin_addr, dest, 4);
    memcpy(&row.NextHop.Ipv4.sin_addr, gateway, 4);
  } else if (family == AF_INET6) {
    memcpy(&row.DestinationPrefix.Prefix.Ipv6.sin6_addr, dest, 16);
    memcpy(&row.NextHop.Ipv6.sin6_addr, gateway, 16);
  } else {
    return false;
  }
  row.ValidLifetime = 0xffffffff;
  row.PreferredLifetime = 0xffffffff;
  row.Metric = 100;
  row.Protocol = MIB_IPPROTO_NETMGMT;

  DWORD error = CreateIpForwardEntry2(&row);
  if (error == NO_ERROR || error == ERROR_OBJECT_ALREADY_EXISTS) {

    if (undo_array)
      undo_array->push_back(row);

    RINFO("Added Route %s  =>  %s%s", print_ip_prefix(buf1, family, dest, dest_prefix),
          print_ip_prefix(buf2, family, gateway, -1), (error == ERROR_OBJECT_ALREADY_EXISTS) ? " (already exists)" : "");
    return true;
  }
  RINFO("AddRoute failed (%d) %s  =>  %s", error, print_ip_prefix(buf1, family, dest, dest_prefix),
        print_ip_prefix(buf2, family, gateway, -1));
  return false;
}

static bool DeleteRoute(MIB_IPFORWARD_ROW2 *row) {
  char buf1[kSizeOfAddress], buf2[kSizeOfAddress];
  DWORD error = DeleteIpForwardEntry2(row);
  
  print_ip_prefix(buf1, row->DestinationPrefix.Prefix.si_family,
    (row->DestinationPrefix.Prefix.si_family == AF_INET) ? (uint8*) &row->DestinationPrefix.Prefix.Ipv4.sin_addr : (uint8*) &row->DestinationPrefix.Prefix.Ipv6.sin6_addr, row->DestinationPrefix.PrefixLength);

  print_ip_prefix(buf2, row->NextHop.si_family,
    (row->NextHop.si_family == AF_INET) ? (uint8*)&row->NextHop.Ipv4.sin_addr : (uint8*)&row->NextHop.Ipv6.sin6_addr, -1);

  if (error == NO_ERROR) {
    RINFO("Deleted Route %s  =>  %s", buf1, buf2);
    return true;
  }
  RINFO("DeleteRoute failed (%d) %s  =>  %s", error, buf1, buf2);
  return false;
}


static uint32 CidrToNetmaskV4(int cidr) {
  return cidr == 32 ? 0xffffffff : 0xffffffff << (32 - cidr);
}

struct RouteInfo {
  uint8 default_gw[16];
  NET_LUID default_adapter;
  bool found_default_adapter;
  uint8 found_null_routes;
};

static bool IsRouteOriginatingFromNullRoute(MIB_IPFORWARD_ROW2 *row) {
  if (!(row->InterfaceLuid.Info.IfType == 24 && row->Protocol == MIB_IPPROTO_NETMGMT && row->DestinationPrefix.PrefixLength == 1))
    return false;
  if (row->NextHop.si_family == AF_INET) {
    return (row->NextHop.Ipv4.sin_addr.S_un.S_addr == 0);
  } else if (row->NextHop.si_family == AF_INET6) {
    static const uint32 nulladdr[4];
    return memcmp(&row->NextHop.Ipv6.sin6_addr, nulladdr, 16) == 0;
  }
  return false;
}

static bool IsDestinationRouteEqualTo(MIB_IPFORWARD_ROW2 *row, const WgCidrAddr *addr) {
  if (addr->size == 32) {
    return row->DestinationPrefix.Prefix.si_family == AF_INET &&
            row->DestinationPrefix.PrefixLength == addr->cidr &&
            memcmp(&row->DestinationPrefix.Prefix.Ipv4.sin_addr, addr->addr, 4) == 0;

  } else if (addr->size == 128) {
    return row->DestinationPrefix.Prefix.si_family == AF_INET6 &&
           row->DestinationPrefix.PrefixLength == addr->cidr &&
           memcmp(&row->DestinationPrefix.Prefix.Ipv6.sin6_addr, addr->addr, 16) == 0;
  } else {
    return false;
  }
}

static bool IsDestinationRouteEqualToAny(MIB_IPFORWARD_ROW2 *row, const std::vector<WgCidrAddr> &addr) {
  for (const WgCidrAddr &x : addr)
    if (IsDestinationRouteEqualTo(row, &x))
      return true;
  return false;
}

static void DeleteRouteOrPrintErr(MIB_IPFORWARD_ROW2 *row) {
  char buf1[kSizeOfAddress];
  UINT32 r = DeleteIpForwardEntry2(row);
  if (r)
    RERROR("Unable to delete old route (%d): %s", r,
           print_ip_prefix(buf1, row->DestinationPrefix.Prefix.si_family, row->DestinationPrefix.Prefix.si_family == AF_INET ?
              (void*)&row->DestinationPrefix.Prefix.Ipv4.sin_addr :
              (void*)&row->DestinationPrefix.Prefix.Ipv6.sin6_addr, row->DestinationPrefix.PrefixLength));
}

static bool GetDefaultRouteAndDeleteOldRoutes(int family, const NET_LUID *InterfaceLuid, bool keep_null_routes, const std::vector<WgCidrAddr> *old_endpoint_to_delete, RouteInfo *ri) {
  MIB_IPFORWARD_TABLE2 *table = NULL;

  assert(family == AF_INET || family == AF_INET6);

  ri->found_default_adapter = false;
  ri->found_null_routes = 0;

  if (GetIpForwardTable2(family, &table))
    return false;
  DWORD rv = 0;
  DWORD gw_metric = 0xffffffff;
  for (unsigned i = 0; i < table->NumEntries; i++) {
    MIB_IPFORWARD_ROW2 *row = &table->Table[i];
    if (InterfaceLuid && memcmp(&row->InterfaceLuid, InterfaceLuid, sizeof(NET_LUID)) == 0) {
      if (row->Protocol == MIB_IPPROTO_NETMGMT && !row->AutoconfigureAddress)
        DeleteRouteOrPrintErr(row);
    } else if (IsRouteOriginatingFromNullRoute(row)) {
      ri->found_null_routes++;
      if (!keep_null_routes)
        DeleteRouteOrPrintErr(row);
    } else if (row->DestinationPrefix.PrefixLength == 0 && row->Metric < gw_metric) {
      gw_metric = row->Metric;
      if (family == AF_INET) {
        memcpy(&ri->default_gw, &row->NextHop.Ipv4.sin_addr, 4);
      } else {
        memcpy(&ri->default_gw, &row->NextHop.Ipv6.sin6_addr, 16);
      }
      ri->default_adapter = row->InterfaceLuid;
      ri->found_default_adapter = true;
    }
  }

  if (old_endpoint_to_delete && ri->found_default_adapter) {
    for (unsigned i = 0; i < table->NumEntries; i++) {
      MIB_IPFORWARD_ROW2 *row = &table->Table[i];
      if (row->Protocol == MIB_IPPROTO_NETMGMT && memcmp(&row->InterfaceLuid, &ri->default_adapter, sizeof(NET_LUID)) == 0) {
        if (IsDestinationRouteEqualToAny(row, *old_endpoint_to_delete))
          DeleteRouteOrPrintErr(row);
      }
    }
  }

  FreeMibTable(table);
  return (rv == 0);
}

void FreePacketList(Packet *pp) {
  while (Packet *p = pp) {
    pp = Packet_NEXT(p);
    FreePacket(p);
  }
}

UdpSocketWin32::UdpSocketWin32(NetworkWin32 *network_win32) {
  network_ = network_win32;
  wqueue_end_ = &wqueue_;
  wqueue_ = NULL;
  socket_ = INVALID_SOCKET;
  socket_ipv6_ = INVALID_SOCKET;
  
  finished_reads_ = NULL;
  finished_reads_end_ = &finished_reads_;
  finished_reads_count_ = 0;

  max_read_ipv6_ = 0;
  num_reads_[0] = num_reads_[1] = 0;
  num_writes_ = 0;
  pending_writes_ = NULL;

  qsize1_ = 0;
  qsize2_ = 0;
}

UdpSocketWin32::~UdpSocketWin32() {
  closesocket(socket_);
  closesocket(socket_ipv6_);
  FreePacketList(wqueue_);
}

bool UdpSocketWin32::Configure(int listen_on_port) {
  // If attempting to initialize when the thread is already started, then stop
  // the thread, reinitialize, and start the thread.
  if (network_->thread_ != NULL) {
    network_->StopThread();
    bool retcode = Configure(listen_on_port);
    network_->StartThread();
    return retcode;
  }

  bool retval = false;
  SOCKET socket_ipv4 = INVALID_SOCKET, socket_ipv6 = INVALID_SOCKET;

  socket_ipv4 = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
  if (socket_ipv4 == INVALID_SOCKET) {
    RERROR("UdpSocketWin32::Initialize WSASocket failed");
    goto fail;
  }
  if (!CreateIoCompletionPort((HANDLE)socket_ipv4, network_->completion_port_handle_, 0, 0)) {
    RERROR("UdpSocketWin32::Initialize CreateIoCompletionPort failed");
    goto fail;
  }

  {
    sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(listen_on_port);
    if (bind(socket_ipv4, (struct sockaddr*)&sin, sizeof(sin)) != 0) {
      RERROR("UdpSocketWin32::Initialize bind failed");
      goto fail;
    }
  }
  // Also open up a socket for ipv6
  socket_ipv6 = WSASocket(AF_INET6, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
  if (socket_ipv6 != INVALID_SOCKET) {
    if (!CreateIoCompletionPort((HANDLE)socket_ipv6, network_->completion_port_handle_, 0, 0)) {
      RERROR("IPv6 Socket completion port failed.");
      closesocket(socket_ipv6);
      socket_ipv6 = INVALID_SOCKET;
    } else {
      sockaddr_in6 sin6 = {0};
      sin6.sin6_family = AF_INET6;
      sin6.sin6_port = htons(listen_on_port);
      if (bind(socket_ipv6, (struct sockaddr*)&sin6, sizeof(sin6)) != 0) {
        RERROR("UdpSocketWin32::Initialize bind failed IPv6");
      }
    }
  } else {
    RERROR("IPv6 Socket creation failed.");
  }
  std::swap(socket_ipv6_, socket_ipv6);
  std::swap(socket_, socket_ipv4);
  retval = true;

  max_read_ipv6_ = socket_ipv6 != INVALID_SOCKET ? 1 : 0;

fail:
  if (socket_ipv4 != INVALID_SOCKET)
    closesocket(socket_ipv4);
  if (socket_ipv6 != INVALID_SOCKET)
    closesocket(socket_ipv6);
  return retval;
}

// Called on another thread to queue up a udp packet
void UdpSocketWin32::WriteUdpPacket(Packet *packet) {
  if (qsize2_ - qsize1_ >= (unsigned)(packet->size < 576 ? MAX_BYTES_IN_UDP_OUT_QUEUE_SMALL : MAX_BYTES_IN_UDP_OUT_QUEUE)) {
    FreePacket(packet);
    return;
  }
  packet->queue_next = NULL;
  qsize2_ += packet->size;

  mutex_.Acquire();
  Packet *was_empty = wqueue_;
  *wqueue_end_ = packet;
  wqueue_end_ = &Packet_NEXT(packet);
  mutex_.Release();

  if (was_empty == NULL)
    network_->WakeUp();
}

enum {
  kUdpGetQueuedCompletionStatusSize = kConcurrentWriteTap + kConcurrentReadTap + 1
};

#ifndef STATUS_PORT_UNREACHABLE
#define STATUS_PORT_UNREACHABLE 0xC000023F
#endif

static inline bool IsIgnoredUdpError(DWORD err) {
  return err == WSAEMSGSIZE || err == WSAECONNRESET || err == WSAENETRESET || err == STATUS_PORT_UNREACHABLE;
}

void UdpSocketWin32::DoMoreReads() {
  // Listen with multiple ipv6 packets only if we ever sent an ipv6 packet.
  for (int i = num_reads_[IPV6]; i < max_read_ipv6_; i++) {
    Packet *p = network_->packet_pool().AllocPacketFromPool();
    if (!p)
      break;
restart_read_udp6:
    ClearOverlapped(&p->overlapped);
    WSABUF wsabuf = {(ULONG)kPacketCapacity, (char*)p->data};
    DWORD flags = 0;
    p->userdata = IPV6;
    p->sin_size = sizeof(p->addr.sin6);
    p->queue_cb = this;
    if (WSARecvFrom(socket_ipv6_, &wsabuf, 1, NULL, &flags, (struct sockaddr*)&p->addr, &p->sin_size, &p->overlapped, NULL) != 0) {
      DWORD err = WSAGetLastError();
      if (err != WSA_IO_PENDING) {
        if (err == WSAEMSGSIZE || err == WSAECONNRESET || err == WSAENETRESET)
          goto restart_read_udp6;
        RERROR("UdpSocketWin32:WSARecvFrom failed 0x%X", err);
        FreePacket(p);
        break;
      }
    }
    num_reads_[IPV6]++;
  }
  // Initiate more reads, reusing the Packet structures in |finished_writes|.

  if (socket_ != INVALID_SOCKET) {
    for (int i = num_reads_[IPV4]; i < kConcurrentReadUdp; i++) {
      Packet *p = network_->packet_pool().AllocPacketFromPool();
      if (!p)
        break;
restart_read_udp:
      ClearOverlapped(&p->overlapped);
      WSABUF wsabuf = {(ULONG)kPacketCapacity, (char*)p->data};
      DWORD flags = 0;
      p->userdata = IPV4;
      p->sin_size = sizeof(p->addr.sin);
      p->queue_cb = this;
      if (WSARecvFrom(socket_, &wsabuf, 1, NULL, &flags, (struct sockaddr*)&p->addr, &p->sin_size, &p->overlapped, NULL) != 0) {
        DWORD err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
          if (err == WSAEMSGSIZE || err == WSAECONNRESET || err == WSAENETRESET)
            goto restart_read_udp;
          RERROR("UdpSocketWin32:WSARecvFrom failed 0x%X", err);
          FreePacket(p);
          break;
        }
      }
      num_reads_[IPV4]++;
    }
  }
}

void UdpSocketWin32::ProcessPackets() {
  // Push all the finished reads to the packet handler
  if (finished_reads_ != NULL) {
    packet_handler_->PostPackets(finished_reads_, finished_reads_end_, finished_reads_count_);
    finished_reads_ = NULL;
    finished_reads_end_ = &finished_reads_;
    finished_reads_count_ = 0;
  }
}

void UdpSocketWin32::DoMoreWrites() {
  Packet *pending_writes = pending_writes_;
  // Initiate more writes from |wqueue_|
  while (num_writes_ < kConcurrentWriteUdp) {
    // Refill from queue if empty, avoid taking the mutex if it looks empty
    if (!pending_writes) {
      if (!wqueue_)
        break;
      mutex_.Acquire();
      pending_writes = wqueue_;
      wqueue_end_ = &wqueue_;
      wqueue_ = NULL;
      mutex_.Release();
      if (!pending_writes)
        break;
    }
    qsize1_ += pending_writes->size;

    // Then issue writes
    Packet *p = pending_writes;
    pending_writes = Packet_NEXT(p);
    ClearOverlapped(&p->overlapped);
    p->userdata = 2;
    p->queue_cb = this;
    WSABUF wsabuf = {(ULONG)p->size, (char*)p->data};
    int rv;
    if (p->addr.sin.sin_family == AF_INET) {
      rv = WSASendTo(socket_, &wsabuf, 1, NULL, 0, (struct sockaddr*)&p->addr.sin, sizeof(p->addr.sin), &p->overlapped, NULL);
    } else {
      if (socket_ipv6_ == INVALID_SOCKET) {
        RERROR("UdpSocketWin32: unavailable ipv6 socket");
        FreePacket(p);
        continue;
      }
      max_read_ipv6_ = kConcurrentReadUdp;
      rv = WSASendTo(socket_ipv6_, &wsabuf, 1, NULL, 0, (struct sockaddr*)&p->addr.sin6, sizeof(p->addr.sin6), &p->overlapped, NULL);
    }
    if (rv != 0) {
      DWORD err = WSAGetLastError();
      if (err != ERROR_IO_PENDING) {
        RERROR("UdpSocketWin32: WSASendTo failed 0x%X", err);
        FreePacket(p);
        continue;
      }
    }
    num_writes_++;
  }
  pending_writes_ = pending_writes;
}

void UdpSocketWin32::CancelAllIO() {
  CancelIo((HANDLE)socket_);
  CancelIo((HANDLE)socket_ipv6_);
  FreePacketList(pending_writes_);
}

bool UdpSocketWin32::HasOutstandingIO() {
  return (num_reads_[IPV4] + num_reads_[IPV6] + num_writes_) != 0;
}

void UdpSocketWin32::OnQueuedItemEvent(QueuedItem *qi, uintptr_t extra) {
  Packet *p = static_cast<Packet*>(qi);
  if (p->userdata < 2) {
    num_reads_[p->userdata]--;
    if ((DWORD)p->overlapped.Internal != 0) {
      if (!IsIgnoredUdpError((DWORD)p->overlapped.Internal))
        RERROR("UdpSocketWin32::Read error 0x%X", (DWORD)p->overlapped.Internal);
      network_->packet_pool().FreePacketToPool(p);
    } else {
      // Remember all the finished packets and queue them up to the next thread once we've
      // collected them all.
      p->size = (int)p->overlapped.InternalHigh;
      p->protocol = kPacketProtocolUdp;
      p->queue_cb = packet_handler_->udp_queue();
      p->queue_next = NULL;
      *finished_reads_end_ = p;
      finished_reads_end_ = &Packet_NEXT(p);
      finished_reads_count_++;
    }
  } else {
    num_writes_--;
    if ((DWORD)p->overlapped.Internal != 0)
      RERROR("UdpSocketWin32::Write error 0x%X", (DWORD)p->overlapped.Internal);
    network_->packet_pool().FreePacketToPool(p);
  }
}

void UdpSocketWin32::OnQueuedItemDelete(QueuedItem *qi) {
  Packet *p = static_cast<Packet*>(qi);
  if (p->userdata < 2) {
    num_reads_[p->userdata]--;
  } else {
    num_writes_--;
  }
  network_->packet_pool().FreePacketToPool(p);
}

void UdpSocketWin32::DoIO() {
  DoMoreWrites();
  ProcessPackets();
  DoMoreReads();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
NetworkWin32::NetworkWin32() : udp_socket_(this) {
  exit_thread_ = false;
  thread_ = NULL;
  completion_port_handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
  tcp_socket_ = NULL;
}

NetworkWin32::~NetworkWin32() {
  assert(thread_ == NULL);
  
  for (TcpSocketWin32 *socket = tcp_socket_; socket; )
    delete exch(socket, socket->next_);

  CloseHandle(completion_port_handle_);
}

DWORD WINAPI NetworkWin32::NetworkThread(void *x) {
  NetworkWin32 *net = (NetworkWin32 *)x;
  net->ThreadMain();
  return 0;
}

void NetworkWin32::ThreadMain() {
  OVERLAPPED_ENTRY entries[kUdpGetQueuedCompletionStatusSize];

  while (!exit_thread_) {
    // TODO: In the future, don't process every socket here, only
    // those sockets that requested it.
    udp_socket_.DoIO();
    for (TcpSocketWin32 *tcp = tcp_socket_; tcp;)
      exch(tcp, tcp->next_)->DoIO();

    packet_pool_.FreeSomePackets();

    ULONG num_entries = 0;
    if (!GetQueuedCompletionStatusEx(completion_port_handle_, entries, kUdpGetQueuedCompletionStatusSize, &num_entries, INFINITE, FALSE)) {
      RINFO("GetQueuedCompletionStatusEx failed.");
      break;
    }
    for (ULONG i = 0; i < num_entries; i++) {
      if (entries[i].lpOverlapped) {
        QueuedItem *w = (QueuedItem*)((byte*)entries[i].lpOverlapped - offsetof(QueuedItem, overlapped));
        w->queue_cb->OnQueuedItemEvent(w, 0);
      }
    }
  }

  udp_socket_.CancelAllIO();
  for (TcpSocketWin32 *tcp = tcp_socket_; tcp; tcp = tcp->next_)
    tcp->CancelAllIO();

  while (HasOutstandingIO()) {
    ULONG num_entries = 0;
    if (!GetQueuedCompletionStatusEx(completion_port_handle_, entries, kUdpGetQueuedCompletionStatusSize, &num_entries, INFINITE, FALSE)) {
      RINFO("GetQueuedCompletionStatusEx failed.");
      break;
    }
    for (ULONG i = 0; i < num_entries; i++) {
      if (entries[i].lpOverlapped) {
        QueuedItem *w = (QueuedItem*)((byte*)entries[i].lpOverlapped - offsetof(QueuedItem, overlapped));
        w->queue_cb->OnQueuedItemDelete(w);
      }
    }
  }
}

bool NetworkWin32::HasOutstandingIO() {
  if (udp_socket_.HasOutstandingIO())
    return true;
  for (TcpSocketWin32 *tcp = tcp_socket_; tcp; tcp = tcp->next_)
    if (tcp->HasOutstandingIO())
      return true;
  return false;
}

void NetworkWin32::StartThread() {
  assert(completion_port_handle_);

  DWORD thread_id;
  thread_ = CreateThread(NULL, 0, &NetworkThread, this, 0, &thread_id);
  SetThreadPriority(thread_, ABOVE_NORMAL_PRIORITY_CLASS);
}

void NetworkWin32::StopThread() {
  if (thread_ != NULL) {
    exit_thread_ = true;
    g_fail_malloc_flag = true;
    PostQueuedCompletionStatus(completion_port_handle_, NULL, NULL, NULL);
    WaitForSingleObject(thread_, INFINITE);
    CloseHandle(thread_);
    thread_ = NULL;
    exit_thread_ = false;
    g_fail_malloc_flag = false;
  }
}

void NetworkWin32::WakeUp() {
  PostQueuedCompletionStatus(completion_port_handle_, NULL, NULL, NULL);
}

void NetworkWin32::PostQueuedItem(QueuedItem *item) {
  PostQueuedCompletionStatus(completion_port_handle_, NULL, NULL, &item->overlapped);
}

/////////////////////////////////////////////////////////////////////////

PacketProcessor::PacketProcessor() {
  event_ = CreateEvent(NULL, FALSE, FALSE, NULL);

  last_ptr_ = &first_;
  first_ = NULL;
  exit_code_ = 0;
  timer_interrupt_ = false;
  packets_in_queue_ = 0;
  need_notify_ = 0;
  udp_cb_maybe_deobfuscate_ = &udp_cb_;
}

PacketProcessor::~PacketProcessor() {
  first_ = NULL;
  last_ptr_ = &first_;
  CloseHandle(event_);
}

void CALLBACK PacketProcessor::ThreadPoolTimerCallback(PTP_CALLBACK_INSTANCE iTimerInstance, PVOID pContext, PTP_TIMER) {
  PacketProcessor *th = (PacketProcessor *)pContext;
  th->mutex_.Acquire();
  th->timer_interrupt_ = true;
  if (th->need_notify_) {
    th->need_notify_ = 0;
    th->mutex_.Release();
    SetEvent(th->event_);
    return;
  }
  th->mutex_.Release();
}

void PacketProcessor::Reset() {
  QueuedItem *packet;

  packet = first_;
  first_ = NULL;
  exit_code_ = 0;
  last_ptr_ = &first_;
  timer_interrupt_ = false;

  while (packet) {
    QueuedItem *next = packet->queue_next;
    packet->queue_cb->OnQueuedItemDelete(packet);
    packet = next;
  }
}

int PacketProcessor::Run(WireguardProcessor *wg, TunsafeRunner *runner) {
  int free_packets_ctr = 0;
  int overload = 0;
  int exit_code;
  QueuedItem *packet;
  PTP_TIMER threadpool_timer;
  QueueContext queue_context = {wg, runner};

  threadpool_timer = CreateThreadpoolTimer(&ThreadPoolTimerCallback, this, NULL);
  static const int64 duetime = -10000000; // the unit is 100ns
  SetThreadpoolTimer(threadpool_timer, (FILETIME*)&duetime, 1000, 1000);

  mutex_.Acquire();
  while (!(exit_code = exit_code_)) {
    if (timer_interrupt_) {
      timer_interrupt_ = false;
      need_notify_ = 0;
      mutex_.Release();
      wg->SecondLoop();

      runner->CollectStats();

      // Conserve memory every 10s
      if (free_packets_ctr++ == 10) {
        free_packets_ctr = 0;
        FreeAllPackets();
      }
      if (overload)
        overload -= 1;
    } else if ((packet = first_) == NULL) {
      need_notify_ = 1;
      mutex_.Release();
      WaitForSingleObject(event_, INFINITE);
    } else {
      // Steal the whole work queue
      first_ = NULL;
      last_ptr_ = &first_;
      int packets_in_queue = packets_in_queue_;
      packets_in_queue_ = 0;
      need_notify_ = 0;
      mutex_.Release();

      if (packets_in_queue >= 1024)
        overload = 2;
      queue_context.overload = (overload != 0);

      do {
        QueuedItem *next = packet->queue_next;
        packet->queue_cb->OnQueuedItemEvent(packet, (uintptr_t)&queue_context);
        packet = next;
      } while (packet);
    }
    wg->RunAllMainThreadScheduled();
    mutex_.Acquire();
  }
  mutex_.Release();

  SetThreadpoolTimer(threadpool_timer, nullptr, 0, 0);
  WaitForThreadpoolTimerCallbacks(threadpool_timer, true);
  CloseThreadpoolTimer(threadpool_timer);

  return exit_code;
}

void PacketProcessorTunCb::OnQueuedItemEvent(QueuedItem *qi, uintptr_t extra) {
  PacketProcessor::QueueContext *context = (PacketProcessor::QueueContext *)extra;
  context->wg->HandleTunPacket(static_cast<Packet*>(qi));
}

void PacketProcessorTunCb::OnQueuedItemDelete(QueuedItem *qi) {
  FreePacket(static_cast<Packet*>(qi));
}

void PacketProcessorUdpCb::OnQueuedItemEvent(QueuedItem *qi, uintptr_t extra) {
  PacketProcessor::QueueContext *context = (PacketProcessor::QueueContext *)extra;
  context->wg->HandleUdpPacket(static_cast<Packet*>(qi), context->overload);
}

void PacketProcessorUdpCb::OnQueuedItemDelete(QueuedItem *qi) {
  FreePacket(static_cast<Packet*>(qi));
}

void PacketProcessorDeobfuscateUdpCb::OnQueuedItemEvent(QueuedItem *qi, uintptr_t extra) {
  PacketProcessor::QueueContext *context = (PacketProcessor::QueueContext *)extra;
  Packet *packet = static_cast<Packet*>(qi);
  context->wg->dev().packet_obfuscator().DeobfuscatePacket(packet);
  PacketProcessorUdpCb::OnQueuedItemEvent(qi, extra);
}

void PacketProcessor::PostExit(int exit_code) {
  mutex_.Acquire();
  exit_code_ = exit_code;
  mutex_.Release();
  SetEvent(event_);
}

void PacketProcessor::PostPackets(Packet *first, Packet **end, int count) {
  mutex_.Acquire();
  if (packets_in_queue_ >= HARD_MAXIMUM_QUEUE_SIZE) {
    mutex_.Release();
    FreePackets(first, end, count);
    return;
  }
  assert(first != NULL);
  assert(first_ || last_ptr_ == &first_);
  packets_in_queue_ += count;
  *last_ptr_ = first;
  last_ptr_ = (QueuedItem**)end;
  assert(first_ || last_ptr_ == &first_);
  if (need_notify_) {
    need_notify_ = 0;
    mutex_.Release();
    SetEvent(event_);
    return;
  }
  mutex_.Release();
}

void PacketProcessor::ForcePost(QueuedItem *item) {
  item->queue_next = NULL;
  mutex_.Acquire();
  packets_in_queue_ += 1;
  *last_ptr_ = item;
  last_ptr_ = &item->queue_next;
  if (need_notify_) {
    need_notify_ = 0;
    mutex_.Release();
    SetEvent(event_);
    return;
  }
  mutex_.Release();
}

bool GetNetLuidFromGuid(const char *adapter_guid, NET_LUID *luid) {
  char buffer[64];
  UUID uuid;
  size_t len = strlen(adapter_guid);
  if (adapter_guid[0] != '{' || adapter_guid[len - 1] != '}' || len >= 64) return false;
  buffer[len - 2] = 0;
  memcpy(buffer, adapter_guid + 1, len - 2);
  RPC_STATUS status = UuidFromStringA((RPC_CSTR)buffer, &uuid);
  if (status != 0)
    return false;
  return ConvertInterfaceGuidToLuid((GUID*)&uuid, luid) == 0;
}

DWORD SetMtuOnNetworkAdapter(NET_LUID *InterfaceLuid, ADDRESS_FAMILY family, int new_mtu) {
  MIB_IPINTERFACE_ROW row;
  DWORD err;
  InitializeIpInterfaceEntry(&row);
  row.Family = family;
  row.InterfaceLuid = *InterfaceLuid;
  if ((err = GetIpInterfaceEntry(&row)) == 0) {
    row.NlMtu = new_mtu;
    if (row.Family == AF_INET)
      row.SitePrefixLength = 0;
    err = SetIpInterfaceEntry(&row);
  }
  return err;
}

DWORD SetMetricOnNetworkAdapter(NET_LUID *InterfaceLuid, ADDRESS_FAMILY family, int new_metric, int *old_metric) {
  MIB_IPINTERFACE_ROW row;
  DWORD err;
  if (old_metric)
    *old_metric = kMetricNone;
  InitializeIpInterfaceEntry(&row);
  row.Family = family;
  row.InterfaceLuid = *InterfaceLuid;
  if ((err = GetIpInterfaceEntry(&row)) == 0) {
    if (old_metric)
      *old_metric = row.UseAutomaticMetric ? kMetricAutomatic : row.Metric;
    row.Metric = new_metric;
    row.UseAutomaticMetric = (new_metric == kMetricAutomatic);
    if (row.Family == AF_INET)
      row.SitePrefixLength = 0;
    err = SetIpInterfaceEntry(&row);
  }
  return err;
}

static const char *PrintIPV6(const uint8 new_address[16]) {
  sockaddr_in6 sin6 = {0};
  static char buf[100];
  // cast to void* to work on VS2015
  if (!inet_ntop(PF_INET6, (void*)new_address, buf, 100))
    memcpy(buf, "unknown", 8);
  return buf;
}

static void AssignIpv6Address(const void *new_address, int new_cidr, WgCidrAddr *target) {
  target->size = 128;
  target->cidr = new_cidr;
  memcpy(target->addr, new_address, 16);
}

static int IsIpv6AddressInList(const std::vector<WgCidrAddr> &addresses, const void *ipv6_addr, int cidr) {
  int i = 0;
  for (auto it = addresses.begin(); it != addresses.end(); ++it, i++) {
    if (it->size == 128 && it->cidr == cidr && memcmp(it->addr, ipv6_addr, 16) == 0)
      return i;
  }
  return -1;
}

// Set new_cidr to 0 to clear it.
static bool SetIPV6AddressOnInterface(NET_LUID *InterfaceLuid, const std::vector<WgCidrAddr> &addresses, std::vector<WgCidrAddr> *old_address) {
  NETIO_STATUS Status;
  PMIB_UNICASTIPADDRESS_TABLE table = NULL;

  if (old_address)
    old_address->clear();

  Status = GetUnicastIpAddressTable(AF_INET6, &table);
  if (Status != 0) {
    RERROR("GetUnicastAddressTable Failed. Error %d\n", Status);
    return false;
  }
  uint64 matching_addr = 0;
  for (int i = 0; i < (int)table->NumEntries; i++) {
    MIB_UNICASTIPADDRESS_ROW *row = &table->Table[i];
    if (!memcmp(&row->InterfaceLuid, InterfaceLuid, sizeof(NET_LUID))) {
      if (row->PrefixOrigin == 1 && row->SuffixOrigin == 1) {
        if (old_address) {
          WgCidrAddr tmp;
          AssignIpv6Address(&row->Address.Ipv6.sin6_addr, row->OnLinkPrefixLength, &tmp);
          old_address->push_back(tmp);
        }

        int idx = IsIpv6AddressInList(addresses, &row->Address.Ipv6.sin6_addr, row->OnLinkPrefixLength);
        if (idx >= 0 && idx < 64) {
          matching_addr |= (uint64)1 << idx;
          RINFO("Using IPv6 address: %s/%d", PrintIPV6((uint8*)&row->Address.Ipv6.sin6_addr), row->OnLinkPrefixLength);
          continue;
        }
        Status = DeleteUnicastIpAddressEntry(row);
        if (Status)
          RERROR("Error %d deleting IPv6 address: %s/%d", Status, PrintIPV6((uint8*)&row->Address.Ipv6.sin6_addr), row->OnLinkPrefixLength);
        else
          RINFO("Deleted IPv6 address: %s/%d", PrintIPV6((uint8*)&row->Address.Ipv6.sin6_addr), row->OnLinkPrefixLength);
      }
    }
  }
  FreeMibTable(table);

  // Add all ipv6 addresses that were not already set.
  bool success = true;
  int i = 0;
  for (auto it = addresses.begin(); it != addresses.end(); ++it, i++) {
    // skip it because of wrong type or already set?
    if (it->size != 128 || i < 64 && (matching_addr & ((uint64)1 << i)))
      continue;

    MIB_UNICASTIPADDRESS_ROW Row;
    InitializeUnicastIpAddressEntry(&Row);
    Row.OnLinkPrefixLength = it->cidr;
    Row.Address.si_family = AF_INET6;
    memcpy(&Row.Address.Ipv6.sin6_addr, it->addr, 16);
    Row.InterfaceLuid = *InterfaceLuid;
    Status = CreateUnicastIpAddressEntry(&Row);
    if (Status != 0) {
      RERROR("Error %d setting IPv6 address: %s/%d", Status, PrintIPV6(it->addr), it->cidr);
      success = false;
    } else {
      RINFO("Added IPV6 Address: %s/%d", PrintIPV6(it->addr), it->cidr);
    }
  }
  return success;
}

static bool SetIPV6DnsOnInterface(NET_LUID *InterfaceLuid, const IpAddr *new_address, size_t new_address_size) {
  char buf[128];
  char ipv6[128];
  bool isfirst = true;
  NET_IFINDEX InterfaceIndex;
  if (ConvertInterfaceLuidToIndex(InterfaceLuid, &InterfaceIndex))
    return false;
  if (new_address_size) {
    for (size_t i = 0; i < new_address_size; i++) {
      if (new_address[i].sin.sin_family != AF_INET6)
        continue;
      if (!inet_ntop(AF_INET6, (void*)&new_address[i].sin6.sin6_addr, ipv6, sizeof(ipv6)))
        return false;
      if (isfirst) {
        isfirst = false;
        snprintf(buf, sizeof(buf), "netsh interface ipv6 set dnsservers name=%d static %s validate=no", InterfaceIndex, ipv6);
      } else {
        snprintf(buf, sizeof(buf), "netsh interface ipv6 add dnsservers name=%d %s validate=no", InterfaceIndex, ipv6);
      }
      if (!RunNetsh(buf))
        return false;
    }
    return true;
  } else {
    snprintf(buf, sizeof(buf), "netsh interface ipv6 delete dns name=%d all", InterfaceIndex);
    return RunNetsh(buf);
  }
}

static uint32 ComputeIpv4DefaultRoute(uint32 ip, uint32 netmask) {
  uint32 default_route_v4 = (ip & netmask) | 1;
  if (default_route_v4 == ip)
    default_route_v4++;
  return default_route_v4;
}

static void ComputeIpv6DefaultRoute(const uint8 *ipv6_address, uint8 ipv6_cidr, uint8 *default_route_v6) {
  memcpy(default_route_v6, ipv6_address, 16);
  // clear the last bits of the ipv6 address to match the cidr.
  size_t n = (ipv6_cidr + 7) >> 3;
  memset(&default_route_v6[n], 0, 16 - n);
  if (n == 0)
    return;
  // adjust the final byte
  default_route_v6[n - 1] &= ~(0xff >> (ipv6_cidr & 7));
  // set the very last byte to something
  default_route_v6[15] |= 1;
  // ensure it doesn't collide
  if (memcmp(default_route_v6, ipv6_address, 16) == 0)
    default_route_v6[15] ^= 3;
}


static bool AddMultipleCatchallRoutes(int inet, int bits, const uint8 *target, const NET_LUID &luid, std::vector<MIB_IPFORWARD_ROW2> *undo_array) {
  uint8 tmp[16] = {0};
  bool success = true;
  for (int i = 0; i < (1 << bits); i++) {
    tmp[0] = i << (8 - bits);
    success &= AddRoute(inet, tmp, bits, target, &luid, undo_array);
  }
  return success;
}

TunWin32Adapter::TunWin32Adapter(DnsBlocker *dns_blocker, const char guid[ADAPTER_GUID_SIZE]) {
  handle_ = NULL;
  dns_blocker_ = dns_blocker;
  old_ipv6_metric_ = kMetricNone;
  old_ipv4_metric_ = kMetricNone;
  has_dns6_setting_ = false;
  guid_[0] = 0;
  if (guid)
    memcpy(guid_, guid, ADAPTER_GUID_SIZE);
}

TunWin32Adapter::~TunWin32Adapter() {

}

bool TunWin32Adapter::OpenAdapter(TunsafeRunner *runner, DWORD open_flags) {
  ULONG info[3];
  DWORD len;
  assert(handle_ == NULL);
  backend_ = runner->backend();
  handle_ = OpenTunAdapter(guid_, runner, open_flags);
  if (handle_ != NULL) {
    memset(info, 0, sizeof(info));
    if (DeviceIoControl(handle_, TAP_IOCTL_GET_VERSION, &info, sizeof(info),
                        &info, sizeof(info), &len, NULL)) {
      RINFO("TAP Driver Version %d.%d %s", (int)info[0], (int)info[1], (info[2] ? "(DEBUG)" : ""));
    }

    if (info[0] < 9 || info[0] == 9 && info[1] <= 8) {
      RERROR("TAP is too old. Go to https://tunsafe.com/download to upgrade the driver");
      CloseHandle(handle_);
      handle_ = NULL;
    }
  }
  return (handle_ != NULL);
}

bool TunWin32Adapter::ConfigureAdapter(const TunInterface::TunConfig &&config, TunInterface::TunConfigOut *out) {
  DWORD len, err;
  
  out->enable_neighbor_discovery_spoofing = false;

  if (!RunPrePostCommand(config.pre_post_commands.pre_up)) {
    RERROR("Pre command failed!");
    return false;
  }

  pre_down_ = std::move(config.pre_post_commands.pre_down);
  post_down_ = std::move(config.pre_post_commands.post_down);

  const WgCidrAddr *ipv4_addr = NULL;
  const WgCidrAddr *ipv6_addr = NULL;
  for (auto it = config.addresses.begin(); it != config.addresses.end(); ++it) {
    if (it->size == 32 && ipv4_addr == NULL)
      ipv4_addr = &*it;
    else if (it->size == 128 && ipv6_addr == NULL)
      ipv6_addr = &*it;
  }
  if (ipv4_addr == NULL) {
    RERROR("The TUN adapter on Windows requires an IPv4 address");
    return false;
  }
  
  uint32 ipv4_netmask = CidrToNetmaskV4(ipv4_addr->cidr);
  uint32 ipv4_ip = ReadBE32(ipv4_addr->addr);

  // Set TAP-Windows TUN subnet mode
  if (1) {
    uint32 v[3];
    v[0] = htonl(ipv4_ip);
    v[1] = htonl(ipv4_ip & ipv4_netmask);
    v[2] = htonl(ipv4_netmask);
    if (!DeviceIoControl(handle_, TAP_IOCTL_CONFIG_TUN, v, sizeof(v), v, sizeof(v), &len, NULL)) {
      RERROR("DeviceIoControl(TAP_IOCTL_CONFIG_TUN) failed");
      return false;
    }
  }

  // Set DHCP IP/netmask
  {
    uint32 v[4];
    v[0] = htonl(ipv4_ip);
    v[1] = htonl(ipv4_netmask);
    v[2] = htonl((ipv4_ip | ~ipv4_netmask) - 1); // x.x.x.254
    v[3] = 31536000;                         // One year
    if (!DeviceIoControl(handle_, TAP_IOCTL_CONFIG_DHCP_MASQ, v, sizeof(v), v, sizeof(v), &len, NULL)) {
      RERROR("DeviceIoControl(TAP_IOCTL_CONFIG_DHCP_MASQ) failed");
      return false;
    }
  }

  // Extract and set IPv4 DNS servers through DHCP
  {
    enum { kMaxDnsServers = 4 };
    uint8 dhcp_options[2 + kMaxDnsServers * 4]; // max 4 dns servers
    uint32 num_dns = 0;
    for (auto it = config.dns.begin(); it != config.dns.end(); ++it) {
      if (it->sin.sin_family != AF_INET)
        continue;
      memcpy(&dhcp_options[2 + num_dns * 4], &it->sin.sin_addr, 4);
      if (++num_dns == kMaxDnsServers)
        break;
    }
    if (num_dns != 0) {
      dhcp_options[0] = 6;
      dhcp_options[1] = (uint8)(num_dns * 4);
      DWORD dhcp_options_size = (DWORD)(num_dns * 4 + 2);
      byte output[10];
      if (!DeviceIoControl(handle_, TAP_IOCTL_CONFIG_DHCP_SET_OPT,
        (void*)dhcp_options, dhcp_options_size, output, sizeof(output), &len, NULL)) {
        RERROR("DeviceIoControl(TAP_IOCTL_CONFIG_DHCP_SET_OPT) failed");
        return false;
      }
    }
  }

  // Get device MAC address
  if (!DeviceIoControl(handle_, TAP_IOCTL_GET_MAC, mac_adress_, 6, mac_adress_, sizeof(mac_adress_), &len, NULL)) {
    RERROR("DeviceIoControl(TAP_IOCTL_GET_MAC) failed");
  } else {
    out->enable_neighbor_discovery_spoofing = true;
    memcpy(out->neighbor_discovery_spoofing_mac, mac_adress_, sizeof(out->neighbor_discovery_spoofing_mac));
  }

  // Set driver media status to 'connected'
  ULONG status = TRUE;
  if (!DeviceIoControl(handle_, TAP_IOCTL_SET_MEDIA_STATUS, &status, sizeof(status),
                       &status, sizeof(status), &len, NULL)) {
    RERROR("DeviceIoControl(TAP_IOCTL_SET_MEDIA_STATUS) failed");
    return false;
  }

  bool has_interface_luid = GetNetLuidFromGuid(guid_, &interface_luid_);
  if (!has_interface_luid) {
    RERROR("Unable to determine interface luid for %s.", guid_);
    return false;
  }

  if (config.mtu) {
    err = SetMtuOnNetworkAdapter(&interface_luid_, AF_INET, config.mtu);
    if (err)
      RERROR("SetMtuOnNetworkAdapter IPv4 failed: %d", err);
    if (ipv6_addr) {
      err = SetMtuOnNetworkAdapter(&interface_luid_, AF_INET6, config.mtu);
      if (err)
        RERROR("SetMtuOnNetworkAdapter IPv6 failed: %d", err);
    }
  }

  has_dns6_setting_ = false;
  if (ipv6_addr) {
    SetIPV6AddressOnInterface(&interface_luid_, config.addresses, &old_ipv6_address_);

    // Check if we have at least one ipv6 setting
    for (auto it = config.dns.begin(); it != config.dns.end(); ++it) {
      if (it->sin.sin_family == AF_INET6) {
        has_dns6_setting_ = true;
        break;
      }
    }
    if (has_dns6_setting_ && !SetIPV6DnsOnInterface(&interface_luid_, config.dns.data(), config.dns.size())) {
      RERROR("SetIPV6DnsOnInterface: failed");
    }
  }

  if (config.dns.size() && config.block_dns_on_adapters) {
    RINFO("Blocking standard DNS on all adapters");
    dns_blocker_->BlockDnsExceptOnAdapter(interface_luid_, has_dns6_setting_);

    err = SetMetricOnNetworkAdapter(&interface_luid_, AF_INET, 2, &old_ipv4_metric_);
    if (err)
      RERROR("SetMetricOnNetworkAdapter IPv4 failed: %d", err);

    if (ipv6_addr) {
      err = SetMetricOnNetworkAdapter(&interface_luid_, AF_INET6, 2, &old_ipv6_metric_);
      if (err)
        RERROR("SetMetricOnNetworkAdapter IPv6 failed: %d", err);
    }
  } else {
    dns_blocker_->RestoreDns();
  }
  
  g_killswitch_currconn = config.internet_blocking;
  uint8 ibs = (g_killswitch_currconn == kBlockInternet_Default) ? g_killswitch_want : g_killswitch_currconn;
  
  bool block_all_traffic_route = (ibs & kBlockInternet_Route) != 0;

  RouteInfo ri, ri6;

  // Delete any current /1 default routes and read some stuff from the routing table.
  if (!GetDefaultRouteAndDeleteOldRoutes(AF_INET, &interface_luid_, block_all_traffic_route, &config.excluded_routes, &ri)) {
    RERROR("Unable to read old default gateway and delete old default routes.");
    return false;
  }

  // Delete any current /1 default routes and read some stuff from the routing table.
  if (!GetDefaultRouteAndDeleteOldRoutes(AF_INET6, &interface_luid_, block_all_traffic_route, &config.excluded_routes, &ri6)) {
    RERROR("Unable to read old default gateway and delete old default routes for IPv6.");
  }

  if (block_all_traffic_route) {
    RINFO("Blocking all regular Internet traffic using routing rules");
    NET_LUID localhost_luid;
    if (ConvertInterfaceIndexToLuid(1, &localhost_luid) || localhost_luid.Info.IfType != 24) {
      RERROR("Unable to get localhost luid - while adding route based blocking.");
    } else {
      g_killswitch_curr |= kBlockInternet_Route;

      uint32 dst[4] = {0};

      if (!AddMultipleCatchallRoutes(AF_INET, 1, (uint8*)&dst, localhost_luid, NULL)) {
        RERROR("Unable to add routes for route based blocking.");
        DeactivateKillSwitch(0);
        return false;
      }

      if (!AddMultipleCatchallRoutes(AF_INET6, 1, (uint8*)&dst, localhost_luid, NULL)) {
        RERROR("Unable to add IPv6 routes for route based blocking.");
        DeactivateKillSwitch(0);
        return false;
      }
    }
  }

  if (ibs & kBlockInternet_Firewall) {
    RINFO("Blocking all regular Internet traffic using firewall rules");
    g_killswitch_curr |= kBlockInternet_Firewall;
    if (!AddKillSwitchFirewall(interface_luid_, true, (ibs & kBlockInternet_AllowLocalNetworks) != 0)) {
      RERROR("Unable to activate firewall based kill switch");
      DeactivateKillSwitch(0);
      return false;
    }
  }

  DeactivateKillSwitch(ibs);

  uint8 default_route_v4[4];
  uint8 default_route_v6[16];

  WriteBE32(default_route_v4, ComputeIpv4DefaultRoute(ipv4_ip, ipv4_netmask));
  if (ipv6_addr)
    ComputeIpv6DefaultRoute(ipv6_addr->addr, ipv6_addr->cidr, default_route_v6);

  // Add all the routes that should go through the VPN
  for (auto it = config.included_routes.begin(); it != config.included_routes.end(); ++it) {
    if (it->cidr == 0) {
      // /0 gets changed to two /1 routes, to avoid overwriting the system's default route
      if (it->size == 32) {
        if (!AddMultipleCatchallRoutes(AF_INET, block_all_traffic_route ? 2 : 1, default_route_v4, interface_luid_, &routes_to_undo_))
          RERROR("Unable to add new default ipv4 route.");
      } else if (it->size == 128 && ipv6_addr) {
        if (!AddMultipleCatchallRoutes(AF_INET6, block_all_traffic_route ? 2 : 1, default_route_v6, interface_luid_, &routes_to_undo_))
          RERROR("Unable to add new default ipv6 route.");
      }
      continue;
    }
    // Avoid adding a route if it's a subset of the address
    if (IsWgCidrAddrSubsetOfAny(*it, config.addresses))
      continue;

    if (it->size == 32) {
      AddRoute(AF_INET, it->addr, it->cidr, default_route_v4, &interface_luid_, &routes_to_undo_);
    } else if (it->size == 128 && ipv6_addr) {
      AddRoute(AF_INET6, it->addr, it->cidr, default_route_v6, &interface_luid_, &routes_to_undo_);
    }
  }

  // Add all the routes that should bypass vpn
  int warned = 0;
  for (auto it = config.excluded_routes.begin(); it != config.excluded_routes.end(); ++it) {
    if (it->size == 32) {
      if (ri.found_default_adapter) {
        AddRoute(AF_INET, it->addr, it->cidr, ri.default_gw, &ri.default_adapter, &routes_to_undo_);
      } else if (!(warned & 1)) {
        warned |= 1;
        RERROR("Unable to read old ipv4 default gateway");
      }
    } else if (it->size == 128) {
      if (ri6.found_default_adapter) {
        AddRoute(AF_INET6, it->addr, it->cidr, ri6.default_gw, &ri6.default_adapter, &routes_to_undo_);
      } else if (!(warned & 2)) {
        warned |= 2;
        RERROR("Unable to read old ipv6 default gateway");
      }
    }
  }

  NET_IFINDEX InterfaceIndex;
  if (ConvertInterfaceLuidToIndex(&interface_luid_, &InterfaceIndex)) {
    RERROR("Unable to get index of adapter");
    return false;
  }
  if ((err = FlushIpNetTable2(AF_INET, InterfaceIndex)) != NO_ERROR) {
    RERROR("FlushIpNetTable failed: 0x%X", err);
    return false;
  }
  if (ipv6_addr != NULL) {
    if ((err = FlushIpNetTable2(AF_INET6, InterfaceIndex)) != NO_ERROR) {
      RERROR("FlushIpNetTable failed: 0x%X", err);
      return false;
    }
  }

  RunPrePostCommand(config.pre_post_commands.post_up);
  return true;
}

void TunWin32Adapter::CloseAdapter(bool is_restart) {
  RunPrePostCommand(pre_down_);

  if (handle_ != NULL) {
    ULONG status = FALSE;
    DWORD len;
    DeviceIoControl(handle_, TAP_IOCTL_SET_MEDIA_STATUS, &status, sizeof(status),
                    &status, sizeof(status), &len, NULL);
    CloseHandle(handle_);
    handle_ = NULL;

    TunAdaptersInUse::GetInstance()->Release(backend_);
  }

  if (old_ipv6_address_.size())
    SetIPV6AddressOnInterface(&interface_luid_, old_ipv6_address_, NULL);
  if (old_ipv4_metric_ != kMetricNone)
    SetMetricOnNetworkAdapter(&interface_luid_, AF_INET, old_ipv4_metric_, NULL);
  if (old_ipv6_metric_ != kMetricNone)
    SetMetricOnNetworkAdapter(&interface_luid_, AF_INET6, old_ipv6_metric_, NULL);
  if (has_dns6_setting_)
    SetIPV6DnsOnInterface(&interface_luid_, NULL, 0);

  old_ipv4_metric_ = old_ipv6_metric_ = -1;
  old_ipv6_address_.clear();
  has_dns6_setting_ = false;

  for (auto it = routes_to_undo_.begin(); it != routes_to_undo_.end(); ++it)
    DeleteRoute(&*it);
  routes_to_undo_.clear();

  if (!is_restart && dns_blocker_)
    dns_blocker_->RestoreDns();
  
  RunPrePostCommand(post_down_);

  pre_down_.clear();
  post_down_.clear();
}

static bool RunOneCommand(const std::string &cmd) {
  std::string command = "cmd.exe /C " + cmd;

  STARTUPINFOA si = {0};
  PROCESS_INFORMATION pi = {0};

  HANDLE hstdout_wr = NULL, hstdout_rd = NULL;
  HANDLE hstdin_wr = NULL, hstdin_rd = NULL;

  bool result = false;

  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  if (!CreatePipe(&hstdout_rd, &hstdout_wr, &saAttr, 0) ||
      !CreatePipe(&hstdin_rd, &hstdin_wr, &saAttr, 0) ||
      !SetHandleInformation(hstdout_rd, HANDLE_FLAG_INHERIT, 0) ||
      !SetHandleInformation(hstdin_wr, HANDLE_FLAG_INHERIT, 0)) {
    goto out;
  }

  CloseHandle(hstdin_wr);
  hstdin_wr = NULL;
  
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdError = hstdout_wr;
  si.hStdOutput = hstdout_wr;
  si.hStdInput = hstdin_rd;

  RINFO("Run: %s", cmd.c_str());
  if (CreateProcessA(NULL, &command[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
    DWORD exit_code = -1;
    char buf[1024];
    DWORD bufend = 0, bufstart = 0;
    
    CloseHandle(hstdout_wr);
    hstdout_wr = NULL;

    for (;;) {
      DWORD bytes_read = 0;
      bool foundeof = (!ReadFile(hstdout_rd, buf + bufend, sizeof(buf) - bufend, &bytes_read, NULL) || bytes_read == 0);
      bufend += bytes_read;
      for(;;) {
        char *nl = (char*)memchr(buf + bufstart, '\n', bufend - bufstart);
        if (!nl)
          break;
        char *st = buf + bufstart;
        char *nl2 = nl;
        if (nl != buf + bufstart && nl[-1] == '\r')
          nl--;
        bufstart = (DWORD)(nl2 - buf + 1);
        RINFO("%.*s", nl - st, st);
      }
      if (bufend - bufstart == sizeof(buf) || foundeof) {
        if (bufend - bufstart)
          RINFO("%.*s", buf + bufstart, bufend - bufstart);
        bufstart = bufend = 0;
      }
      if (foundeof)
        break;
      if (bufstart) {
        bufend -= bufstart;
        memmove(buf, buf + bufstart, bufend);
        bufstart = 0;
      }
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (exit_code != 0) {
      RERROR("Command line failed (%d) : %s", exit_code, cmd.c_str());
    } else {
      result = true;
    }
  } else {
    RERROR("CreateProcess failed: %s", cmd.c_str());
  }
  CloseHandle(hstdout_rd);
  CloseHandle(hstdout_wr);
  CloseHandle(hstdin_rd);
  CloseHandle(hstdin_wr);
out:
  return result;
}

bool TunWin32Adapter::RunPrePostCommand(const std::vector<std::string> &vec) {
  bool success = true;
  for (auto it = vec.begin(); it != vec.end(); ++it) {
    if (!g_allow_pre_post) {
      RERROR("Pre/Post commands are disabled. Ignoring: %s", it->c_str());
    } else {
      success &= RunOneCommand(*it);
    }
  }
  return success;
}

//////////////////////////////////////////////////////////////////////////////

TunWin32Iocp::TunWin32Iocp(DnsBlocker *blocker, TunsafeRunner *runner) : adapter_(blocker, runner->backend()->guid_), runner_(runner) {
  wqueue_end_ = &wqueue_;
  wqueue_ = NULL;
  wqueue_size_ = 0;

  thread_ = NULL;
  completion_port_handle_ = NULL;
  packet_handler_ = NULL;
  exit_thread_ = false;
  did_show_tun_queue_warning_ = false;
}

TunWin32Iocp::~TunWin32Iocp() {
  //assert(num_reads_ == 0 && num_writes_ == 0);
  assert(thread_ == NULL);
  CloseTun(false);
}

bool TunWin32Iocp::Configure(const TunConfig &&config, TunConfigOut *out) {
  // Reconfigure while started?
  if (thread_ != NULL) {
    assert(completion_port_handle_);
    StopThread();
    bool rv = Configure(std::move(config), out);
    StartThread();
    return rv;
  }
  CloseTun(true);
  if (adapter_.OpenAdapter(runner_, FILE_FLAG_OVERLAPPED)) {
    completion_port_handle_ = CreateIoCompletionPort(adapter_.handle(), NULL, NULL, 0);
    if (completion_port_handle_ != NULL) {
      if (adapter_.ConfigureAdapter(std::move(config), out))
        return true;
    }
  }
  CloseTun(false);
  return false;
}

void TunWin32Iocp::CloseTun(bool is_restart) {
  assert(thread_ == NULL);
  adapter_.CloseAdapter(is_restart);
  if (completion_port_handle_)
    CloseHandle(exch_null(completion_port_handle_));
  FreePacketList(wqueue_);
}

enum {
  kTunGetQueuedCompletionStatusSize = kConcurrentWriteTap + kConcurrentReadTap + 1
};

static inline bool AllocPacketFrom(Packet **list, int *counter, bool *exit_flag, Packet **res) {
  Packet *p;
  if (p = *list) {
    *list = Packet_NEXT(p);
    (*counter)--;
    p->data = p->data_buf;
  } else {
    if (!(p = AllocPacket()))
      return false;
  }
  *res = p;
  return true;
}

void TunWin32Iocp::ThreadMain() {
  OVERLAPPED_ENTRY entries[kTunGetQueuedCompletionStatusSize];
  Packet *pending_writes = NULL;
  int num_reads = 0, num_writes = 0;
  Packet *finished_reads = NULL, **finished_reads_end;
  Packet *freed_packets = NULL, **freed_packets_end;
  int freed_packets_count = 0;
  DWORD err;
  if (!completion_port_handle_)
    return;

  while (!exit_thread_) {
    // Initiate more reads, reusing the Packet structures in |finished_writes|.
    for (int i = num_reads; i < kConcurrentReadTap; i++) {
      Packet *p;
      if (!AllocPacketFrom(&freed_packets, &freed_packets_count, &exit_thread_, &p))
        break;
      ClearOverlapped(&p->overlapped);
      p->userdata = 0;
      if (!ReadFile(adapter_.handle(), p->data, kPacketCapacity, NULL, &p->overlapped) && (err = GetLastError()) != ERROR_IO_PENDING) {
        FreePacket(p);

        RERROR("TunWin32: ReadFile failed 0x%X", err);

        if (err == ERROR_OPERATION_ABORTED || err == ERROR_FILE_NOT_FOUND) {
          RERROR("TAP driver stopped communicating. Attempting to restart.", err);
          // This can happen if we reinstall the TAP driver while there's an active connection.
          runner_->PostTunRestart();
          goto EXIT;
        }
      } else {
        num_reads++;
      }
    }

    assert(freed_packets_count >= 0);
    if (freed_packets_count >= 32) {
      FreePackets(freed_packets, freed_packets_end, freed_packets_count);
      freed_packets_count = 0;
      freed_packets_end = &freed_packets;
    } else if (freed_packets == NULL) {
      assert(freed_packets_count == 0);
      freed_packets_end = &freed_packets;
    }

    ULONG num_entries = 0;
    if (!GetQueuedCompletionStatusEx(completion_port_handle_, entries, kTunGetQueuedCompletionStatusSize, &num_entries, INFINITE, FALSE)) {
      RINFO("GetQueuedCompletionStatusEx failed.");
      break;
    }
    finished_reads_end = &finished_reads;
    int finished_reads_count = 0;

    // Go through the finished entries and determine which ones are reads, and which ones are writes.
    for (ULONG i = 0; i < num_entries; i++) {
      if (!entries[i].lpOverlapped)
        continue; // This is the dummy entry from |PostQueuedCompletionStatus|
      Packet *p = (Packet*)((byte*)entries[i].lpOverlapped - offsetof(Packet, overlapped));
      if (p->userdata == 0) {
        num_reads--;
        if ((int)p->overlapped.Internal != 0) {
          RERROR("TunWin32::ReadComplete error 0x%X", (int)p->overlapped.Internal);
          FreePacket(p);
          continue;
        }
        p->size = (int)p->overlapped.InternalHigh;
        p->queue_cb = packet_handler_->tun_queue();
        *finished_reads_end = p;
        finished_reads_end = &Packet_NEXT(p);
        finished_reads_count++;
      } else {
        num_writes--;
        if ((int)p->overlapped.Internal != 0) {
          RERROR("TunWin32::WriteComplete error 0x%X", (int)p->overlapped.Internal);
          FreePacket(p);
          continue;
        }
        freed_packets_count++;
        *freed_packets_end = p;
        freed_packets_end = &Packet_NEXT(p);
      }
    }
    *finished_reads_end = NULL;
    *freed_packets_end = NULL;

    if (finished_reads != NULL)
      packet_handler_->PostPackets(finished_reads, finished_reads_end, finished_reads_count);

    // Initiate more writes from |wqueue_|
    while (num_writes < kConcurrentWriteTap) {
      // Refill from queue if empty, avoid taking the mutex if it looks empty 
      if (!pending_writes) {
        if (!wqueue_)
          break;
        mutex_.Acquire();
        pending_writes = wqueue_;
        wqueue_end_ = &wqueue_;
        wqueue_ = NULL;
        wqueue_size_ = 0;
        mutex_.Release();
        if (!pending_writes)
          break;
      }
      // Then issue writes
      Packet *p = pending_writes;
      pending_writes = Packet_NEXT(p);
      ClearOverlapped(&p->overlapped);
      p->userdata = 1;
      if (!WriteFile(adapter_.handle(), p->data, p->size, NULL, &p->overlapped) && (err = GetLastError()) != ERROR_IO_PENDING) {
        RERROR("TunWin32: WriteFile failed 0x%X", err);
        FreePacket(p);
      } else {
        num_writes++;
      }
    }
  }

EXIT:
  // Cancel all IO and wait for all completions
  CancelIo(adapter_.handle());
  while (num_reads + num_writes) {
    ULONG num_entries = 0;
    if (!GetQueuedCompletionStatusEx(completion_port_handle_, entries, 1, &num_entries, INFINITE, FALSE)) {
      RINFO("GetQueuedCompletionStatusEx failed.");
      break;
    }
    if (!entries[0].lpOverlapped)
      continue; // This is the dummy entry from |PostQueuedCompletionStatus|
    Packet *p = (Packet*)((byte*)entries[0].lpOverlapped - offsetof(Packet, overlapped));
    if (p->userdata == 0) {
      num_reads--;
    } else {
      num_writes--;
    }
    FreePacket(p);
  }

  FreePacketList(freed_packets);
  FreePacketList(pending_writes);
}

DWORD WINAPI TunWin32Iocp::TunThread(void *x) {
  TunWin32Iocp *xx = (TunWin32Iocp *)x;
  xx->ThreadMain();
  return 0;
}

void TunWin32Iocp::StartThread() {
  DWORD thread_id;
  assert(thread_ == NULL);
  assert(completion_port_handle_ != NULL);
  thread_ = CreateThread(NULL, 0, &TunThread, this, 0, &thread_id);
  SetThreadPriority(thread_, ABOVE_NORMAL_PRIORITY_CLASS);
}

void TunWin32Iocp::StopThread() {
  exit_thread_ = true;
  PostQueuedCompletionStatus(completion_port_handle_, NULL, NULL, NULL);
  WaitForSingleObject(thread_, INFINITE);
  CloseHandle(thread_);
  thread_ = NULL;
  exit_thread_ = false;
}

void TunWin32Iocp::WriteTunPacket(Packet *packet) {
  Packet_NEXT(packet) = NULL;
  mutex_.Acquire();
  if (wqueue_size_ >= HARD_MAXIMUM_TUN_QUEUE_SIZE) {
    mutex_.Release();
    FreePacket(packet);
    if (!did_show_tun_queue_warning_) {
      did_show_tun_queue_warning_ = true;
      RERROR("TUN Queue Overload! This might happen if you use the NDIS6 driver on Windows 7.");
    }
    return;
  }
  wqueue_size_++;

  Packet *was_empty = wqueue_;
  *wqueue_end_ = packet;
  wqueue_end_ = &Packet_NEXT(packet);
  mutex_.Release();
  if (was_empty == NULL) {
    // Notify the worker thread that it should attempt more writes
    PostQueuedCompletionStatus(completion_port_handle_, NULL, NULL, NULL);
  }
}

//////////////////////////////////////////////////////////////////////////////

TunsafeRunner::TunsafeRunner(TunsafeBackendWin32 *backend)
  : backend_(backend),
    tun_(&backend->dns_blocker_, this),
    wg_proc_(this, &tun_, this),
    plugin_(CreateTunsafePlugin(this, &wg_proc_)),
    tcp_socket_queue_(&net_, &wg_proc_.dev().packet_obfuscator()) {

  wg_proc_.dev().SetPlugin(plugin_);

  net_.udp().SetPacketHandler(&packet_processor_);
  tcp_socket_queue_.SetPacketHandler(&packet_processor_);
  tun_.SetPacketHandler(&packet_processor_);
}

TunsafeRunner::~TunsafeRunner() {
  wg_proc_.dev().SetCurrentThreadAsMainThread();
  delete plugin_;
}

bool TunsafeRunner::Configure(int listen_port, int listen_port_tcp) {
  if (listen_port_tcp)
    RERROR("ListenPortTCP not supported in this version");
  return net_.udp().Configure(listen_port);
}

void TunsafeRunner::WriteUdpPacket(Packet *packet) {
  if (packet->protocol & kPacketProtocolUdp) {
    if (wg_proc_.dev().packet_obfuscator().enabled())
      wg_proc_.dev().packet_obfuscator().ObfuscatePacket(packet);
    net_.udp().WriteUdpPacket(packet);
  } else {
    tcp_socket_queue_.WritePacket(packet);
  }
}

void TunsafeRunner::OnConnected() {
  TunsafeBackendWin32 *backend = backend_;
  if (backend->status() != TunsafeBackend::kStatusConnected) {
    const WgCidrAddr *ipv4_addr = NULL;
    for (const WgCidrAddr &x : wg_proc_.addr()) {
      if (x.size == 32) {
        ipv4_addr = &x;
        break;
      }
    }
    backend->ipv4_ip_ = ipv4_addr ? ReadBE32(ipv4_addr->addr) : 0;
    if (backend->status() != TunsafeBackend::kStatusReconnecting) {
      char buf[kSizeOfAddress];
      RINFO("Connection established. IP %s", ipv4_addr ? print_ip_prefix(buf, AF_INET, ipv4_addr->addr, -1) : "(none)");
    }
    backend->SetStatus(TunsafeBackend::kStatusConnected);
  }
}

void TunsafeRunner::OnConnectionRetry(uint32 attempts) {
  TunsafeBackendWin32 *backend = backend_;
  if (backend->status() == TunsafeBackend::kStatusInitializing)
    backend->SetStatus(TunsafeBackend::kStatusConnecting);
  else if (attempts >= 3 && backend->status() == TunsafeBackend::kStatusConnected)
    backend->SetStatus(TunsafeBackend::kStatusReconnecting);
}


bool TunsafeRunner::Start() {
  wg_proc_.dev().SetCurrentThreadAsMainThread();

  if (config_file_.size()) {
    if (config_file_is_text_format_) {
      if (!ParseWireGuardConfigString(&wg_proc_, config_file_.c_str(), config_file_.size(), &backend_->dns_resolver_))
        return false;
    } else {
      if (!ParseWireGuardConfigFile(&wg_proc_, config_file_.c_str(), &backend_->dns_resolver_))
        return false;
    }
  }
  if (wg_proc_.dev().packet_obfuscator().enabled())
    packet_processor_.EnableDeobfuscation();
  
  if (!wg_proc_.Start())
    return false;

  backend_->SetPublicKey(wg_proc_.dev().public_key());

  net_.StartThread();
  tun_.StartThread();
  int stop_mode = packet_processor_.Run(&wg_proc_, this);
  net_.StopThread();
  tun_.StopThread();

  if (stop_mode != TunsafeBackendWin32::MODE_EXIT)
    tun_.adapter().DisassociateDnsBlocker();
  else
    backend_->dns_resolver_.ClearCache();

  return true;
}

void TunsafeRunner::PostTunRestart() {
  QueuedItem *qi = new QueuedItem;
  qi->queue_cb = this;
  packet_processor_.ForcePost(qi);
}

void TunsafeRunner::OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) {
  backend_->SetStatus(TunsafeBackend::kStatusTunRetrying);
  RINFO("Restarting TUN adapter");
  Sleep(1000);
  wg_proc_.ConfigureTun();
  delete ow;
}

void TunsafeRunner::OnQueuedItemDelete(QueuedItem *ow) {

  delete ow;
}


void TunsafeRunner::OnRequestToken(WgPeer *peer, uint32 type) {
  backend_->OnRequestToken(peer, type);
}

void TunsafeRunner::CollectStats() {
  backend_->CollectStats();
}

void TunsafeRunner::SetConfigFile(const char *file, bool is_text_format) {
  config_file_is_text_format_ = is_text_format;
  config_file_ = file;
}

//////////////////////////////////////////////////////////////////////////////


TunsafeBackend::TunsafeBackend() {
  is_started_ = false;
  is_remote_ = false;
  ipv4_ip_ = 0;
  status_ = kStatusStopped;
  memset(public_key_, 0, sizeof(public_key_));
}

TunsafeBackend::~TunsafeBackend() {
  
}

static bool GetKillSwitchRouteActive() {
  RouteInfo ri;
  return (GetDefaultRouteAndDeleteOldRoutes(AF_INET, NULL, TRUE, NULL, &ri) && ri.found_null_routes == 2);
}

static void RemoveKillSwitchRoute() {
  RouteInfo ri;
  GetDefaultRouteAndDeleteOldRoutes(AF_INET, NULL, FALSE, NULL, &ri);
  GetDefaultRouteAndDeleteOldRoutes(AF_INET6, NULL, FALSE, NULL, &ri);
}

TunsafeBackendWin32::TunsafeBackendWin32(Delegate *delegate) : delegate_(delegate), dns_resolver_(&dns_blocker_) {
  memset(&stats_, 0, sizeof(stats_));
  token_request_ = 0;
  runner_ = NULL;
  InitPacketMutexes();
  worker_thread_ = NULL;
  last_tun_adapter_failed_ = 0;
  want_periodic_stats_ = false;
  guid_[0] = 0;
  if (g_hklm_reg_key == NULL) {
    RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\TunSafe", NULL, NULL, 0, KEY_ALL_ACCESS, NULL, &g_hklm_reg_key, NULL);
    g_killswitch_want = RegReadInt(g_hklm_reg_key, "KillSwitch", 0);
    g_killswitch_curr = GetKillSwitchRouteActive() * kBlockInternet_Route +
                        GetKillSwitchFirewallActive() * kBlockInternet_Firewall;
  }
  delegate_->OnStateChanged();
}

TunsafeBackendWin32::~TunsafeBackendWin32() {
  StopInner(false);
  TunAdaptersInUse::GetInstance()->Release(this);
}

void TunsafeBackendWin32::SetPublicKey(const uint8 key[32]) {
  memcpy(public_key_, key, 32);
  delegate_->OnStateChanged();
}

void TunsafeBackendWin32::CollectStats() {
  stats_mutex_.Acquire();
  stats_ = runner_->wg_proc_.GetStats();
  float data[2] = {
    // unit is megabits/second
    stats_.data_bytes_out_per_second * (1.0f / 125000),
    stats_.data_bytes_in_per_second * (1.0f / 125000),
  };
  stats_collector_.AddSamples(data);
  stats_mutex_.Release();

  delegate_->OnGraphAvailable();
  PushStats();
}

DWORD WINAPI TunsafeBackendWin32::WorkerThread(void *bk) {
  TunsafeBackendWin32 *backend = (TunsafeBackendWin32*)bk;
 
  if (!backend->runner_->Start()) {
    backend->SetStatus(TunsafeBackend::kErrorInitialize);
    backend->dns_blocker_.RestoreDns();
  }
  return 0;
}

void TunsafeBackendWin32::SetStatus(StatusCode status) {
  status_ = status;
  delegate_->OnStatusCode(status);
}

bool TunsafeBackendWin32::Configure() {
  // it's always initialized
  return true;
}

void TunsafeBackendWin32::Teardown() {

}

bool TunsafeBackendWin32::SetTunAdapterName(const char *name) {
  assert(worker_thread_ == NULL);
  size_t len = strlen(name);
  if (len >= sizeof(guid_) || guid_[0])
    return false;
  if (!TunAdaptersInUse::GetInstance()->Acquire(name, this))
    return false;
  memcpy(guid_, name, len + 1);
  return true;
}


void TunsafeBackendWin32::RequestStats(bool enable) {
  want_periodic_stats_ = enable;
  PushStats();
}

void TunsafeBackendWin32::PushStats() {
  if (want_periodic_stats_) {
    stats_mutex_.Acquire();
    WgProcessorStats stats = stats_;
    stats_mutex_.Release();
    delegate_->OnGetStats(stats);
  }
}

void TunsafeBackendWin32::Stop() {
  StopInner(false);
  delegate_->OnStatusCode(status_);
  delegate_->OnStateChanged();
}

void TunsafeBackendWin32::Start(const char *config_file) {
  StopInner(true);
  dns_resolver_.ResetCancel();
  g_killswitch_currconn = kBlockInternet_Default;
  is_started_ = true;
  token_request_ = 0;
  memset(public_key_, 0, sizeof(public_key_));
  SetStatus(kStatusInitializing);
  delegate_->OnClearLog();
  DWORD thread_id;

  runner_ = new TunsafeRunner(this);

  // Connect to a server given by an ID.
  if (strncmp(config_file, ":srv:", 5) == 0) {
//    config_file_is_text_format_ = true;
//    auto server = GetServerById(config_file + 5, NULL);
//    config_file_ = GetServerConfigFile(server);
  } else {
    runner_->SetConfigFile(config_file, false);
  }
  
  worker_thread_ = CreateThread(NULL, 0, &WorkerThread, this, 0, &thread_id);
  SetThreadPriority(worker_thread_, THREAD_PRIORITY_ABOVE_NORMAL);
  delegate_->OnStateChanged();
}


void TunsafeBackendWin32::StopInner(bool is_restart) {
  if (runner_) {
    ipv4_ip_ = 0;
    dns_resolver_.Cancel();
    runner_->packet_processor_.PostExit(is_restart ? MODE_RESTART : MODE_EXIT);
    WaitForSingleObject(worker_thread_, INFINITE);
    CloseHandle(exch_null(worker_thread_));
    is_started_ = false;
    status_ = kStatusStopped;
    delete runner_;
    runner_ = NULL;

    FreeAllPackets();

    uint8 wanted_ibs = (g_killswitch_currconn == kBlockInternet_Default) ? g_killswitch_want : g_killswitch_currconn;
    if (!is_restart && !(wanted_ibs & kBlockInternet_BlockOnDisconnect))
      DeactivateKillSwitch(kBlockInternet_Off);
  }
}

void TunsafeBackendWin32::ResetStats() {
}

LinearizedGraph *TunsafeBackendWin32::GetGraph(int type) {
  if (type < 0 || type >= 4)
    return NULL;
  
  size_t size = sizeof(LinearizedGraph) + 2 * (sizeof(uint32) + sizeof(float) * 120);
  LinearizedGraph *graph = (LinearizedGraph *)malloc(size);
  if (graph) {
    graph->total_size = (uint32)size;
    graph->num_charts = 2;
    graph->graph_type = type;
    memset(graph->reserved, 0, sizeof(graph->reserved));
    stats_mutex_.Acquire();

    uint8 *ptr = (uint8*)(graph + 1);
    for (size_t i = 0; i < 2; i++) {
      *(uint32*)ptr = 120;
      ptr += 4;
      const StatsCollector::TimeSeries *series = stats_collector_.GetTimeSeries((int)i, type);
      memcpy(postinc(ptr, (series->size - series->shift) * sizeof(float)),
             series->data + series->shift,
             (series->size - series->shift) * sizeof(float));
      memcpy(postinc(ptr, series->shift * sizeof(float)), series->data, series->shift * sizeof(float));
    }
    stats_mutex_.Release();
  }
  return graph;
}

InternetBlockState TunsafeBackendWin32::GetInternetBlockState() {
  return (InternetBlockState)(g_killswitch_want | (g_killswitch_curr ? kBlockInternet_Active : 0));
}

static void DeactivateKillSwitch(uint32 want) {
  // Disable blocking without reconnecting
  uint32 maybeon = g_killswitch_curr;
  if ((maybeon & kBlockInternet_Route) > (want & kBlockInternet_Route)) {
    if (g_killswitch_curr & kBlockInternet_Route) {
      g_killswitch_curr &= ~kBlockInternet_Route;
      RINFO("Removing the routing rule internet block");
    }
    RemoveKillSwitchRoute();
  }
  if ((maybeon & kBlockInternet_Firewall) > (want & kBlockInternet_Firewall)) {
    if (g_killswitch_curr & kBlockInternet_Firewall) {
      g_killswitch_curr &= ~kBlockInternet_Firewall;
      RINFO("Removing the firewall internet block");
    }
    RemoveKillSwitchFirewall();
  }
}

void TunsafeBackendWin32::SetInternetBlockState(InternetBlockState want) {
  if (worker_thread_ == NULL && !(want & kBlockInternet_BlockOnDisconnect) || !(want & kBlockInternet_Active))
    DeactivateKillSwitch(kBlockInternet_Off);
  else
    DeactivateKillSwitch(want);

  int value = want & 0xff;
  g_killswitch_want = value;
  RegWriteInt(g_hklm_reg_key, "KillSwitch", (int)value);

  delegate_->OnStateChanged();
}

void TunsafeBackendWin32::SetServiceStartupFlags(uint32 flags) {
  // not used
}

std::string TunsafeBackendWin32::GetConfigFileName() {
  return std::string();
}

struct ConfigQueueItem : QueuedItem, QueuedItemCallback {
  virtual void OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) override;
  virtual void OnQueuedItemDelete(QueuedItem *ow) override;

  enum Type {
    SendConfigurationProtocolPacket,
    SubmitToken
  };
  Type type;
  std::string message;
  uint32 ident;
};

void ConfigQueueItem::OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) {
  PacketProcessor::QueueContext *context = (PacketProcessor::QueueContext *)extra;
  
  if (type == SendConfigurationProtocolPacket) {
    std::string reply;
    WgConfig::HandleConfigurationProtocolMessage(context->wg, std::move(message), &reply);
    context->runner->backend()->delegate_->OnConfigurationProtocolReply(ident, std::move(reply));
  } else {
    context->runner->plugin()->SubmitToken((const uint8*)message.data(), message.size());
  }
  delete this;
}

void ConfigQueueItem::OnQueuedItemDelete(QueuedItem *ow) {
  delete this;
}

void TunsafeBackendWin32::SendConfigurationProtocolPacket(uint32 identifier, const std::string &&message) {
  if (runner_) {
    ConfigQueueItem *queue_item = new ConfigQueueItem;
    queue_item->type = ConfigQueueItem::SendConfigurationProtocolPacket;
    queue_item->ident = identifier;
    queue_item->message = std::move(message);
    queue_item->queue_cb = queue_item;
    runner_->packet_processor_.ForcePost(queue_item);
  }
}

void TunsafeBackendWin32::SubmitToken(const std::string &&message) {
  if (runner_) {
    // Clear out the old token request so GetTokenRequest returns zero.
    token_request_ = 0;

    ConfigQueueItem *queue_item = new ConfigQueueItem;
    queue_item->type = ConfigQueueItem::SubmitToken;
    queue_item->message = std::move(message);
    queue_item->queue_cb = queue_item;
    runner_->packet_processor_.ForcePost(queue_item);
  }
}

uint32 TunsafeBackendWin32::GetTokenRequest() {
  return token_request_;
}

// This is called on the wireguard thread whenever it needs a token,
// it should reschedule
void TunsafeBackendWin32::OnRequestToken(WgPeer *peer, uint32 type) {
  token_request_ = type;
  delegate_->OnStateChanged();
}

void TunsafeBackend::Delegate::DoWork() {
  // implemented by subclasses
}

TunsafeBackendDelegateThreaded::TunsafeBackendDelegateThreaded(TunsafeBackend::Delegate *delegate, const std::function<void(void)> &callback) {
  callback_ = callback;
  delegate_ = delegate;
}

TunsafeBackendDelegateThreaded::~TunsafeBackendDelegateThreaded() {
  for (auto it = incoming_entry_.begin(); it != incoming_entry_.end(); ++it)
    FreeEntry(&*it);
}

void TunsafeBackendDelegateThreaded::FreeEntry(Entry *e) {
  if (e->lparam) {
    if (e->which == Id_OnConfigurationProtocolReply)
      delete (std::string*)e->lparam;
    else
      free((void*)e->lparam);
    e->lparam = NULL;
  }
}

void TunsafeBackendDelegateThreaded::DoWork() {
  mutex_.Acquire();
  std::swap(incoming_entry_, processing_entry_);
  mutex_.Release();
  TunsafeBackend::Delegate *delegate = delegate_;
  for (auto it = processing_entry_.begin(); it != processing_entry_.end(); ++it) {
    switch (it->which) {
      case Id_OnGetStats:         delegate->OnGetStats(*(WgProcessorStats*)it->lparam); break;
      case Id_OnStateChanged:     delegate->OnStateChanged(); break;
      case Id_OnLogLine:          delegate->OnLogLine((const char**)&it->lparam); break;
      case Id_OnStatusCode:       delegate->OnStatusCode((TunsafeBackend::StatusCode)it->wparam); break;
      case Id_OnClearLog:         delegate->OnClearLog(); break;
      case Id_OnGraphAvailable:   delegate->OnGraphAvailable(); break;
      case Id_OnConfigurationProtocolReply: delegate->OnConfigurationProtocolReply(it->wparam, std::move(*(std::string*)it->lparam)); break;
    }
    FreeEntry(&*it);
  } 
  processing_entry_.clear();
}

void TunsafeBackendDelegateThreaded::AddEntry(Which which, intptr_t lparam, uint32 wparam) {
  mutex_.Acquire();
  bool was_empty = incoming_entry_.empty();
  incoming_entry_.emplace_back(which, wparam, lparam);
  mutex_.Release();
  if (was_empty)
    callback_();
}

void TunsafeBackendDelegateThreaded::OnGetStats(const WgProcessorStats &stats) {
  AddEntry(Id_OnGetStats, (intptr_t)memdup(&stats, sizeof(stats)));
}

void TunsafeBackendDelegateThreaded::OnGraphAvailable() {
  AddEntry(Id_OnGraphAvailable);
}

void TunsafeBackendDelegateThreaded::OnStateChanged() {
  AddEntry(Id_OnStateChanged);
}

void TunsafeBackendDelegateThreaded::OnLogLine(const char **s) {
  const char *ss = *s;
  *s = NULL;
  AddEntry(Id_OnLogLine, (intptr_t)ss);
}

void TunsafeBackendDelegateThreaded::OnStatusCode(TunsafeBackend::StatusCode status) {
  AddEntry(Id_OnStatusCode, 0, status);
}

void TunsafeBackendDelegateThreaded::OnClearLog() {
  AddEntry(Id_OnClearLog);
}

void TunsafeBackendDelegateThreaded::OnConfigurationProtocolReply(uint32 ident, const std::string &&reply) {
  AddEntry(Id_OnConfigurationProtocolReply, (intptr_t)new std::string(std::move(reply)), ident);
}

TunsafeBackend::Delegate::~Delegate() {
}

TunsafeBackend *CreateNativeTunsafeBackend(TunsafeBackend::Delegate *delegate) {
  return new TunsafeBackendWin32(delegate);
}
 
TunsafeBackend::Delegate *CreateTunsafeBackendDelegateThreaded(TunsafeBackend::Delegate *delegate, const std::function<void(void)> &callback) {
  return new TunsafeBackendDelegateThreaded(delegate, callback);
}

///////////////////////////////////////////////////

void StatsCollector::Init() {
  Accumulator *acc = &accum_[0][0];
  static const int kAccMax[TIMEVALS] = {5, 6, 10, 0};

  // Configure all stats channels
  for (uint32 channel = 0; channel != CHANNELS; channel++) {
    for (uint32 timeval = 0; timeval != TIMEVALS; timeval++, acc++) {
      acc->acc = 0;
      acc->dirty = false;
      acc->acc_count = 0;
      acc->acc_max = kAccMax[timeval];
      acc->data.size = 120;
      acc->data.data = (float*)calloc(sizeof(float), acc->data.size);
      acc->data.shift = 0;
    }
  }
}

void StatsCollector::AddToGraphDataSource(StatsCollector::TimeSeries *ts, float value) {
  ts->data[ts->shift] = value;
  if (++ts->shift == ts->size)
    ts->shift = 0;
}

void StatsCollector::AddToAccumulators(StatsCollector::Accumulator *acc, float rval) {
  for (;;) {
    AddToGraphDataSource(&acc->data, rval);
    acc->dirty = true;
    acc->acc += rval;
    if (acc->acc_max == 0 || ++acc->acc_count < acc->acc_max)
      break;
    rval = acc->acc / (float)acc->acc_count;
    acc->acc_count = 0;
    acc->acc = 0.0f;
    acc++;
  }
}

void StatsCollector::AddSamples(float data[CHANNELS]) {
  for (size_t i = 0; i < CHANNELS; i++)
    AddToAccumulators(&accum_[i][0], data[i]);
}


TunAdaptersInUse::TunAdaptersInUse() {
  num_inuse_ = 0;
}

bool TunAdaptersInUse::Acquire(const char guid[ADAPTER_GUID_SIZE], void *context) {
  size_t len = strlen(guid);
  if (len >= ADAPTER_GUID_SIZE)
    return false;
  ScopedLock scoped_lock(&mutex_);
  Entry *e = entry_;
  for (uint32 n = num_inuse_; ; n--, e++) {
    if (n == 0) {
      if (num_inuse_ == kMaxAdaptersInUse)
        return false;
      num_inuse_++;
      e->context = context;
      e->count = 0;
      memcpy(e->guid, guid, len + 1);
      return true;
    }
    if (!strcmp(e->guid, guid)) {
      if (e->context != context)
        return false;
      e->count++;
      return true;
    }
  }
}

void TunAdaptersInUse::Release(void *context) {
  ScopedLock scoped_lock(&mutex_);
  Entry *e = entry_;
  for (uint32 n = num_inuse_; n; n--, e++) {
    if (e->context == context) {
      if (e->count-- == 0)
        *e = entry_[num_inuse_-- - 1];
      break;
    }
  }
}

void *TunAdaptersInUse::LookupContextFromGuid(const char guid[ADAPTER_GUID_SIZE]) {
  ScopedLock scoped_lock(&mutex_);
  Entry *e = entry_;
  for (uint32 n = num_inuse_; n; n--, e++) {
    if (!strcmp(e->guid, guid))
      return e->context;
  }
  return NULL;
}

char *TunAdaptersInUse::GetAllGuid() {
  ScopedLock scoped_lock(&mutex_);
  char *rv = (char*)malloc(ADAPTER_GUID_SIZE * num_inuse_ + 1), *p = rv;
  if (rv) {
    Entry *e = entry_;
    for (uint32 n = num_inuse_; n; n--, e++) {
      size_t len = strlen(e->guid);
      p[len] = '\n';
      memcpy(p, e->guid, len);
      p += len + 1;
    }
    *p = 0;
  }
  return rv;
}

static TunAdaptersInUse g_tun_adapters_in_use;
TunAdaptersInUse *TunAdaptersInUse::GetInstance() {
  return &g_tun_adapters_in_use;
}

TunsafeBackend *TunsafeBackend::FindBackendByTunGuid(const char *guid) {
  return (TunsafeBackend*)TunAdaptersInUse::GetInstance()->LookupContextFromGuid(guid);
}

char *TunsafeBackend::GetAllGuid() {
  return TunAdaptersInUse::GetInstance()->GetAllGuid();
}
