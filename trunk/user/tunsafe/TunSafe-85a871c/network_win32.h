// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once

#include "netapi.h"
#include "network_win32_api.h"
#include "network_win32_dnsblock.h"
#include "wireguard_config.h"
#include "tunsafe_threading.h"
#include "tunsafe_dnsresolve.h"
#include "network_common.h"
#include "network_win32_tcp.h"
#include "tunsafe_wg_plugin.h"

enum {
  ADAPTER_GUID_SIZE = 40,
};

class WireguardProcessor;
class TunsafeBackendWin32;
class TunsafeRunner;
class DnsBlocker;

struct PacketProcessorTunCb : QueuedItemCallback {
  virtual void OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) override;
  virtual void OnQueuedItemDelete(QueuedItem *ow) override;
};

struct PacketProcessorUdpCb : QueuedItemCallback {
  virtual void OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) override;
  virtual void OnQueuedItemDelete(QueuedItem *ow) override;
};

struct PacketProcessorDeobfuscateUdpCb : PacketProcessorUdpCb {
  virtual void OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) override;
};

class PacketProcessor {
public:
  explicit PacketProcessor();
  ~PacketProcessor();

  void Reset();

  int Run(WireguardProcessor *wg, TunsafeRunner *runner);
  void PostPackets(Packet *first, Packet **end, int count);
  void ForcePost(QueuedItem *item);
  void PostExit(int exit_code);
  void EnableDeobfuscation() {
    udp_cb_maybe_deobfuscate_ = &udp_cb_deobfuscate_;
  }

  const uint32 *posted_exit_code() { return &exit_code_; }

  // Handler for tun packets
  QueuedItemCallback *tun_queue() { return &tun_cb_; }
   
  // Handler for udp packets
  QueuedItemCallback *udp_queue() { return udp_cb_maybe_deobfuscate_; }

  // Incoming queue for tcp packets that do not use deobfuscation
  QueuedItemCallback *tcp_queue() { return &udp_cb_; }

  struct QueueContext {
    WireguardProcessor *wg;
    TunsafeRunner *runner;
    bool overload;
  };

private:
  static void CALLBACK ThreadPoolTimerCallback(PTP_CALLBACK_INSTANCE iTimerInstance, PVOID pContext, PTP_TIMER);
  QueuedItem *first_;
  QueuedItem **last_ptr_;
  uint32 packets_in_queue_;
  uint32 need_notify_;
  Mutex mutex_;
  HANDLE event_;

  uint32 exit_code_;
  bool timer_interrupt_;

  QueuedItemCallback *udp_cb_maybe_deobfuscate_;

  PacketProcessorTunCb tun_cb_;
  PacketProcessorUdpCb udp_cb_;
  PacketProcessorDeobfuscateUdpCb udp_cb_deobfuscate_;
};

class NetworkWin32;
class PacketAllocPool;

// Encapsulates a UDP socket pair (ipv4 / ipv6), optionally listening for incoming packets
// on a specific port.
class UdpSocketWin32 : public QueuedItemCallback {
public:
  explicit UdpSocketWin32(NetworkWin32 *network_win32);
  ~UdpSocketWin32();

  void SetPacketHandler(PacketProcessor *packet_handler) { packet_handler_ = packet_handler; }

  bool Configure(int listen_on_port);
  inline void WriteUdpPacket(Packet *packet);

  void DoIO();
  void CancelAllIO();
  bool HasOutstandingIO();

  enum {
    kConcurrentReadUdp = 16,
    kConcurrentWriteUdp = 16
  };

private:
  void DoMoreReads();
  void DoMoreWrites();
  void ProcessPackets();

  // From OverlappedCallbacks
  virtual void OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) override;
  virtual void OnQueuedItemDelete(QueuedItem *ow) override;
  
  NetworkWin32 *network_;

  // All packets queued for writing. Locked by |mutex_|
  // Both ipv6 and ipv4 are supported
  Packet *wqueue_, **wqueue_end_;

  // Protects wqueue
  Mutex mutex_;

  // This is where packets end up
  PacketProcessor *packet_handler_;

  // The two socket handles, since we support both ipv4 and ipv6
  SOCKET socket_, socket_ipv6_;

  enum { IPV4, IPV6 };
  int max_read_ipv6_;
  int num_reads_[2];
  int num_writes_;
  Packet *pending_writes_;
    
  Packet *finished_reads_, **finished_reads_end_;
  int finished_reads_count_;

  uint32 qsize1_;
  uint8 align[64-4];
  uint32 qsize2_;
};

// Holds the thread for network communications
class NetworkWin32 {
  friend class UdpSocketWin32;
  friend class TcpSocketWin32;
  friend class TcpSocketQueue;
public:
  explicit NetworkWin32();
  ~NetworkWin32();
  
  void StartThread();
  void StopThread();

