// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "service_win32.h"
#include <strsafe.h>
#include "util.h"
#include "network_win32_api.h"
#include <algorithm>
#include <string>
#include <assert.h>
#include "util_win32.h"
#include "service_win32_constants.h"

static SERVICE_STATUS_HANDLE m_statusHandle;
static TunsafeServiceManager *g_service;

#define SERVICE_DEBUGGING 0

#define SERVICE_NAME             L"TunSafeService"
#define SERVICE_NAMEA            "TunSafeService"
#define SERVICE_START_TYPE       SERVICE_AUTO_START
#define SERVICE_DEPENDENCIES     L"tap0901\0dhcp\0"
#define SERVICE_ACCOUNT          NULL
#define SERVICE_PASSWORD         NULL

struct ServiceHandles {
  SC_HANDLE manager;
  SC_HANDLE service;

  ServiceHandles() : manager(NULL), service(NULL) {}
  ~ServiceHandles() {
    if (manager)
      CloseServiceHandle(manager);
    if (service)
      CloseServiceHandle(service);
  }

  bool Open(PWSTR pszServiceName, DWORD sc_rights, DWORD service_rights);
  bool StopService();
  bool StartService();
};

static DWORD InstallService(PWSTR pszServiceName,
                    PWSTR pszDisplayName,
                    DWORD dwStartType,
                    PWSTR pszDependencies,
                    PWSTR pszAccount,
                    PWSTR pszPassword) {
  wchar_t szPath[MAX_PATH + 32];
  ServiceHandles handles;
  DWORD res;

  szPath[0] = '"';
  if (GetModuleFileNameW(NULL, szPath + 1, MAX_PATH) == 0) {
    res = GetLastError();
    goto Cleanup;
  }
  size_t len = wcslen(szPath);
  memcpy(szPath + len, L"\" --service", 12 * sizeof(wchar_t));

  // Open the local default service control manager database
  handles.manager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT |
                               SC_MANAGER_CREATE_SERVICE);
  if (handles.manager == NULL) {
    res = GetLastError();
    goto Cleanup;
  }

  // Install the service into SCM by calling CreateService
  handles.service = CreateServiceW(
    handles.manager,                   // SCManager database
    pszServiceName,                 // Name of service
    pszDisplayName,                 // Name to display
    SERVICE_QUERY_STATUS,           // Desired access
    SERVICE_WIN32_OWN_PROCESS,      // Service type
    dwStartType,                    // Service start type
    SERVICE_ERROR_NORMAL,           // Error control type
    szPath,                         // Service's binary
    NULL,                           // No load ordering group
    NULL,                           // No tag identifier
    pszDependencies,                // Dependencies
    pszAccount,                     // Service running account
    pszPassword                     // Password of the account
  );
  if (handles.service == NULL) {
    res = GetLastError();
    goto Cleanup;
  }
  {
    SERVICE_DESCRIPTIONA desc;
    desc.lpDescription = "TunSafe uses this service to connect to a VPN server in the background.";
    ChangeServiceConfig2A(handles.service, SERVICE_CONFIG_DESCRIPTION, &desc);
  }
  res = 0;
Cleanup:
  if (res && res != ERROR_SERVICE_EXISTS)
    RERROR("TunSafe service installation failed: %d", res);
  return res;
}

bool ServiceHandles::Open(PWSTR pszServiceName, DWORD sc_rights, DWORD service_rights) {
  manager = OpenSCManagerW(NULL, NULL, sc_rights);
  if (manager == NULL)
    return false;
  service = OpenServiceW(manager, pszServiceName, service_rights);
  return (service != NULL);
}

bool ServiceHandles::StopService() {
  SERVICE_STATUS ssSvcStatus = {};
  // Try to stop the service
  if (ControlService(service, SERVICE_CONTROL_STOP, &ssSvcStatus)) {
    Sleep(100);
    while (QueryServiceStatus(service, &ssSvcStatus)) {
      if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
        Sleep(100);
      } else {
        break;
      }
    }
  }
  return (ssSvcStatus.dwCurrentState == SERVICE_STOPPED);
}

static wchar_t *GetUsernameOfCurrentUser(bool use_thread_token) {
  HANDLE thread_token = NULL;
  wchar_t *result = NULL;
  DWORD len;
  PTOKEN_USER token_user = NULL;
  DWORD domain_len;
  WCHAR username[256], domain[256];
  SID_NAME_USE sid_type;

  if (use_thread_token) {
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &thread_token))
      goto getout;
  } else {
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &thread_token))
      goto getout;

  }
  len = 0;
  token_user = NULL;
  while (!GetTokenInformation(thread_token, TokenUser, token_user, len, &len)) {
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
      goto getout;
    token_user = (PTOKEN_USER)realloc(token_user, len);
    if (!token_user)
      goto getout;
  }
  if (!IsValidSid(token_user->User.Sid))
    goto getout;
  domain_len = len = 256;
  if (!LookupAccountSidW(NULL, token_user->User.Sid, username, &len, domain, &domain_len, &sid_type))
    goto getout;

  size_t alen = wcslen(username);
  size_t blen = wcslen(domain);

  result = (wchar_t*)malloc((alen + blen + 2) * sizeof(wchar_t));
  if (result) {
    result[alen] = '@';
    memcpy(result, username, alen * sizeof(wchar_t));
    memcpy(result + alen + 1, domain, (blen + 1) * sizeof(wchar_t));
  }
