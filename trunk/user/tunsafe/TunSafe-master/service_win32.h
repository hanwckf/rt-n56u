// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once

#include "service_win32_api.h"
#include "service_pipe_win32.h"
#include "network_win32_api.h"
#include "tunsafe_threading.h"

// Takes care of multiple TunsafeServiceBackend
class TunsafeServiceManager : public PipeManager::Delegate {
  friend class TunsafeServiceBackend;
  friend class TunsafeServiceServer;
public:
  TunsafeServiceManager();
  virtual ~TunsafeServiceManager();
  
  // -- from PipeManager::Delegate
  virtual void HandleNotify() override;
  virtual PipeConnection::Delegate *HandleNewConnection(PipeConnection *connection) override;

  // Called by the service control code to bring the service up or down
  unsigned OnStart(int argc, wchar_t **argv);
  void OnStop();
  void OnShutdown();

  TunsafeServiceBackend *main_backend() { return main_backend_; }
  
  TunsafeServiceBackend *CreateBackend(const char *guid);
  void DestroyBackend(TunsafeServiceBackend *backend);

  bool SwitchInterface(TunsafeServiceServer *server, const char *interfac, bool want_create);

private:
  // Points at the Tunsafe hklm reg key
  HKEY hkey_;
  uint32 server_unique_id_;

  PipeManager pipe_manager_;

  TunsafeServiceBackend *main_backend_;
  std::vector<TunsafeServiceBackend *> backends_;
};

// One of these exist for each TunsafeBackend
class TunsafeServiceBackend : public TunsafeBackend::Delegate {
  friend class TunsafeServiceServer;
public:
  explicit TunsafeServiceBackend(TunsafeServiceManager *manager);
  virtual ~TunsafeServiceBackend();

  // -- from TunsafeBackend::Delegate
  virtual void OnGetStats(const WgProcessorStats &stats) override;
  virtual void OnClearLog() override;
  virtual void OnLogLine(const char **s) override;
  virtual void OnStateChanged() override;
  virtual void OnStatusCode(TunsafeBackend::StatusCode status) override;
  virtual void OnGraphAvailable() override;
  virtual void OnConfigurationProtocolReply(uint32 ident, const std::string &&reply) override;

  TunsafeBackend *backend() { return backend_; }
  TunsafeBackend::Delegate *delegate() { return thread_delegate_; }

  void Start(const char *filename);
  void RememberLastUsedConfigFile(const char *filename);

  void Stop();

  // Trigger backend stats updates whenever a connected pipe client needs it
  void UpdateRequestStats();

  // Called by TunsafeServiceManager::HandleNotify to process events
  // on each backend.
  void HandleNotify();

  // Send a state update to all connected pipes unless filter is set, then it
  // sends only to that.
  void SendStateUpdate(TunsafeServiceServer *filter);

  // Called whenever a pipe server disconnects
  void RemovePipeServer(TunsafeServiceServer *pipe_server);

  // Called to register a pipe server with this backend
  void AddPipeServer(TunsafeServiceServer *pipe_server);
private:
  // toggled every time a token submit is processed
  uint8 token_request_flag_;

  // Points at the service manager
  TunsafeServiceManager *manager_;

  // Points at the actual TunsafeBackend
  TunsafeBackend *backend_;

  // Points at all |TunsafeServiceServer| currently associated with this
  // backend.
  std::vector<TunsafeServiceServer*> pipe_servers_;
  
  // Points at the thing that transmits TunsafeBackend events to
  // the main thread
  TunsafeBackend::Delegate *thread_delegate_;
  
  // The config filename that is loaded
  std::string current_filename_;

  // Positions into |historical_log_lines_|
  uint32 historical_log_lines_pos_;
  uint32 historical_log_lines_count_;

  enum { LOGLINE_COUNT = 256 };
  char *historical_log_lines_[LOGLINE_COUNT];
};

// The server side of the client<->server pipe connection
class TunsafeServiceServer : public PipeConnection::Delegate {
 
public:
  TunsafeServiceServer(PipeConnection *pipe, TunsafeServiceBackend *backend, uint32 unique_id);
  virtual ~TunsafeServiceServer();

  void WritePacket(int type, const uint8 *data, size_t data_size);

  // -- from PipeConnection::Delegate
  virtual bool HandleMessage(int type, uint8 *data, size_t size) override;
  virtual void HandleDisconnect() override;

  // Called by TunsafeServiceBackend to push a graph to the client
  void OnGraphAvailable();

  // Called by TunsafeServiceBackend to push more log lines to the client
  void SendQueuedLogLines();

  bool want_stats() const { return want_stats_; }
  bool want_state_updates() const { return want_state_updates_; }
  uint32 unique_id() const { return unique_id_; }
  TunsafeServiceBackend *service_backend() { return service_backend_; }
  void set_service_backend(TunsafeServiceBackend *sb) { service_backend_ = sb; }
private:
  bool AuthenticateUser();

  // Whether the client wants state updates
  bool want_state_updates_;

  // Whether the client has authenticated
  bool did_authenticate_user_;

  // Whether we want stats
  bool want_stats_;

  // Whether the currently connected user wants a graph
  uint32 want_graph_type_;

  // The last log line sent to the currently connected user
  uint32 last_line_sent_;

  uint32 unique_id_;

  // The pipe used to communicate
  PipeConnection *connection_;

  // The backend we're currently associated with
  TunsafeServiceBackend *service_backend_;
};


struct ServiceState {
  uint8 is_started : 1;
  uint8 token_request_flag : 1; // toggled each time token_request changes
  uint8 reserved1;
  uint16 internet_block_state;
  uint8 reserved[20 + 64];
  uint32 token_request;
  uint32 ipv4_ip;
  uint8 public_key[32];
};

STATIC_ASSERT(sizeof(ServiceState) == 128, ServiceState_wrong_size);

class TunsafeServiceClient : public TunsafeBackend, public PipeConnection::Delegate, public PipeManager::Delegate {
public:
  TunsafeServiceClient(TunsafeBackend::Delegate *delegate);
  virtual ~TunsafeServiceClient();

  // -- from TunsafeBackend
  virtual bool Configure();
  virtual void Teardown();
  virtual bool SetTunAdapterName(const char *name);
  virtual void Start(const char *config_file);
  virtual void Stop();
  virtual void RequestStats(bool enable);
  virtual void ResetStats();
  virtual InternetBlockState GetInternetBlockState();
  virtual void SetInternetBlockState(InternetBlockState s);
  virtual std::string GetConfigFileName();
  virtual void SetServiceStartupFlags(uint32 flags);
  virtual LinearizedGraph *GetGraph(int type);
  virtual void SendConfigurationProtocolPacket(uint32 identifier, const std::string &&message) override;
  virtual uint32 GetTokenRequest() override;
  virtual void SubmitToken(const std::string &&token) override;

  // -- from PipeConnection::Delegate
  virtual bool HandleMessage(int type, uint8 *data, size_t size) override;
  virtual void HandleDisconnect() override;

  // -- from PipeManager::Delegate
  virtual void HandleNotify() override;
  virtual PipeConnection::Delegate *HandleNewConnection(PipeConnection *connection) override;


protected:
  TunsafeBackend::Delegate *delegate_;
  uint8 want_stats_;
  uint8 token_request_flag_;
  bool got_state_from_control_;
  ServiceState service_state_;
  std::string config_file_;
  PipeManager pipe_manager_;
  PipeConnection *connection_;
  LinearizedGraph *cached_graph_;
  uint32 last_graph_type_;
  Mutex mutex_;
};