  UdpSocketWin32 &udp() { return udp_socket_; }
  SimplePacketPool &packet_pool() { return packet_pool_; }
  void WakeUp();
  void PostQueuedItem(QueuedItem *item);
private:
  void ThreadMain();
  static DWORD WINAPI NetworkThread(void *x);

  bool HasOutstandingIO();

  // The network thread handle
  HANDLE thread_;

  // Whether we're exiting the thread
  bool exit_thread_;

  // The handle to the completion port
  HANDLE completion_port_handle_;

  // Right now there's always one udp socket only
  UdpSocketWin32 udp_socket_;

  // A linked list of all tcp sockets
  TcpSocketWin32 *tcp_socket_;

  SimplePacketPool packet_pool_;
};

class TunWin32Adapter {
public:
  TunWin32Adapter(DnsBlocker *dns_blocker, const char guid[ADAPTER_GUID_SIZE]);
  ~TunWin32Adapter();

  bool OpenAdapter(TunsafeRunner *backend, DWORD open_flags);
  bool ConfigureAdapter(const TunInterface::TunConfig &&config, TunInterface::TunConfigOut *out);
  void CloseAdapter(bool is_restart);

  HANDLE handle() { return handle_; }

  void DisassociateDnsBlocker() { dns_blocker_ = NULL; }

private:
  bool RunPrePostCommand(const std::vector<std::string> &vec);

  HANDLE handle_;
  DnsBlocker *dns_blocker_;

  std::vector<MIB_IPFORWARD_ROW2> routes_to_undo_;
  uint8 mac_adress_[6];
  bool has_dns6_setting_;
  int mtu_;
  
  int old_ipv4_metric_, old_ipv6_metric_;

  std::vector<WgCidrAddr> old_ipv6_address_;

  NET_LUID interface_luid_;

  void *backend_;

  std::vector<std::string> pre_down_, post_down_;
  char guid_[ADAPTER_GUID_SIZE];
};

// Implementation of TUN interface handling using IO Completion Ports
class TunWin32Iocp : public TunInterface {
public:
  explicit TunWin32Iocp(DnsBlocker *blocker, TunsafeRunner *backend);
  ~TunWin32Iocp();

  void SetPacketHandler(PacketProcessor *packet_handler) { packet_handler_ = packet_handler; }

  void StartThread();
  void StopThread();

  // -- from TunInterface
  virtual bool Configure(const TunConfig &&config, TunConfigOut *out) override;
  virtual void WriteTunPacket(Packet *packet) override;

  TunWin32Adapter &adapter() { return adapter_; }

private:
  void CloseTun(bool is_restart);
  void ThreadMain();
  static DWORD WINAPI TunThread(void *x);

  PacketProcessor *packet_handler_;
  HANDLE completion_port_handle_;
  HANDLE thread_;

  Mutex mutex_;

  bool exit_thread_;
  bool did_show_tun_queue_warning_;

  int wqueue_size_;

  // All packets queued for writing
  Packet *wqueue_, **wqueue_end_;

  TunsafeRunner *runner_;
  TunWin32Adapter adapter_;
};

// This class is the actual TunSafe thing and runs inside of a thread.
class TunsafeRunner : public UdpInterface, public ProcessorDelegate, public PluginDelegate, public QueuedItemCallback {
  friend class TunsafeBackendWin32;
public:
  TunsafeRunner(TunsafeBackendWin32 *backend);
  ~TunsafeRunner();

  void SetConfigFile(const char *file, bool is_text_format);

  TunsafeBackendWin32 *backend() { return backend_; }

  // -- from UdpInterface
  virtual bool Configure(int listen_port_udp, int listen_port_tcp) override;
  virtual void WriteUdpPacket(Packet *packet) override;

  virtual void OnConnected() override;
  virtual void OnConnectionRetry(uint32 attempts) override;

  // -- from PluginDelegate
  virtual void OnRequestToken(WgPeer *peer, uint32 type) override;

  bool Start();

  // Called by the tun thing if tun stops working and a reset is needed.
  void PostTunRestart();

  uint32 exit_code() { return *packet_processor_.posted_exit_code(); }

  TunsafePlugin *plugin() { return plugin_; }

  void CollectStats();
  
private:
  // From OverlappedCallbacks
  virtual void OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) override;
  virtual void OnQueuedItemDelete(QueuedItem *ow) override;

  TunsafeBackendWin32 *backend_;
  TunsafePlugin *plugin_;
  bool config_file_is_text_format_;
  std::string config_file_;
  TunWin32Iocp tun_;
  NetworkWin32 net_;
  TcpSocketQueue tcp_socket_queue_;
  WireguardProcessor wg_proc_;
  PacketProcessor packet_processor_;
};