getout:
  free(token_user);
  CloseHandle(thread_token);
  return result;
}

static wchar_t *RegReadStrW(HKEY hkey, const wchar_t *key, const wchar_t *def) {
  wchar_t buf[1024];
  DWORD n = sizeof(buf) - 2;
  DWORD type = 0;
  if (RegQueryValueExW(hkey, key, NULL, &type, (BYTE*)buf, &n) != ERROR_SUCCESS || type != REG_SZ)
    return def ? _wcsdup(def) : NULL;
  n >>= 1;
  if (n && buf[n - 1] == 0)
    n--;
  buf[n] = 0;
  return _wcsdup(buf);
}

static DWORD GetNonTransientServiceStatus(SC_HANDLE service) {
  SERVICE_STATUS ssSvcStatus = {};
  int delay = 100;
  for(;;) {
    if (!QueryServiceStatus(service, &ssSvcStatus))
      return 0;

    if (--delay == 0 || 
        ssSvcStatus.dwCurrentState != SERVICE_START_PENDING &&
        ssSvcStatus.dwCurrentState != SERVICE_STOP_PENDING)
      return ssSvcStatus.dwCurrentState;
    Sleep(100);
    delay--;
  }
}

bool ServiceHandles::StartService() {
  DWORD state = GetNonTransientServiceStatus(service);
  if (state == 0 || state == SERVICE_RUNNING)
    return false; // service already running, no need to start
  if (!::StartService(service, 0, NULL)) {
//    if (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
//      return false;
    return false;
  }
  return GetNonTransientServiceStatus(service) == SERVICE_RUNNING;
}

static bool StartTunsafeService() {
  ServiceHandles handles;

  if (!handles.Open(SERVICE_NAME, SC_MANAGER_CONNECT, SERVICE_START | SERVICE_QUERY_STATUS)) 
    return false;
  return handles.StartService();
}

bool IsTunsafeServiceRunning() {
  ServiceHandles handles;
#if SERVICE_DEBUGGING
  return true;
#endif
  if (!handles.Open(SERVICE_NAME, SC_MANAGER_CONNECT, SERVICE_QUERY_STATUS))
    return false;
  return GetNonTransientServiceStatus(handles.service) == SERVICE_RUNNING;
}

void StopTunsafeService() {
  ServiceHandles handles;
  if (!handles.Open(SERVICE_NAME, SC_MANAGER_CONNECT,
                    SERVICE_STOP | SERVICE_QUERY_STATUS))
    goto Cleanup;
  handles.StopService();
Cleanup:
  return;
}

static void SetTunsafeUserNameInRegistry() {
  wchar_t *user = GetUsernameOfCurrentUser(false);
  if (!user) {
    RERROR("Unable to get current username");
    return;
  }
  HKEY hkey = NULL;
  RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\TunSafe", NULL, NULL, 0, KEY_ALL_ACCESS, NULL, &hkey, NULL);
  if (!hkey) {
    RERROR("Unable to open registry key");
    return;
  }
  if (RegSetValueExW(hkey, L"AllowedUsername", NULL, REG_SZ, (BYTE*)user, (DWORD)(wcslen(user) + 1) * 2) != ERROR_SUCCESS) {
    RERROR("Unable to set registry key");
  }
  RegCloseKey(hkey);
}

void InstallTunSafeWindowsService() {
  InstallService(SERVICE_NAME, L"TunSafe Service", SERVICE_START_TYPE,
                 SERVICE_DEPENDENCIES, SERVICE_ACCOUNT, SERVICE_PASSWORD);
  StartTunsafeService();
  SetTunsafeUserNameInRegistry();
}

bool UninstallTunSafeWindowsService() {
  ServiceHandles handles;

  if (!handles.Open(SERVICE_NAME, SC_MANAGER_CONNECT,
                    SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE))
    goto Cleanup;

  handles.StopService();

  if (!DeleteService(handles.service))
    goto Cleanup;
  return true;
Cleanup:
  return false;
}

bool IsTunSafeServiceInstalled() {
  ServiceHandles handles;
  return handles.Open(SERVICE_NAME, SC_MANAGER_CONNECT, SERVICE_QUERY_STATUS);
}

static void WriteServiceLog(const char *pszFunction, WORD dwError) {
  char szMessage[260];
  snprintf(szMessage, ARRAYSIZE(szMessage), "%s failed w/err 0x%08lx", pszFunction, dwError);
  HANDLE hEventSource = NULL;
  LPCSTR lpszStrings[2] = {NULL, NULL};
  hEventSource = RegisterEventSourceW(NULL, SERVICE_NAME);
  if (hEventSource) {
    lpszStrings[0] = SERVICE_NAMEA;
    lpszStrings[1] = szMessage;

    ReportEventA(hEventSource,  // Event log handle
                dwError,                 // Event type
                0,                     // Event category
                0,                     // Event identifier
                NULL,                  // No security identifier
                2,                     // Size of lpszStrings array
                0,                     // No binary data
                lpszStrings,           // Array of strings
                NULL                   // No binary data
    );
    DeregisterEventSource(hEventSource);
  }
}

static void SetServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode = 0,
                             DWORD dwWaitHint = 0) {
  static DWORD dwCheckPoint = 1;

  SERVICE_STATUS m_status;
  m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  m_status.dwServiceSpecificExitCode = 0;
  m_status.dwCurrentState = dwCurrentState;
  m_status.dwWin32ExitCode = dwWin32ExitCode;
  m_status.dwWaitHint = dwWaitHint;
  m_status.dwCheckPoint = ((dwCurrentState == SERVICE_RUNNING) ||
                           (dwCurrentState == SERVICE_STOPPED)) ? 0 : dwCheckPoint++;
  // Report the status of the service to the SCM.
  ::SetServiceStatus(m_statusHandle, &m_status);
}

static void OnServiceStart(DWORD dwArgc, PWSTR *pszArgv) {
  WriteServiceLog("Service Starting", EVENTLOG_INFORMATION_TYPE);
  SetServiceStatus(SERVICE_START_PENDING);
  DWORD rv = g_service->OnStart(dwArgc, pszArgv);
  if (rv) {
    SetServiceStatus(SERVICE_STOPPED, rv);
  } else {
    SetServiceStatus(SERVICE_RUNNING);
  }
}

static void OnServiceStop() {
  WriteServiceLog("Service Stopping", EVENTLOG_INFORMATION_TYPE);
  SetServiceStatus(SERVICE_STOP_PENDING);
  g_service->OnStop();
  SetServiceStatus(SERVICE_STOPPED);
}

static void OnServiceShutdown() {
  g_service->OnShutdown();
  SetServiceStatus(SERVICE_STOPPED);
}

static void WINAPI ServiceCtrlHandler(DWORD dwCtrl) {
  switch (dwCtrl) {
  case SERVICE_CONTROL_STOP: OnServiceStop(); break;
//  case SERVICE_CONTROL_PAUSE: OnServicePause(); break;
//  case SERVICE_CONTROL_CONTINUE: OnServiceContinue(); break;
  case SERVICE_CONTROL_SHUTDOWN: OnServiceShutdown(); break;
  case SERVICE_CONTROL_INTERROGATE: break;
  default: break;
  }
}

static void WINAPI ServiceMain(DWORD dwArgc, PWSTR *pszArgv) {
  // Register the handler function for the service
  m_statusHandle = RegisterServiceCtrlHandlerW(SERVICE_NAME, ServiceCtrlHandler);
  if (m_statusHandle == NULL)
    throw GetLastError();
  // Start the service.
  OnServiceStart(dwArgc, pszArgv);
}

static const SERVICE_TABLE_ENTRYW serviceTable[] = {
  {SERVICE_NAME, ServiceMain},
  {NULL, NULL}
};

///////////////////////////////////////////////////////////////////////////////////////
// TunsafeServiceManager
///////////////////////////////////////////////////////////////////////////////////////

TunsafeServiceManager::TunsafeServiceManager()
    : pipe_manager_(TUNSAFE_PIPE_NAME, true, this) {
  server_unique_id_ = 0;
  
  hkey_ = NULL;
  RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\TunSafe", NULL, NULL, 0, KEY_ALL_ACCESS, NULL, &hkey_, NULL);

  main_backend_ = new TunsafeServiceBackend(this);
  backends_.push_back(main_backend_);
}

TunsafeServiceManager::~TunsafeServiceManager() {
  for (TunsafeServiceBackend *backend : backends_)
    delete backend;
  RegCloseKey(hkey_);
}

void TunsafeServiceManager::HandleNotify() {
  for (TunsafeServiceBackend *backend : backends_)
    backend->HandleNotify();
}

PipeConnection::Delegate *TunsafeServiceManager::HandleNewConnection(PipeConnection *connection) {
  TunsafeServiceServer *server = new TunsafeServiceServer(connection, main_backend_, server_unique_id_++);
  main_backend_->AddPipeServer(server);
  pipe_manager_.TryStartNewListener();
  return server;
}

unsigned TunsafeServiceManager::OnStart(int argc, wchar_t **argv) {
  uint32 service_flags = RegReadInt(hkey_, "ServiceStartupFlags", 0);
  if ((service_flags & kStartupFlag_BackgroundService) && (service_flags & kStartupFlag_ConnectWhenWindowsStarts)) {
    char *conf = RegReadStr(hkey_, "LastUsedConfigFile", "");
    if (conf && *conf)
      main_backend_->Start(conf);
    free(conf);
  }
  pipe_manager_.StartThread();
  return 0;
}

void TunsafeServiceManager::OnStop() {
  pipe_manager_.StopThread();
  for (TunsafeServiceBackend *backend : backends_)
    backend->Stop();
}

void TunsafeServiceManager::OnShutdown() {

}