class TunsafeBackendWin32 : public TunsafeBackend {
  friend class TunsafeRunner;
  friend class PacketProcessor;
  friend class TunWin32Iocp;
  friend class TunWin32Overlapped;
  friend class TunWin32Adapter;
  friend struct ConfigQueueItem;
public:
  TunsafeBackendWin32(Delegate *delegate);
  ~TunsafeBackendWin32();

  // -- from TunsafeBackend
  virtual bool Configure() override;
  virtual void Teardown() override;
  virtual bool SetTunAdapterName(const char *name) override;
  virtual void Start(const char *config_file) override;
  virtual void Stop() override;
  virtual void RequestStats(bool enable) override;
  virtual void ResetStats() override;
  virtual InternetBlockState GetInternetBlockState() override;
  virtual void SetInternetBlockState(InternetBlockState s) override;
  virtual void SetServiceStartupFlags(uint32 flags) override;
  virtual LinearizedGraph *GetGraph(int type) override;
  virtual std::string GetConfigFileName() override;
  virtual void SendConfigurationProtocolPacket(uint32 identifier, const std::string &&message) override;
  virtual uint32 GetTokenRequest() override;
  virtual void SubmitToken(const std::string &&message) override;
  
  void OnRequestToken(WgPeer *peer, uint32 type);

  void SetPublicKey(const uint8 key[32]);

  StatusCode status() { return status_; }
  void SetStatus(StatusCode status);

  void CollectStats();

private:

  enum {
    MODE_NONE = 0,
    MODE_EXIT = 1,
    MODE_RESTART = 2,
  };

  void StopInner(bool is_restart);
  static DWORD WINAPI WorkerThread(void *x);
  void PushStats();

  TunsafeRunner *runner_;
  HANDLE worker_thread_;
  bool want_periodic_stats_;

  Delegate *delegate_;

  std::atomic<uint32> token_request_;

  DnsBlocker dns_blocker_;
  DnsResolver dns_resolver_;

  uint32 last_tun_adapter_failed_;
  StatsCollector stats_collector_;

  Mutex stats_mutex_;
  WgProcessorStats stats_;

  char guid_[ADAPTER_GUID_SIZE];
};

// This class ensures that all callbacks get rescheduled to another thread
class TunsafeBackendDelegateThreaded : public TunsafeBackend::Delegate {
public:
  TunsafeBackendDelegateThreaded(TunsafeBackend::Delegate *delegate, const std::function<void(void)> &callback);
  ~TunsafeBackendDelegateThreaded();

private:
  virtual void OnGetStats(const WgProcessorStats &stats) override;
  virtual void OnGraphAvailable() override;
  virtual void OnStateChanged() override;
  virtual void OnClearLog() override;
  virtual void OnLogLine(const char **s) override;
  virtual void OnStatusCode(TunsafeBackend::StatusCode status) override;
  virtual void OnConfigurationProtocolReply(uint32 ident, const std::string &&reply) override;
  virtual void DoWork() override;

  enum Which {
    Id_OnGetStats,
    Id_OnStateChanged,
    Id_OnClearLog,
    Id_OnLogLine,
    Id_OnUpdateUI,
    Id_OnStatusCode,
    Id_OnGraphAvailable,
    Id_OnConfigurationProtocolReply,
  };

  void AddEntry(Which which, intptr_t lparam = 0, uint32 wparam = 0);

  TunsafeBackend::Delegate *delegate_;
  std::function<void(void)> callback_;

  struct Entry {
    uint8 which;
    uint32 wparam;
    intptr_t lparam;
    Entry(uint8 which, uint32 wparam, intptr_t lparam) : which(which), wparam(wparam), lparam(lparam) {}
  };

  static void FreeEntry(Entry *e);

  Mutex mutex_;
  std::vector<Entry> incoming_entry_;
  std::vector<Entry> processing_entry_;
};

// For each adapter, remembers whether the adapter is in use
class TunAdaptersInUse {
public:
  TunAdaptersInUse();

  // attempt to acquire the adapter, so it can't be acquired by anyone else
  bool Acquire(const char guid[ADAPTER_GUID_SIZE], void *context);

  // mark as free
  void Release(void *context);

  // Lookup a context from a guid
  void *LookupContextFromGuid(const char guid[ADAPTER_GUID_SIZE]);

  // Lookup a guid from a context
  bool LookupGuidFromContext(void *context, char guid[ADAPTER_GUID_SIZE]);

  char *GetAllGuid();

  static TunAdaptersInUse *GetInstance();

private:
  enum {
    kMaxAdaptersInUse = 16,
  };
  struct Entry {
    char guid[ADAPTER_GUID_SIZE];
    void *context;
    int count;
  };
  Mutex mutex_;
  uint8 num_inuse_;
  Entry entry_[kMaxAdaptersInUse];
};

static inline void ClearOverlapped(OVERLAPPED *o) {
  memset(o, 0, sizeof(*o));
}