TunsafeServiceBackend *TunsafeServiceManager::CreateBackend(const char *guid) {
  TunsafeServiceBackend *service_backend = new TunsafeServiceBackend(this);
 
  // If we're unable to assign the name, maybe it's already in use
  if (!service_backend->backend()->SetTunAdapterName(guid)) {
    delete service_backend;
    return NULL;
  }
  backends_.push_back(service_backend);
  return service_backend;
}

void TunsafeServiceManager::DestroyBackend(TunsafeServiceBackend *service_backend) {
  assert(service_backend != main_backend_);

  // Erase from the list
  auto it = std::find(backends_.begin(), backends_.end(), service_backend);
  if (it != backends_.end())
    backends_.erase(it);

  delete service_backend;
}

bool TunsafeServiceManager::SwitchInterface(TunsafeServiceServer *server, const char *interfac, bool want_create) {
  // Find a backend by name
  TunsafeBackend *backend = TunsafeBackend::FindBackendByTunGuid(interfac);
  TunsafeServiceBackend *service_backend = NULL;
  if (backend) {
    for (TunsafeServiceBackend *sb : backends_) {
      if (sb->backend() == backend) {
        service_backend = sb;
        break;
      }
    }
  }
  if (!service_backend) {
    if (!want_create)
      return false;
    service_backend = CreateBackend(interfac);
    if (!service_backend)
      return false;
  }
  if (server->service_backend() != service_backend) {
    server->service_backend()->RemovePipeServer(server);
    service_backend->AddPipeServer(server);
    server->set_service_backend(service_backend);
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////////////////
// TunsafeServiceBackend
///////////////////////////////////////////////////////////////////////////////////////

TunsafeServiceBackend::TunsafeServiceBackend(TunsafeServiceManager *manager) {
  token_request_flag_ = 0;
  manager_ = manager;
  historical_log_lines_count_ = historical_log_lines_pos_ = 0;
  memset(historical_log_lines_, 0, sizeof(historical_log_lines_));
  HANDLE event = manager_->pipe_manager_.notify_handle();
  thread_delegate_ = CreateTunsafeBackendDelegateThreaded(this, [=] { SetEvent(event); });
  backend_ = CreateNativeTunsafeBackend(thread_delegate_);
}

TunsafeServiceBackend::~TunsafeServiceBackend() {
  assert(pipe_servers_.empty());
  delete backend_;
  delete thread_delegate_;
}

void TunsafeServiceBackend::OnGetStats(const WgProcessorStats &stats) {
  for (TunsafeServiceServer *pipe_server : pipe_servers_)
    if (pipe_server->want_stats())
      pipe_server->WritePacket(TS_SERVICE_MSG_STATS, (uint8*)&stats, sizeof(stats));
}

void TunsafeServiceBackend::OnClearLog() {
  historical_log_lines_pos_ = 0;
  historical_log_lines_count_ = 0;
  for (TunsafeServiceServer *pipe_server : pipe_servers_)
    if (pipe_server->want_state_updates())
      pipe_server->WritePacket(TS_SERVICE_MSG_CLEARLOG, NULL, 0);
}

void TunsafeServiceBackend::OnLogLine(const char **s) {
  assert(manager_->pipe_manager_.VerifyThread());
  char *ss = (char*)*s;
  *s = NULL;
  char *&x = historical_log_lines_[historical_log_lines_pos_++ & (LOGLINE_COUNT - 1)];
  std::swap(x, ss);
  if (historical_log_lines_count_ < LOGLINE_COUNT)
    historical_log_lines_count_++;
  free(ss);

  for (TunsafeServiceServer *pipe_server : pipe_servers_)
    pipe_server->SendQueuedLogLines();
}

void TunsafeServiceBackend::OnStateChanged() {
  SendStateUpdate(NULL); // Send to all
}

void TunsafeServiceBackend::OnStatusCode(TunsafeBackend::StatusCode status) {
  if (status == TunsafeBackend::kStatusConnected)
    OnStateChanged(); // ensure we know the ip first
  uint32 v32 = (uint32)status;
  for (TunsafeServiceServer *pipe_server : pipe_servers_)
    if (pipe_server->want_state_updates())
      pipe_server->WritePacket(TS_SERVICE_MSG_STATUS_CODE, (uint8*)&v32, 4);
}

void TunsafeServiceBackend::OnGraphAvailable() {
  for (TunsafeServiceServer *pipe_server : pipe_servers_)
    pipe_server->OnGraphAvailable();
}

void TunsafeServiceBackend::OnConfigurationProtocolReply(uint32 ident, const std::string &&reply) {
  for (TunsafeServiceServer *pipe_server : pipe_servers_)
    if (pipe_server->unique_id() == ident)
      pipe_server->WritePacket(TS_SERVICE_REQ_TEXT_PROTOCOL_REPLY, (uint8*)reply.data(), reply.size());
}

void TunsafeServiceBackend::Start(const char *filename) {
  g_allow_pre_post = RegReadInt(manager_->hkey_, "AllowPrePost", 0) != 0;
  current_filename_ = filename;
  backend_->Start(filename);
}

void TunsafeServiceBackend::RememberLastUsedConfigFile(const char *filename) {
  if (manager_->main_backend() == this)
    RegWriteStr(manager_->hkey_, "LastUsedConfigFile", filename);
}

void TunsafeServiceBackend::Stop() {
  if (manager_->main_backend() == this)
    RegWriteStr(manager_->hkey_, "LastUsedConfigFile", "");

  backend_->Stop();
  OnStateChanged();
}

void TunsafeServiceBackend::UpdateRequestStats() {
  bool want = false;
  for (auto it = pipe_servers_.begin(); it != pipe_servers_.end(); ++it) {
    if ((*it)->want_stats()) {
      want = true;
      break;
    }
  }
  backend_->RequestStats(want);
}

void TunsafeServiceBackend::HandleNotify() {
  thread_delegate_->DoWork();
}

void TunsafeServiceBackend::SendStateUpdate(TunsafeServiceServer *filter) {
  if (pipe_servers_.empty())
    return;

  uint8 *temp = new uint8[current_filename_.size() + 1 + sizeof(ServiceState)];

  memset(temp, 0, sizeof(ServiceState));
  ServiceState *ss = (ServiceState *)temp;
  ss->is_started = backend_->is_started();
  ss->internet_block_state = backend_->GetInternetBlockState();
  ss->ipv4_ip = backend_->GetIP();
  ss->token_request = backend_->GetTokenRequest();
  ss->token_request_flag = token_request_flag_;
  memcpy(ss->public_key, backend_->public_key(), 32);
  memcpy(temp + sizeof(ServiceState), current_filename_.c_str(), current_filename_.size() + 1);
  for (TunsafeServiceServer *pipe_server : pipe_servers_) {
    if (filter != NULL && pipe_server != filter)
      continue;
    if (pipe_server->want_state_updates())
      pipe_server->WritePacket(TS_SERVICE_MSG_STATE, temp, current_filename_.size() + 1 + sizeof(ServiceState));
  }

  delete[] temp;
}

void TunsafeServiceBackend::RemovePipeServer(TunsafeServiceServer *pipe_server) {
  auto it = std::find(pipe_servers_.begin(), pipe_servers_.end(), pipe_server);
  if (it != pipe_servers_.end())
    pipe_servers_.erase(it);

  UpdateRequestStats();

  // Stop the main backend, or destroy a disconnetced backend, when the last client disconnects.
  if (pipe_servers_.empty()) {
    if (this == manager_->main_backend_) {
      uint32 service_flags = RegReadInt(manager_->hkey_, "ServiceStartupFlags", 0);
      if (!(service_flags & kStartupFlag_BackgroundService))
        backend_->Stop();
    } else {
      if (!backend_->is_started())
        manager_->DestroyBackend(this);
    }
  }
}

void TunsafeServiceBackend::AddPipeServer(TunsafeServiceServer *pipe_server) {
  pipe_servers_.push_back(pipe_server);
}

///////////////////////////////////////////////////////////////////////////////////////
// TunsafeServiceServer
///////////////////////////////////////////////////////////////////////////////////////

TunsafeServiceServer::TunsafeServiceServer(PipeConnection *pipe, TunsafeServiceBackend *backend, uint32 unique_id) {
  unique_id_ = unique_id;
  connection_ = pipe;
  service_backend_ = backend;
  last_line_sent_ = 0;
  want_state_updates_ = false;
  did_authenticate_user_ = false;
  want_stats_ = false;
  want_graph_type_ = 0xffffffff;
}

TunsafeServiceServer::~TunsafeServiceServer() {
}

void TunsafeServiceServer::WritePacket(int type, const uint8 *data, size_t data_size) {
  connection_->WritePacket(type, data, data_size);
}

struct ServiceLoginMessage {
  uint64 version;
  char interfac[kTsMaxDevnameSize];
  bool want_state_updates;
  bool want_create_interface;
};

bool TunsafeServiceServer::HandleMessage(int type, uint8 *data, size_t size) {
  if (!did_authenticate_user_) {
    if (type != TS_SERVICE_REQ_LOGIN ||
        size < sizeof(ServiceLoginMessage) || 
        ((ServiceLoginMessage*)data)->version != TUNSAFE_SERVICE_PROTOCOL_VERSION) {
      const char *s = "Versioning Problem: The TunSafe service is a different version than the UI.";
      connection_->WritePacket(TS_SERVICE_MSG_ERROR_REPLY, (uint8*)s, strlen(s));
      return false;
    }
    if (!AuthenticateUser()) {
      const char *s = "Permission Problem: Your Windows account is different from the account\r\nthat installed the TunSafe Service. Please reinstall it.";
      connection_->WritePacket(TS_SERVICE_MSG_ERROR_REPLY, (uint8*)s, strlen(s));
      return false;
    }
  }

  switch (type) {
  case TS_SERVICE_REQ_START: {
    if (size == 0 || data[size - 1] != 0)
      return false;

    for (size_t i = 0; i < size; i++) {
      if (data[i] == '/')
        data[i] = '\\';
    }

    char buf[MAX_PATH];
    buf[0] = 0;

    if (data[0]) {
      if (!ExpandConfigPath((char*)data, buf, sizeof(buf)) || GetFileAttributesA(buf) == INVALID_FILE_ATTRIBUTES) {
        char *s = str_cat_alloc("File '", (char*)data, "' not found");
        connection_->WritePacket(TS_SERVICE_MSG_ERROR_REPLY, (uint8*)s, strlen(s));
        free(s);
        return false;
      }
      // Don't allow reading arbitrary files on disk
      if (!EnsureValidConfigPath(buf)) {
        GetConfigPath(buf, sizeof(buf));
        char *s = str_cat_alloc("Permission Problem: The Config file is in an unsafe location.\r\n   Must be in: ", buf, "");
        connection_->WritePacket(TS_SERVICE_MSG_ERROR_REPLY, (uint8*)s, strlen(s));
        free(s);
        return false;
      }
    }
    service_backend_->Start(buf);
    service_backend_->RememberLastUsedConfigFile(buf);

    // Ensure we reply with something
    if (!want_state_updates_) {
      uint32 v32 = (uint32)service_backend_->backend_->status();
      connection_->WritePacket(TS_SERVICE_MSG_STATUS_CODE, (uint8*)&v32, 4);
    }
    break;
  }

  case TS_SERVICE_REQ_STOP:
    service_backend_->Stop();
    if (!want_state_updates_) {
      uint32 v32 = (uint32)service_backend_->backend_->status();
      connection_->WritePacket(TS_SERVICE_MSG_STATUS_CODE, (uint8*)&v32, 4);
    }
    break;

  case TS_SERVICE_REQ_LOGIN: {
    if (((ServiceLoginMessage*)data)->interfac[kTsMaxDevnameSize - 1])
      return false; // sanity check

    if (((ServiceLoginMessage*)data)->interfac[0] != 0) {
      if (!service_backend_->manager_->SwitchInterface(this, ((ServiceLoginMessage*)data)->interfac, ((ServiceLoginMessage*)data)->want_create_interface)) {
        const char *s = ((ServiceLoginMessage*)data)->want_create_interface ? "Unable to add the interface" : "Interface is not started";
        connection_->WritePacket(TS_SERVICE_MSG_ERROR_REPLY, (uint8*)s, strlen(s));
        return false;
      }
    }
    want_state_updates_ = ((ServiceLoginMessage*)data)->want_state_updates;
    if (want_state_updates_) {
      SendQueuedLogLines();
      service_backend_->SendStateUpdate(this);
      uint32 v32 = (uint32)service_backend_->backend_->status();
      connection_->WritePacket(TS_SERVICE_MSG_STATUS_CODE, (uint8*)&v32, 4);
    }

    break;
  }

  // return a list of all running interfaces
  case TS_SERVICE_REQ_GETINTERFACES: {
    char *s = TunsafeBackend::GetAllGuid();
    connection_->WritePacket(TS_SERVICE_REQ_GETINTERFACES_REPLY, (uint8*)s, s ? strlen(s) : 0);
    free(s);
    break;
  }

  case TS_SERVICE_REQ_GETSTATS:
    if (size < 1) return false;
    want_stats_ = (data[0] != 0);
    service_backend_->UpdateRequestStats();
    break;

  case TS_SERVICE_REQ_SET_INTERNET_BLOCKSTATE:
    if (size < 2)
      return false;
    service_backend_->backend_->SetInternetBlockState((InternetBlockState)Read16(data));
    break;

  case TS_SERVICE_REQ_RESETSTATS:
    service_backend_->backend_->ResetStats();
    break;

  case TS_SERVICE_REQ_GET_GRAPH:
    if (size < 4) return false;
    want_graph_type_ = *(int*)data;
    TunsafeServiceServer::OnGraphAvailable();
    break;

  case TS_SERVICE_REQ_SET_STARTUP_FLAGS:
    if (size < 4)
      return false;
    RegSetValueEx(service_backend_->manager_->hkey_, "ServiceStartupFlags", NULL, REG_DWORD, (BYTE*)data, 4);
    break;

  case TS_SERVICE_REQ_TEXT_PROTOCOL:
    if (!service_backend_->backend_->is_started())
      service_backend_->Start("");
    service_backend_->backend_->SendConfigurationProtocolPacket(unique_id_, std::string((char*)data, size));
    break;
  case TS_SERVICE_REQ_SUBMIT_TOKEN:
    service_backend_->token_request_flag_ ^= 1;
    service_backend_->backend_->SubmitToken(std::string((char*)data, size));
    break;

  default:
    return false;
  }
  return true;
}

void TunsafeServiceServer::HandleDisconnect() {
  service_backend_->RemovePipeServer(this);
  delete this;
}

void TunsafeServiceServer::OnGraphAvailable() {
  if (want_graph_type_ != 0xffffffff) {
    LinearizedGraph *graph = service_backend_->backend_->GetGraph(want_graph_type_);
    if (graph) {
      connection_->WritePacket(TS_SERVICE_MSG_GRAPH, (uint8*)graph, graph->total_size);
      free(graph);
    }
  }
}

void TunsafeServiceServer::SendQueuedLogLines() {
  if (!want_state_updates_)
    return;
  assert(connection_->VerifyThread());
  uint32 maxi = std::min<uint32>(service_backend_->historical_log_lines_count_, service_backend_->historical_log_lines_pos_ - last_line_sent_);
  last_line_sent_ = service_backend_->historical_log_lines_pos_;
  for (uint32 i = 0; i < maxi; i++) {
    const char *s = service_backend_->historical_log_lines_[(service_backend_->historical_log_lines_pos_ - maxi + i) & (TunsafeServiceBackend::LOGLINE_COUNT - 1)];
    if (s)
      connection_->WritePacket(TS_SERVICE_MSG_LOGLINE, (uint8*)s, strlen(s));
  }
}

bool TunsafeServiceServer::AuthenticateUser() {
  if (!ImpersonateNamedPipeClient(connection_->pipe_handle()))
    return false;
  wchar_t *user = GetUsernameOfCurrentUser(true);
  RevertToSelf();
  if (!user)
    return false;
  wchar_t *valid_user = RegReadStrW(service_backend_->manager_->hkey_, L"AllowedUsername", L"");
  bool rv = valid_user && wcscmp(user, valid_user) == 0;
  did_authenticate_user_ = rv;
  free(user);
  free(valid_user);
  return rv;
}

static void PushServiceLine(int type, const char *s) {
  if (g_service) {
    char buf[64];
    SYSTEMTIME t;

    size_t l = strlen(s);
    GetLocalTime(&t);
    snprintf(buf, sizeof(buf), "[%.2d:%.2d:%.2d] ", t.wHour, t.wMinute, t.wSecond);
    size_t tl = strlen(buf);

    char *x = (char*) malloc(tl + l + 1);
    memcpy(x, buf, tl);
    memcpy(x + tl, s, l);
    x[l + tl] = '\0';
    g_service->main_backend()->delegate()->OnLogLine((const char**)&x);
    free(x);
  } else {
    size_t l = strlen(s);
    char buf[1024];
    SYSTEMTIME t;
    GetLocalTime(&t);

    snprintf(buf, sizeof(buf), "[%.2d:%.2d:%.2d] ", t.wHour, t.wMinute, t.wSecond);
    size_t tl = strlen(buf);

    if (l >= ARRAYSIZE(buf) - tl - 1)
      l = ARRAYSIZE(buf) - tl - 1;

    memcpy(buf + tl, s, l);
    buf[l + tl] = '\0';

    WriteServiceLog(buf, EVENTLOG_INFORMATION_TYPE);
  }
}

BOOL RunProcessAsTunsafeServiceProcess() {
  g_service = new TunsafeServiceManager;
  g_logger = &PushServiceLine;

#if SERVICE_DEBUGGING
  g_service->OnStart(NULL, 0);
  while (true)
    Sleep(1000);
#endif

  // Connects the main thread of a service process to the service control 
  // manager, which causes the thread to be the service control dispatcher 
  // thread for the calling process. This call returns when the service has 
  // stopped. The process should simply terminate when the call returns.
  return StartServiceCtrlDispatcherW(serviceTable);
}


///////////////////////////////////////////////////////////////////////////////////////
// TunsafeServiceClient
///////////////////////////////////////////////////////////////////////////////////////

TunsafeServiceClient::TunsafeServiceClient(TunsafeBackend::Delegate *delegate)
  : pipe_manager_(TUNSAFE_PIPE_NAME, false, this) {
  is_remote_ = true;
  got_state_from_control_ = false;
  delegate_ = delegate;
  cached_graph_ = 0;
  last_graph_type_ = 0xffffffff;
  token_request_flag_ = 0xff;
  memset(&service_state_, 0, sizeof(service_state_));
  connection_ = pipe_manager_.GetClientConnection();
}

TunsafeServiceClient::~TunsafeServiceClient() {
  pipe_manager_.StopThread();
}

bool TunsafeServiceClient::Configure() {
  return pipe_manager_.StartThread();
}

void TunsafeServiceClient::Start(const char *config_file) {
  connection_->WritePacket(TS_SERVICE_REQ_START, (uint8*)config_file, strlen(config_file) + 1);
}

void TunsafeServiceClient::Stop() {
  connection_->WritePacket(TS_SERVICE_REQ_STOP, NULL, 0);
}

void TunsafeServiceClient::RequestStats(bool enable) {
  want_stats_ = enable;
  if (connection_->is_connected())
    connection_->WritePacket(TS_SERVICE_REQ_GETSTATS, &want_stats_, 1);
}

void TunsafeServiceClient::ResetStats() {
  connection_->WritePacket(TS_SERVICE_REQ_RESETSTATS, NULL, 0);
}

InternetBlockState TunsafeServiceClient::GetInternetBlockState() {
  return (InternetBlockState)service_state_.internet_block_state;
}

void TunsafeServiceClient::SetInternetBlockState(InternetBlockState s) {
  uint16 v = (uint16)s;
  connection_->WritePacket(TS_SERVICE_REQ_SET_INTERNET_BLOCKSTATE, (uint8*)&v, sizeof(v));
}

void TunsafeServiceClient::SetServiceStartupFlags(uint32 flags) {
  connection_->WritePacket(TS_SERVICE_REQ_SET_STARTUP_FLAGS, (uint8*)&flags, 4);
}

LinearizedGraph *TunsafeServiceClient::GetGraph(int type) {
  if (type != last_graph_type_) {
    last_graph_type_ = type;
    connection_->WritePacket(TS_SERVICE_REQ_GET_GRAPH, (uint8*)&type, 4);
  }
  mutex_.Acquire();
  LinearizedGraph *graph = cached_graph_;
  LinearizedGraph *new_graph = (graph && graph->graph_type == type) ? (LinearizedGraph*)memdup(graph, graph->total_size) : NULL;
  mutex_.Release();
  return new_graph;
}

void TunsafeServiceClient::SendConfigurationProtocolPacket(uint32 identifier, const std::string &&message) {
}

uint32 TunsafeServiceClient::GetTokenRequest() {
  mutex_.Acquire();
  uint32 rv = (token_request_flag_ == service_state_.token_request_flag) ? 0 : service_state_.token_request;
  mutex_.Release();
  return rv;
}

void TunsafeServiceClient::SubmitToken(const std::string &&token) {
  token_request_flag_ = service_state_.token_request_flag;
  connection_->WritePacket(TS_SERVICE_REQ_SUBMIT_TOKEN, (const uint8*)token.data(), token.size());
}

std::string TunsafeServiceClient::GetConfigFileName() {
  mutex_.Acquire();
  std::string rv = config_file_;
  mutex_.Release();
  return rv;
}

bool TunsafeServiceClient::HandleMessage(int type, uint8 *data, size_t data_size) {
  switch (type) {
  case TS_SERVICE_MSG_STATE:
    if (data_size <= sizeof(service_state_) || data[data_size - 1])
      return false;
    got_state_from_control_ = true;

    mutex_.Acquire();
    config_file_.assign((char*)data + sizeof(service_state_), data_size - 1 - sizeof(service_state_));
    memcpy(&service_state_, data, sizeof(service_state_));
    memcpy(public_key_, service_state_.public_key, 32);
    is_started_ = service_state_.is_started;
    ipv4_ip_ = service_state_.ipv4_ip;
    mutex_.Release();
    delegate_->OnStateChanged();
    return true;
  case TS_SERVICE_MSG_LOGLINE: 
  case TS_SERVICE_MSG_ERROR_REPLY: {
    if (data_size == 0)
      return false;
    char *s = my_strndup((char*)data, data_size);
    if (s) {
      delegate_->OnLogLine((const char **)&s);
      free(s);
    }
    return true;
  }
  case TS_SERVICE_MSG_STATS: {
    WgProcessorStats stats;
    if (data_size != sizeof(WgProcessorStats))
      return false;
    memcpy(&stats, data, sizeof(WgProcessorStats));
    delegate_->OnGetStats(stats);
    return true;
  }
  case TS_SERVICE_MSG_CLEARLOG:
    delegate_->OnClearLog();
    return true;

  case TS_SERVICE_MSG_STATUS_CODE:
    if (data_size < 4)
      return false;
    status_ = (StatusCode)*(uint32*)data;
    delegate_->OnStatusCode(status_);
    return true;

  case TS_SERVICE_MSG_GRAPH:
    if (data_size < sizeof(LinearizedGraph) || data_size != *(uint32*)data)
      return false;
    LinearizedGraph *graph = (LinearizedGraph*)memdup(data, data_size);
    mutex_.Acquire();
    std::swap(graph, cached_graph_);
    mutex_.Release();
    free(graph);
    delegate_->OnGraphAvailable();
    return true;
  }

  return false;
}

void TunsafeServiceClient::HandleNotify() {
}

PipeConnection::Delegate *TunsafeServiceClient::HandleNewConnection(PipeConnection *connection) {
  assert(connection == connection_);
  ServiceLoginMessage msg = {0};
  msg.want_state_updates = true;
  msg.version = TUNSAFE_SERVICE_PROTOCOL_VERSION;
  connection_->WritePacket(TS_SERVICE_REQ_LOGIN, (uint8*)&msg, sizeof(msg));
  if (want_stats_)
    connection_->WritePacket(TS_SERVICE_REQ_GETSTATS, &want_stats_, 1);
  return this;
}

void TunsafeServiceClient::HandleDisconnect() {
  status_ = TunsafeBackend::kErrorServiceLost;
  delegate_->OnStatusCode(TunsafeBackend::kErrorServiceLost);
}

void TunsafeServiceClient::Teardown() {
  pipe_manager_.StopThread();
}

bool TunsafeServiceClient::SetTunAdapterName(const char *name) {
  // override which tun adapter we want to start
  return false;
}

TunsafeBackend *CreateTunsafeServiceClient(TunsafeBackend::Delegate *delegate) {
  TunsafeServiceClient *client = new TunsafeServiceClient(delegate);
  if (client && !client->Configure()) {
    delete client;
    client = NULL;
  }
  return client;
}
