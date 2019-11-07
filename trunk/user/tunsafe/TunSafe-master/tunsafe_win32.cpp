// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "wireguard_config.h"
#include "network_win32_api.h"
#include "network_win32_dnsblock.h"
#include "tunsafe_wg_plugin.h"
#include <Commctrl.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include <stddef.h>
#include "resource.h"
#include <string.h>
#include <Richedit.h>
#include <vector>
#include <Iphlpapi.h>
#include <assert.h>
#include <shldisp.h>
#include <shlobj.h>
#include <exdisp.h>
#include "tunsafe_endian.h"
#include "util.h"
#include <atlbase.h>
#include <algorithm>
#include "crypto/curve25519/curve25519-donna.h"
#include "service_win32.h"
#include "util_win32.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

void InitCpuFeatures();
void PrintCpuFeatures();
void Benchmark();
void ShowTwoFactorDialog();

static const char *GetCurrentConfigTitle(char *buf, size_t max_size);
static char *PrintMB(char *buf, int64 bytes);
static void LoadConfigFile(const char *filename, bool save, bool force_start);
static void SetCurrentConfigFilename(const char *filename);
static void CreateLocalOrRemoteBackend(bool remote);
static void UpdateGraphReq();

#pragma warning(disable: 4200)

static bool g_is_connected_to_server;
static bool g_notified_connected_server;
static HWND g_ui_window;
static HICON g_icons[2];
static bool g_minimize_on_connect;

static bool g_ui_visible;
static char *g_current_filename;
static HINSTANCE g_hinstance;
static TunsafeBackend *g_backend;
static TunsafeBackend::Delegate *g_backend_delegate;
static const char *g_cmdline_filename;
static bool g_first_state_msg;
static bool g_is_limited_uac_account;
static bool g_is_tunsafe_service_running;
static bool g_disable_connect_on_start;
static bool g_not_first_status_msg; 
static HANDLE g_runonce_mutex;
static int g_startup_flags;
static HKEY g_reg_key;
static HKEY g_hklm_reg_key;
static HKEY g_hklm_readonly_reg_key;
static HWND hwndPaintBox, hwndStatus, hwndGraphBox, hwndTab, hwndAdvancedBox, hwndEdit;
static WgProcessorStats g_processor_stats;
static int g_large_fonts;
static TunsafeBackend::StatusCode g_status_code;
static UINT g_message_taskbar_created;
static int g_current_tab;
static bool wm_dropfiles_recursive;
static bool g_has_icon;
static bool g_twofactor_dialog_shown;
static uint32 g_twofactor_dialog_request;
static int g_selected_graph_type;
static RECT comborect;
static HBITMAP arrowbitmap;
static uint32 g_timestamp_of_exit_menuloop;
enum UpdateIconWhy {
  UIW_NONE = 0,
  UIW_STOPPED_WORKING_FAIL = 1,
  UIW_START = 2,
};
static void UpdateIcon(UpdateIconWhy error);


int RescaleDpi(int size) {
  return (g_large_fonts == 96) ? size : size * g_large_fonts / 96;
}

RECT RescaleDpiRect(const RECT &r) {
  RECT rr = r;
  if (g_large_fonts != 96) {
    rr.left = rr.left * g_large_fonts / 96;
    rr.top = rr.top * g_large_fonts / 96;
    rr.right = rr.right * g_large_fonts / 96;
    rr.bottom = rr.bottom * g_large_fonts / 96;
  }
  return rr;
}

static void SetUiVisibility(bool visible) {
  g_ui_visible = visible;
  ShowWindow(g_ui_window, visible ? SW_SHOW : SW_HIDE);
  g_backend->RequestStats(visible);
  UpdateGraphReq();
}

void StopTunsafeBackend(UpdateIconWhy why) {
  if (g_backend->is_started()) {
    g_backend->Stop();
    if (g_is_connected_to_server)
      RINFO("Disconnected");
    g_is_connected_to_server = false;
    UpdateIcon(why);
    RegWriteInt(g_reg_key, "IsConnected", 0);
  }
}

const char *print_ip(char buf[kSizeOfAddress], uint32 ip) {
  snprintf(buf, kSizeOfAddress, "%d.%d.%d.%d", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, (ip >> 0) & 0xff);
  return buf;
}

void StartTunsafeBackend(UpdateIconWhy reason) {
  if (!*g_current_filename)
    return;

  // recreate service connection
  if (g_backend->status() == TunsafeBackend::kErrorServiceLost)
    CreateLocalOrRemoteBackend(g_backend->is_remote());

  if (g_backend->is_remote() && !EnsureValidConfigPath(g_current_filename)) {
    RERROR("The config file needs to be in the Config-directory. Maybe the TunSafe\r\n   process doesn't match with the running service. Try selecting 'Don't Use a Service'.");
    StopTunsafeBackend(UIW_NONE);
    return;
  }
  g_notified_connected_server = false;
  g_is_connected_to_server = false;
  memset(&g_processor_stats, 0, sizeof(g_processor_stats));
  g_backend->Start(g_current_filename);
  RegWriteInt(g_reg_key, "IsConnected", 1);
}

static void InvalidatePaintbox() {
  InvalidateRect(hwndPaintBox, NULL, FALSE);
}

class MyBackendDelegate : public TunsafeBackend::Delegate {
public:
  virtual void OnGraphAvailable() {
    InvalidateRect(hwndGraphBox, NULL, FALSE);
  }

  virtual void OnGetStats(const WgProcessorStats &stats) {
    g_processor_stats = stats;
    InvalidatePaintbox();

    char buf[64];
    uint32 mbs_in = (uint32)(stats.data_bytes_in_per_second * (1.0 / 1250));
    uint32 gb_in = (uint32)(stats.data_bytes_in * (1.0 / (1024 * 1024 * 1024 / 100)));

    snprintf(buf, ARRAYSIZE(buf), "D: %d.%.2d Mbps (%d.%.2d GB)", mbs_in / 100, mbs_in % 100, gb_in / 100, gb_in % 100);
    SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)buf);

    uint32 mbs_out = (uint32)(stats.data_bytes_out_per_second * (1.0 / 1250));
    uint32 gb_out = (uint32)(stats.data_bytes_out * (1.0 / (1024 * 1024 * 1024 / 100)));

    snprintf(buf, ARRAYSIZE(buf), "U: %d.%.2d Mbps (%d.%.2d GB)", mbs_out / 100, mbs_out % 100, gb_out / 100, gb_out % 100);
    SendMessage(hwndStatus, SB_SETTEXT, 2, (LPARAM)buf);

    InvalidateRect(hwndAdvancedBox, NULL, FALSE);
  }

  virtual void OnLogLine(const char **s) {
    const char *line = *s;
    size_t len = strlen(line);
    char *tmp = (char*)alloca(len + 3);

    tmp[len + 0] = '\r';
    tmp[len + 1] = '\n';
    tmp[len + 2] = 0;
    memcpy(tmp, line, len);

    CHARRANGE cr;
    cr.cpMin = -1;
    cr.cpMax = -1;
    // hwnd = rich edit hwnd
    SendMessage(hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
    SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)tmp);
  }

  virtual void OnStateChanged() {
    if (!g_first_state_msg) {
      g_first_state_msg = true;
      char fullname[1024];

      const char *filename = g_cmdline_filename;
      if (filename) {
        if (ExpandConfigPath(filename, fullname, sizeof(fullname)))
          SetCurrentConfigFilename(fullname);
      } else {
        std::string currconfig = g_backend->GetConfigFileName();
        if (currconfig.empty()) {
          char *conf = RegReadStr(g_reg_key, "ConfigFile", "TunSafe.conf");
          if (ExpandConfigPath(conf, fullname, sizeof(fullname)))
            SetCurrentConfigFilename(fullname);
          free(conf);
        } else {
          SetCurrentConfigFilename(currconfig.c_str());
        }
      }

      if (filename != NULL || !(g_startup_flags & kStartupFlag_BackgroundService) && !g_disable_connect_on_start && RegReadInt(g_reg_key, "IsConnected", 0)) {
        StartTunsafeBackend(UIW_START);
      } else {
        if (!g_backend->is_started())
          RINFO("Press Connect to initiate a connection to the WireGuard server.");
      }
    }

    uint32 state = g_backend->GetInternetBlockState();
    bool running = g_backend->is_started();

    ShowWindow(GetDlgItem(g_ui_window, ID_BTN_KILLSWITCH), (!running || (state & kBlockInternet_Both) == 0) && (state & kBlockInternet_Active) ? SW_SHOW : SW_HIDE);
      
    SetDlgItemText(g_ui_window, ID_START, running ? "Re&connect" : "&Connect");
    InvalidatePaintbox();
    EnableWindow(GetDlgItem(g_ui_window, ID_STOP), running);

    if (running && !g_twofactor_dialog_shown) {
      uint32 token_request = g_backend->GetTokenRequest();
      if (token_request != 0) {
        g_twofactor_dialog_shown = true;
        g_twofactor_dialog_request = token_request;
        PostMessage(g_ui_window, WM_USER + 3, 0, 0); // show two factor dialog
      }
    }
  }

  virtual void OnStatusCode(TunsafeBackend::StatusCode status) override {
    if (status != g_status_code)
      InvalidatePaintbox();

    g_status_code = status;
    if (TunsafeBackend::IsPermanentError(status)) {
      UpdateIcon(g_is_connected_to_server ? UIW_STOPPED_WORKING_FAIL : UIW_NONE);
      return;
    }
    bool is_connected = (status == TunsafeBackend::kStatusConnected);
    if (is_connected && g_minimize_on_connect) {
      g_minimize_on_connect = false;
      SetUiVisibility(false);
    }

    bool not_first = g_not_first_status_msg;
    g_not_first_status_msg = true;

    if (is_connected != g_is_connected_to_server) {
      g_is_connected_to_server = is_connected;
      // avoid showing a notice if service is already connected
      if (is_connected > not_first && (g_startup_flags & kStartupFlag_BackgroundService))
        g_notified_connected_server = true;
      UpdateIcon(UIW_NONE);
    }
  }

  virtual void OnClearLog() override {
    SetWindowText(hwndEdit, "");
  }

  virtual void OnConfigurationProtocolReply(uint32 ident, const std::string &&reply) override {
  }
};

static MyBackendDelegate my_procdel;

static void CreateLocalOrRemoteBackend(bool remote) {
  delete g_backend;

  if (!remote) {
    g_backend = CreateNativeTunsafeBackend(g_backend_delegate);
  } else {
    RINFO("Connecting to the TunSafe Service...");
    g_backend = CreateTunsafeServiceClient(g_backend_delegate);
  }

  g_backend->RequestStats(g_ui_visible);
}

static char *PrintMB(char *buf, int64 bytes) {
  char *bo = buf;
  if (bytes < 0) {
    *buf++ = '-';
    bytes = -bytes;
  }
  int64 big = bytes / (1024*1024);
  int little = bytes % (1024*1024);
  if (bytes < 10*1024*1024) {
    // X.XXX
    snprintf(buf, 64, "%lld.%.3d MB", big, 1000 * little / (1024*1024));
  } else if (bytes < 100*1024*1024) {
    // XX.XX
    snprintf(buf, 64, "%lld.%.2d MB", big, 100 * little / (1024*1024));
  } else {
    // XX.X
    snprintf(buf, 64, "%lld.%.1d MB", big, 10 * little / (1024*1024));
  }
  return bo;
}

static void UpdateIcon(UpdateIconWhy why) {
  NOTIFYICONDATA nid;
  memset(&nid, 0, sizeof(nid));
  nid.cbSize = sizeof(nid);
  nid.hWnd = g_ui_window;
  nid.uID = 1;
  nid.uVersion = NOTIFYICON_VERSION;
  nid.uCallbackMessage = WM_USER + 1;
  nid.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
  nid.hIcon = g_icons[g_is_connected_to_server ? 0 : 1];
  
  char buf[kSizeOfAddress];
  char namebuf[128];
  if (g_is_connected_to_server) {
    snprintf(nid.szTip, sizeof(nid.szTip), "TunSafe [%s - %s]", GetCurrentConfigTitle(namebuf, sizeof(namebuf)), print_ip(buf, g_backend->GetIP()));
    if (!g_notified_connected_server) {
      g_notified_connected_server = true;
      nid.uFlags |= NIF_INFO;
      snprintf(nid.szInfoTitle, sizeof(nid.szInfoTitle), "Connected to: %s", namebuf);
      snprintf(nid.szInfo, sizeof(nid.szInfo), "IP: %s", buf);
      nid.uTimeout = 5000;
      nid.dwInfoFlags = NIIF_INFO;
    }
  } else {
    g_notified_connected_server = false;
    snprintf(nid.szTip, sizeof(nid.szTip), "TunSafe [%s]", "Disconnected");

    if (why == UIW_STOPPED_WORKING_FAIL) {
      nid.uFlags |= NIF_INFO;
      strcpy(nid.szInfoTitle, "Disconnected!");
      strcpy(nid.szInfo, "There was a problem with the connection. You are now disconnected.");
      nid.uTimeout = 5000;
      nid.dwInfoFlags = NIIF_ERROR;
    }
  }
  Shell_NotifyIcon(g_has_icon ? NIM_MODIFY : NIM_ADD, &nid);

  SendMessage(g_ui_window, WM_SETICON, ICON_SMALL, (LPARAM)g_icons[g_is_connected_to_server ? 0 : 1]);

  g_has_icon = true;
}

static void RemoveIcon() {
  if (g_has_icon) {
    NOTIFYICONDATA nid;
    memset(&nid, 0, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_ui_window;
    nid.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nid);
  }
}

#define MAX_CONFIG_FILES 1024
#define ID_POPUP_CONFIG_FILE 10000
char *config_filenames[MAX_CONFIG_FILES];
uint8 config_filenames_indent[MAX_CONFIG_FILES];

static char *StripConfExtension(const char *src, char *target, size_t size) {
  size_t len = strlen(src);
  if (len >= 5 && memcmp(src + len - 5, ".conf", 5) == 0)
    len -= 5;

  len = std::min(len, size - 1);
  target[len] = 0;
  memcpy(target, src, len);
  return target;
}

static const char *GetCurrentConfigTitle(char *target, size_t size) {
  const char *ll = FindFilenameComponent(g_current_filename);
  return StripConfExtension(ll, target, size);
}

static void SetCurrentConfigFilename(const char *filename) {
  str_set(&g_current_filename, filename);
  char namebuf[64];
  char *f = str_cat_alloc("TunSafe - ", GetCurrentConfigTitle(namebuf, sizeof(namebuf)));
  SetWindowText(g_ui_window, f);
  free(f);

  InvalidateRect(hwndPaintBox, NULL, FALSE);
}


static void LoadConfigFile(const char *filename, bool save, bool force_start) {
  SetCurrentConfigFilename(filename);

  if (force_start || g_backend->is_started())
    StartTunsafeBackend(UIW_START);

  if (save)
    RegWriteStr(g_reg_key, "ConfigFile", filename);
}

class ConfigMenuBuilder {
public:
  ConfigMenuBuilder();
 
  void Recurse();

  int depth_;
  int nfiles_;
  size_t bufpos_;
  WIN32_FIND_DATA wfd_;
  char buf_[1024];
};

ConfigMenuBuilder::ConfigMenuBuilder() 
    : nfiles_(0), depth_(0) {
  if (!ExpandConfigPath("", buf_, sizeof(buf_)))
    bufpos_ = sizeof(buf_);
  else
    bufpos_ = strlen(buf_);
}

void ConfigMenuBuilder::Recurse() {
  if (bufpos_ >= sizeof(buf_) - 4)
    return;
  memcpy(buf_ + bufpos_, "*.*", 4);
  HANDLE handle = FindFirstFile(buf_, &wfd_);
  if (handle != INVALID_HANDLE_VALUE) {
    do {
      if (wfd_.cFileName[0] == '.')
        continue;

      size_t len = strlen(wfd_.cFileName);
      if (bufpos_ + len >= sizeof(buf_) - 1)
        continue;

      // Ensure it ends with .conf
      if (!(wfd_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (len < 5 || _strnicmp(&wfd_.cFileName[len - 5], ".conf", 5) != 0))
        continue;

      size_t old_bufpos = bufpos_;
      memcpy(buf_ + bufpos_, wfd_.cFileName, len + 1);
      bufpos_ = bufpos_ + len + 1;
      config_filenames_indent[nfiles_] = depth_ + !!(wfd_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
      str_set(&config_filenames[nfiles_], buf_);
      nfiles_++;
      if (nfiles_ == MAX_CONFIG_FILES)
        break;
      if (wfd_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        buf_[bufpos_ - 1] = '\\';
        int old_nfiles = nfiles_;
        depth_++;
        if (depth_ < 16)
          Recurse();
        depth_--;
        // Remove directory if it had no files
        if (old_nfiles == nfiles_)
          nfiles_--;

        if (nfiles_ == MAX_CONFIG_FILES)
          break;

      }
      bufpos_ = old_bufpos;
    } while (FindNextFile(handle, &wfd_));
    FindClose(handle);
  }
}


static int AddToAvailableFilesPopup(HMENU menu, int max_num_items, bool is_settings) {
  ConfigMenuBuilder menu_builder;
  HMENU where[16] = {0};

  menu_builder.Recurse();

  bool is_connected = g_backend->is_started();
  uint32 last_indent = 0;
  where[0] = menu;

  for (int i = 0; i < menu_builder.nfiles_; i++) {
    uint32 indent = config_filenames_indent[i];
    if (indent > last_indent) {
      HMENU n = CreatePopupMenu();
      where[indent] = n;
      AppendMenu(where[last_indent], MF_POPUP, (UINT_PTR)n, FindFilenameComponent(config_filenames[i]));
    } else {
      bool selected_item = (strcmp(g_current_filename, config_filenames[i]) == 0);
      AppendMenu(where[indent], (selected_item && is_connected) ?
          MF_CHECKED : 0, ID_POPUP_CONFIG_FILE + i,
          StripConfExtension(
            FindFilenameComponent(config_filenames[i]), menu_builder.buf_, sizeof(menu_builder.buf_)));
      if (selected_item)
        SetMenuDefaultItem(where[indent], ID_POPUP_CONFIG_FILE + i, MF_BYCOMMAND);
    }
    last_indent = indent;
  }

  if (menu_builder.nfiles_ == 0)
    AppendMenu(menu, MF_GRAYED | MF_DISABLED, 0, "(no config files found)");

  return menu_builder.nfiles_;
}

static void ShowSettingsMenu(HWND wnd) {
  HMENU menu = CreatePopupMenu();

  AddToAvailableFilesPopup(menu, 10, true);

  //POINT pt;
  //GetCursorPos(&pt);

  RECT r = GetParentRect(GetDlgItem(g_ui_window, ID_START));

  RECT r2 = GetParentRect(hwndPaintBox);

  POINT pt = {r2.left, r.bottom};

  ClientToScreen(g_ui_window, &pt);




  int rv = TrackPopupMenu(menu, 0, pt.x, pt.y, 0, wnd, NULL);
  DestroyMenu(menu);
}

static bool HasReadWriteAccess(const char *filename) {
  HANDLE fileH = CreateFile(filename,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, // For Exclusive access
                            0,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
  if (fileH != INVALID_HANDLE_VALUE) {
    CloseHandle(fileH);
    return true;
  }
  return false;
}

static void OpenEditor() {
  SHELLEXECUTEINFO shinfo = {0};
  shinfo.hwnd = g_ui_window;
  shinfo.cbSize = sizeof(shinfo);
  shinfo.nShow = SW_SHOWNORMAL;

  if (g_current_filename[0]) {
    if (!HasReadWriteAccess(g_current_filename)) {
      // Need to runas admin
      char buf[1024];
      if (!ExpandEnvironmentStrings("%windir%\\system32\\notepad.exe", buf, sizeof(buf)))
        return;
      shinfo.lpFile = buf;
      char *filename = str_cat_alloc("\"", g_current_filename, "\"");
      shinfo.lpParameters = filename;
      shinfo.lpVerb = "runas";
      ShellExecuteEx(&shinfo);
      free(filename);
    } else {
      shinfo.fMask = SEE_MASK_CLASSNAME;
      shinfo.lpFile = g_current_filename;
      shinfo.lpParameters = "";
      shinfo.lpClass = ".txt";
      ShellExecuteEx(&shinfo);
    }
  }
}

static void BrowseFiles() {
  char buf[MAX_PATH];
  if (ExpandConfigPath("", buf, ARRAYSIZE(buf))) {
    size_t l = strlen(buf);
    buf[l - 1] = 0;
    ShellExecuteFromExplorer(buf, NULL, NULL, "explore");
  }
}

bool ImportFile(const char *s, bool silent = false) {
  char buf[1024];
  char mesg[1024];
  size_t filesize;
  const char *last = FindFilenameComponent(s);
  uint8 *filedata = NULL;
  bool rv = false;
  int filerv;

  if (!*last || !ExpandConfigPath(last, buf, ARRAYSIZE(buf)) || _stricmp(buf, s) == 0)
    goto out;

  filedata = LoadFileSane(s, &filesize);
  if (!filedata)
    goto out;

  if (!silent) {
    if (FileExists(buf)) {
      snprintf(mesg, ARRAYSIZE(mesg), "A file already exists with the name '%s' in the configuration folder. Do you want to overwrite it?", last);
      if (MessageBoxA(g_ui_window, mesg, "TunSafe", MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
        goto out;
    } else {
      snprintf(mesg, ARRAYSIZE(mesg), "Do you want to import '%s' into TunSafe?", last);
      if (MessageBoxA(g_ui_window, mesg, "TunSafe", MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
        goto out;
    }
  }

  filerv = WriteOutFile(buf, filedata, filesize);
  
  // elevate?
  if (filerv == kWriteOutFile_AccessError && g_is_limited_uac_account) {
    char *args = str_cat_alloc("--import \"", s, "\"");
    rv = RunProcessAsAdminWithArgs(args, true);
    free(args);
    return rv;
  }

  rv = (filerv == kWriteOutFile_Ok);
  if (!rv)
    DeleteFileA(buf);

out:
  free(filedata);

  if (!silent) {
    if (rv)
      LoadConfigFile(buf, true, false);
    else
      MessageBoxA(g_ui_window, "There was a problem importing the file.", "TunSafe", MB_ICONEXCLAMATION);
  }
  return !rv;
}

void ShowUI(HWND hWnd) {
  SetUiVisibility(true);
  BringWindowToTop(hWnd);
  SetForegroundWindow(hWnd);
}

void HandleDroppedFiles(HWND wnd, HDROP hdrop) {
  char buf[MAX_PATH];
  if (DragQueryFile(hdrop, -1, NULL, 0) == 1) {
    if (DragQueryFile(hdrop, 0, buf, ARRAYSIZE(buf))) {
      SetForegroundWindow(wnd);
      ImportFile(buf);
    }
  }
  DragFinish(hdrop);
}

void BrowseFile(HWND wnd) {
  char szFile[1024];

  // open a file name
  OPENFILENAME ofn = {0};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = g_ui_window;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "Config Files (*.conf)\0*.conf\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  if (GetOpenFileName(&ofn))
    ImportFile(szFile);
}

static void SetKeyBox(HWND wnd, int ctr, uint8 buf[32]) {
  char base64[WG_PUBLIC_KEY_LEN_BASE64 + 1];
  SetDlgItemText(wnd, ctr, base64_encode(buf, 32, base64, sizeof(base64), NULL));
}

static INT_PTR WINAPI KeyPairDlgProc(HWND hWnd, UINT message, WPARAM wParam,
                              LPARAM lParam) {
  switch (message) {
  case WM_INITDIALOG:
    return TRUE;
  case WM_CLOSE:
    EndDialog(hWnd, 0);
    return TRUE;
  case WM_COMMAND:
    switch (wParam) {
    case IDCANCEL:
      EndDialog(hWnd, 0);
      return TRUE;
    case IDC_PRIVATE_KEY | (EN_CHANGE << 16) : {
      char buf[128];
      uint8 pub[32];
      uint8 priv[32];
      buf[0] = 0;
      size_t len = GetDlgItemText(hWnd, IDC_PRIVATE_KEY, buf, sizeof(buf));
      size_t olen = 32;
      if (base64_decode((uint8*)buf, len, priv, &olen) && olen == 32) {
        curve25519_donna(pub, priv, kCurve25519Basepoint);
        SetKeyBox(hWnd, IDC_PUBLIC_KEY, pub);
      } else {
        SetDlgItemText(hWnd, IDC_PUBLIC_KEY, "(Invalid Private Key)");
      }

      return TRUE;
    }
    case IDRAND: {
      uint8 priv[32];
      uint8 pub[32];
      OsGetRandomBytes(priv, 32);
      curve25519_normalize(priv);
      curve25519_donna(pub, priv, kCurve25519Basepoint);
      SetKeyBox(hWnd, IDC_PRIVATE_KEY, priv);
      SetKeyBox(hWnd, IDC_PUBLIC_KEY, pub);
      return TRUE;
    }
    }
  }
  return FALSE;
}

static void SetStartupFlags(int new_flags) {
  // Determine whether to autorun or not.
  bool autorun = (new_flags & kStartupFlag_MinimizeToTrayWhenWindowsStarts) ||
                 !(new_flags & kStartupFlag_BackgroundService) && (new_flags & kStartupFlag_ConnectWhenWindowsStarts);

  // Update the autorun key.
  HKEY hkey;
  LSTATUS result;
  result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hkey);
  if (result == 0) {
    if (autorun) {
      wchar_t buf[512 + 32];
      buf[0] = '"';
      DWORD len = GetModuleFileNameW(NULL, buf + 1, 512);
      if (len < 512) {
        memcpy(buf + len + 1, L"\" --autostart", sizeof(wchar_t) * 14);
        result = RegSetValueExW(hkey, L"TunSafe", NULL, REG_SZ, (BYTE*)buf, (DWORD)(len + 15) * sizeof(wchar_t));
      } else {
        RERROR("Unable to add to startup list, filename too long.");
      }
    } else {
      RegDeleteValueW(hkey, L"TunSafe");
    }
    RegCloseKey(hkey);
  }
  if (result != 0)
    RERROR("Unable to modify startup list, error code = 0x%x", (int)result);

  RegWriteInt(g_reg_key, "StartupFlags", new_flags);

  bool was_started = g_backend && g_backend->is_started();
  bool recreate_backend = false;

  if (!!(new_flags & (kStartupFlag_BackgroundService | kStartupFlag_ForegroundService))) {
    // Want to run as a service - make sure service is installed and running.
    if (!IsTunsafeServiceRunning()) {
      g_backend->Stop();
      RINFO("Starting TunSafe service...");
      InstallTunSafeWindowsService();
      recreate_backend = true;
    }
} else {
    if (IsTunSafeServiceInstalled()) {
      g_backend->Stop();
      g_backend->Teardown();

      RINFO("Removing TunSafe service...");
      // Don't want to run as a service - Make sure we delete the service.
      if (g_is_limited_uac_account) {
        // Need to stop this early so service process is able to open.
        CloseHandle(g_runonce_mutex);
        if (!RunProcessAsAdminWithArgs("--delete-service-and-start", false)) {
          RINFO("Unable to stop and remove service");
          uint32 m = kStartupFlag_BackgroundService | kStartupFlag_ForegroundService;
          new_flags = (g_startup_flags & m) | (new_flags & ~m);
        } else {
          PostQuitMessage(0);
          return;
        }
      } else {
        if (!UninstallTunSafeWindowsService()) {
          RINFO("Unable to stop and remove service");
          uint32 m = kStartupFlag_BackgroundService | kStartupFlag_ForegroundService;
          new_flags = (g_startup_flags & m) | (new_flags & ~m);
        }
      }
      recreate_backend = true;
    }
  }
  if (recreate_backend) {
    CreateLocalOrRemoteBackend(!!(new_flags & (kStartupFlag_BackgroundService | kStartupFlag_ForegroundService)));
    if (was_started)
      StartTunsafeBackend(UIW_START);
  }
  g_startup_flags = new_flags;
  g_backend->SetServiceStartupFlags(g_startup_flags);
}

enum {
  kTab_Logs = 0,
  kTab_Charts = 1,
  kTab_Advanced = 2,
};

static void UpdateGraphReq() {
  if (g_backend && (g_current_tab != 1 || !g_ui_visible)) {
    free(g_backend->GetGraph(-1));
  }
}

static void UpdateTabSelection() {
  int tab = TabCtrl_GetCurSel(hwndTab);
  HWND wnd = g_ui_window;
  g_current_tab = tab;
  ShowWindow(hwndEdit, (tab == kTab_Logs) ? SW_SHOW : SW_HIDE);
  ShowWindow(hwndGraphBox, (tab == kTab_Charts) ? SW_SHOW : SW_HIDE);
  ShowWindow(hwndAdvancedBox, (tab == kTab_Advanced) ? SW_SHOW : SW_HIDE);
  UpdateGraphReq();
}

struct WindowSizingItem {
  uint16 id;
  uint16 edges;
};

enum {
  WSI_LEFT = 1,
  WSI_RIGHT = 2,
  WSI_TOP = 4,
  WSI_BOTTOM = 8,
};

static const WindowSizingItem kWindowSizing[] = {
  {ID_START,WSI_LEFT | WSI_RIGHT},
  {ID_BTN_KILLSWITCH, WSI_LEFT | WSI_RIGHT},
  {ID_STOP,WSI_LEFT | WSI_RIGHT},
  {ID_EDITCONF,WSI_LEFT | WSI_RIGHT},
  {IDC_PAINTBOX,WSI_RIGHT},
  {IDC_TAB, WSI_RIGHT | WSI_BOTTOM},
};

static void HandleWindowSizing() {
  RECT wr;

  GetClientRect(g_ui_window, &wr);

  static int g_orig_w, g_orig_h;
  static RECT g_orig_rects[ARRAYSIZE(kWindowSizing)];

  if (g_orig_w == 0) {
    g_orig_w = wr.right;
    g_orig_h = wr.bottom;
    for (size_t i = 0; i < ARRAYSIZE(kWindowSizing); i++) {
      const WindowSizingItem *it = &kWindowSizing[i];
      g_orig_rects[i] = GetParentRect(GetDlgItem(g_ui_window, it->id));
    }
  }

  int dx = wr.right - g_orig_w;
  int dy = wr.bottom - g_orig_h;

  if (dx|dy) {
    HDWP dwp = BeginDeferWindowPos(10), dwp_next;
    for (size_t i = 0; i < ARRAYSIZE(kWindowSizing); i++) {
      const WindowSizingItem *it = &kWindowSizing[i];
      HWND wnd = GetDlgItem(g_ui_window, it->id);
      RECT r = g_orig_rects[i];
      if (it->edges & WSI_LEFT) r.left += dx;
      if (it->edges & WSI_RIGHT) r.right += dx;
      if (it->edges & WSI_TOP) r.top += dy;
      if (it->edges & WSI_BOTTOM) r.bottom += dy;
      if (r.right < r.left) r.right = r.left;
      if (r.bottom < r.top) r.bottom = r.top;
      dwp_next = DeferWindowPos(dwp, wnd, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOREPOSITION | SWP_NOACTIVATE);
      dwp = dwp_next ? dwp_next : dwp;
    }
    EndDeferWindowPos(dwp);
  }

  RECT rect = GetParentRect(hwndTab);
  TabCtrl_AdjustRect(hwndTab, false, &rect);
  MoveWindow(hwndEdit, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
  MoveWindow(hwndGraphBox, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
  MoveWindow(hwndAdvancedBox, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

  int parts[3] = {
    (int)(wr.right * 0.2f),
    (int)(wr.right * 0.6f),
    (int)-1,
  };

  SendMessage(hwndStatus, SB_SETPARTS, 3, (LPARAM)parts);
  SendMessage(hwndStatus, WM_SIZE, 0, 0);
  InvalidateRect(hwndStatus, NULL, TRUE);
}

static void HandleClickedItem(HWND hWnd, int wParam) {
  if (wParam >= ID_POPUP_CONFIG_FILE && wParam < ID_POPUP_CONFIG_FILE + MAX_CONFIG_FILES) {
    const char *new_conf = config_filenames[wParam - ID_POPUP_CONFIG_FILE];
    if (!new_conf)
      return;

    if (strcmp(new_conf, g_current_filename) == 0 && g_backend->is_started()) {
      StopTunsafeBackend(UIW_NONE);
    } else {
      LoadConfigFile(new_conf, true, GetAsyncKeyState(VK_SHIFT) >= 0);
    }

    return;
  }
  switch (wParam) {
  case ID_START: StartTunsafeBackend(UIW_START); break;
  case ID_STOP:  StopTunsafeBackend(UIW_NONE); break;
  case ID_EXIT:  PostQuitMessage(0); break;
  case ID_MORE_BUTTON: ShowSettingsMenu(hWnd); break;
  case IDSETT_WEB_PAGE: ShellExecute(g_ui_window, NULL, "https://tunsafe.com/", NULL, NULL, 0); break;
  case IDSETT_OPENSOURCE: ShellExecute(g_ui_window, NULL, "https://tunsafe.com/open-source", NULL, NULL, 0); break;
  case ID_EDITCONF: OpenEditor(); break;
  case IDSETT_BROWSE_FILES:BrowseFiles(); break;
  case IDSETT_OPEN_FILE: BrowseFile(hWnd); break;
  case IDSETT_ABOUT:
    MessageBoxA(g_ui_window, TUNSAFE_VERSION_STRING_LONG "\r\n\r\nCopyright © 2018, Ludvig Strigeus\r\n\r\nThanks for choosing TunSafe!\r\n\r\nThis version was built on " __DATE__ " " __TIME__, "About TunSafe", MB_ICONINFORMATION);
    break;
  case IDSETT_KEYPAIR:
    DialogBox(g_hinstance, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, &KeyPairDlgProc);
    break;
  case IDSETT_BLOCKINTERNET_OFF:
  case IDSETT_BLOCKINTERNET_ROUTE:
  case IDSETT_BLOCKINTERNET_FIREWALL:
  case IDSETT_BLOCKINTERNET_BOTH:
  {
    uint32 old_state = g_backend->GetInternetBlockState();
    uint32 new_state = wParam - IDSETT_BLOCKINTERNET_OFF;

    if ((old_state & kBlockInternet_TypeMask) == kBlockInternet_Off && new_state != kBlockInternet_Off) {
      if (MessageBoxA(g_ui_window, "Warning! All Internet traffic will be blocked while TunSafe is active. Only traffic through TunSafe will be allowed.\r\n\r\nThe blocking is activated the next time TunSafe connects.\r\n\r\nDo you want to continue?", "TunSafe", MB_ICONWARNING | MB_OKCANCEL) == IDCANCEL)
        return;
    }
    g_backend->SetInternetBlockState((InternetBlockState)(new_state | (old_state & ~kBlockInternet_TypeMask)));

    if ((~old_state & new_state) && g_backend->is_started())
      StartTunsafeBackend(UIW_START);
    return;
  }
  case IDSETT_BLOCKINTERNET_DISCONN: {
    g_backend->SetInternetBlockState((InternetBlockState)(g_backend->GetInternetBlockState() ^ kBlockInternet_BlockOnDisconnect));
    return;
  }

  case IDSETT_BLOCKINTERNET_ALLOWLOCAL: {
    g_backend->SetInternetBlockState((InternetBlockState)(g_backend->GetInternetBlockState() ^ kBlockInternet_AllowLocalNetworks));
    if (g_backend->is_started())
      StartTunsafeBackend(UIW_START);
    return;
  }

  case ID_BTN_KILLSWITCH: {
    g_backend->SetInternetBlockState((InternetBlockState)(g_backend->GetInternetBlockState() & ~kBlockInternet_Active));
    return;
  }

  case IDSETT_SERVICE_OFF:
  case IDSETT_SERVICE_FOREGROUND:
  case IDSETT_SERVICE_BACKGROUND:
    SetStartupFlags((int)((g_startup_flags & ~3) + wParam - IDSETT_SERVICE_OFF));
    break;
  case IDSETT_SERVICE_CONNECT_AUTO:
    SetStartupFlags(g_startup_flags ^ kStartupFlag_ConnectWhenWindowsStarts);
    break;
  case IDSETT_SERVICE_MINIMIZE_AUTO:
    SetStartupFlags(g_startup_flags ^ kStartupFlag_MinimizeToTrayWhenWindowsStarts);
    break;

  case IDSETT_PREPOST:
  {
    if (!g_hklm_reg_key) {
      if (!RunProcessAsAdminWithArgs(g_allow_pre_post ? "--set-allow-pre-post 0" : "--set-allow-pre-post 1", true))
        MessageBox(g_ui_window, "You need to run TunSafe as an Administrator to be able to change this setting.", "TunSafe", MB_ICONWARNING);
      g_allow_pre_post = RegReadInt(g_hklm_readonly_reg_key, "AllowPrePost", 0) != 0;
      return;
    }
    g_allow_pre_post = !g_allow_pre_post;
    RegWriteInt(g_hklm_reg_key, "AllowPrePost", g_allow_pre_post);
    return;
  }
  }
}

static INT_PTR WINAPI DlgProc(HWND hWnd, UINT message, WPARAM wParam,
                                LPARAM lParam) {

  switch (message) {
  case WM_INITDIALOG:
    SetMenu(hWnd, LoadMenu(g_hinstance, MAKEINTRESOURCE(IDR_MENU1)));
    return TRUE;
  case WM_CLOSE:
    SetUiVisibility(false);
    return TRUE;
  case WM_NOTIFY: {
    UINT idFrom = (UINT)((NMHDR*)lParam)->idFrom;
    switch (((NMHDR*)lParam)->code) {
    case TCN_SELCHANGE:
      switch (idFrom) {
      case IDC_TAB:
        UpdateTabSelection();
        return TRUE;
      }

      break;
    }
    break;
  }
  case WM_COMMAND:
    switch (HIWORD(wParam)) {
    case 0:
      HandleClickedItem(hWnd, (int)wParam);
      break;
    }
    break;
  case WM_DROPFILES:
    if (!wm_dropfiles_recursive) {
      wm_dropfiles_recursive = true;
      HandleDroppedFiles(hWnd, (HDROP)wParam);
      wm_dropfiles_recursive = false;
    }
    break;
  case WM_USER + 1:
    if (lParam == WM_RBUTTONUP) {
      HMENU menu = CreatePopupMenu();
      if (AddToAvailableFilesPopup(menu, 10, false))
        AppendMenu(menu, MF_SEPARATOR, 0, 0);

      bool active = g_backend->is_started();
      AppendMenu(menu, 0, ID_START, active ? "Re&connect" : "&Connect");
      AppendMenu(menu, active ? 0 : MF_GRAYED, ID_STOP, "&Disconnect");
      AppendMenu(menu, MF_SEPARATOR, 0, NULL);
      AppendMenu(menu, 0, ID_EXIT, "&Exit");
      POINT pt;
      GetCursorPos(&pt);

      SetForegroundWindow(hWnd);

      int rv = TrackPopupMenu(menu, 0, pt.x, pt.y, 0, hWnd, NULL);
      DestroyMenu(menu);
    } else if (lParam == WM_LBUTTONDBLCLK) {
      if (IsWindowVisible(hWnd)) {
        SetUiVisibility(false);
      } else {
        ShowUI(hWnd);
      }
    }
    return TRUE;
  case WM_USER + 2:
    g_backend_delegate->DoWork();
    return true;

  case WM_USER + 3:
    ShowTwoFactorDialog();
    return true;

  case WM_INITMENU: {
    HMENU menu = GetMenu(g_ui_window);

    CheckMenuItem(menu, IDSETT_SERVICE_CONNECT_AUTO, MF_CHECKED * !!(g_startup_flags & kStartupFlag_ConnectWhenWindowsStarts));
    CheckMenuItem(menu, IDSETT_SERVICE_MINIMIZE_AUTO, MF_CHECKED * !!(g_startup_flags & kStartupFlag_MinimizeToTrayWhenWindowsStarts));
    CheckMenuItem(menu, IDSETT_PREPOST, g_allow_pre_post ? MF_CHECKED : 0);

    int value = g_backend->GetInternetBlockState();
    CheckMenuRadioItem(menu, IDSETT_BLOCKINTERNET_OFF, IDSETT_BLOCKINTERNET_BOTH, IDSETT_BLOCKINTERNET_OFF + (value & kBlockInternet_Both), MF_BYCOMMAND);
    CheckMenuRadioItem(menu, IDSETT_SERVICE_OFF, IDSETT_SERVICE_BACKGROUND, IDSETT_SERVICE_OFF + (g_startup_flags & 3), MF_BYCOMMAND);
    CheckMenuItem(menu, IDSETT_BLOCKINTERNET_DISCONN, (value & kBlockInternet_BlockOnDisconnect) ? MF_CHECKED : 0);
    CheckMenuItem(menu, IDSETT_BLOCKINTERNET_ALLOWLOCAL, (value & kBlockInternet_AllowLocalNetworks) ? MF_CHECKED : 0);
    break;
  }

  case WM_SIZE:
    if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED) {
      if (g_ui_window)
        HandleWindowSizing();
    }
    break;

  case WM_EXITMENULOOP:
    g_timestamp_of_exit_menuloop = GetTickCount();
    break;

  default:
    if (message == g_message_taskbar_created) {
      g_has_icon = false;
      UpdateIcon(UIW_NONE);
    }
    break;
  }
  return FALSE;
}

void PushLine(int type, const char *s) {
  size_t l = strlen(s);
  char buf[64];
  SYSTEMTIME t;

  GetLocalTime(&t);

  snprintf(buf, sizeof(buf), "[%.2d:%.2d:%.2d] ", t.wHour, t.wMinute, t.wSecond);
  size_t tl = strlen(buf);

  char *x = (char*)malloc(tl + l + 1);
  if (!x) return;
  memcpy(x, buf, tl);
  memcpy(x + tl, s, l);
  x[l + tl] = '\0';
  g_backend_delegate->OnLogLine((const char**)&x);
  free(x);
}

void EnsureConfigDirCreated() {
  char fullname[1024];
  if (ExpandConfigPath("", fullname, sizeof(fullname)))
    CreateDirectory(fullname, NULL);
}

void EnableControl(int wnd, bool b) {
  EnableWindow(GetDlgItem(g_ui_window, wnd), b);
}

LRESULT CALLBACK NotifyWndProc(HWND  hwnd, UINT  uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_USER + 10:
    if (wParam == 1) {
      PostQuitMessage(0);
      return 31337;
    } else if (wParam == 0) {
      ShowUI(g_ui_window);
      return 31337;
    }
    break;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateNotificationWindow() {
  WNDCLASSEX wce = {0};
  wce.cbSize = sizeof(wce);
  wce.lpfnWndProc = &NotifyWndProc;
  wce.hInstance = g_hinstance;
  wce.lpszClassName = "TunSafe-f19e092db01cbe0fb6aee132f8231e5b71c98f90";
  RegisterClassEx(&wce);
  CreateWindow("TunSafe-f19e092db01cbe0fb6aee132f8231e5b71c98f90", "TunSafe-f19e092db01cbe0fb6aee132f8231e5b71c98f90", 0, 0, 0, 0, 0, 0, 0, g_hinstance, NULL);
}

HFONT CreateBoldUiFont() {
  LOGFONT lf;
  HFONT ffont = (HFONT)SendMessage(g_ui_window, WM_GETFONT, 0, 0);
  GetObject(ffont, sizeof(lf), &lf);
  lf.lfWeight = FW_BOLD;
  HFONT font = CreateFontIndirect(&lf);
  return font;
}

void FillRectColor(HDC dc, const RECT &r, COLORREF color) {
  COLORREF old = ::SetBkColor(dc, color);
  ExtTextOut(dc, 0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);
  ::SetBkColor(dc, old);
}

void DrawRectOutline(HDC dc, const RECT &r) {
  POINT points[5] = {
    {r.left, r.top},
    {r.right, r.top},
    {r.right, r.bottom},
    {r.left, r.bottom},
    {r.left, r.top}
  };
  Polyline(dc, points, 5);
}

static HFONT CreateFontHelper(int size, byte flags, const char *face, int angle = 0) {
  return CreateFontA(-RescaleDpi(size), 0, angle, angle, flags & 1 ? FW_BOLD : 0, FALSE, flags & 2 ? 1 : 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, face);
}

static const char *StatusCodeToString(TunsafeBackend::StatusCode code) {
  switch (code) {
  case TunsafeBackend::kErrorInitialize:    return "Configuration Error";
  case TunsafeBackend::kErrorServiceLost:   return "Service Lost";
  case TunsafeBackend::kStatusStopped:      return "Disconnected";
  case TunsafeBackend::kStatusInitializing: return "Initializing";
  case TunsafeBackend::kStatusConnecting:   return "Connecting...";
  case TunsafeBackend::kStatusReconnecting: return "Reconnecting...";
  case TunsafeBackend::kStatusConnected:    return "Connected";
  case TunsafeBackend::kStatusTunRetrying:  return "TUN Adapter Error, retrying...";
  default:
    return "Unknown";
  }
}

static void DrawInPaintBox(HDC hdc, int w, int h) {
  RECT rect = {0, 0, w, h};
  FillRect(hdc, &rect, (HBRUSH)(COLOR_3DFACE + 1));
  
  HFONT font = CreateBoldUiFont();

  char namebuf[128];
  GetCurrentConfigTitle(namebuf, sizeof(namebuf));

  RECT btrect = GetParentRect(GetDlgItem(g_ui_window, ID_START));

  HPEN pen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
  HBRUSH brush = GetSysColorBrush(COLOR_WINDOW);

  SelectObject(hdc, pen);
  SelectObject(hdc, brush);

  comborect = MakeRect(0, btrect.top + 1, w, btrect.bottom - 1);
  Rectangle(hdc, 0, btrect.top + 1, w, btrect.bottom - 1);

  if (arrowbitmap == NULL)
    arrowbitmap = LoadBitmap(g_hinstance, MAKEINTRESOURCE(IDB_DOWNARROW));

  int bw = RescaleDpi(6);

  HDC memdc = CreateCompatibleDC(hdc);
  SelectObject(memdc, arrowbitmap);
  StretchBlt(hdc, w - 1 - bw - 5, btrect.top + 1 + ((btrect.bottom - btrect.top - bw) >> 1), 
    bw, bw, memdc, 0, 0, 6, 6, SRCCOPY);

  int th = RescaleDpi(20);

  SelectObject(hdc, font);
  SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
  TextOut(hdc, RescaleDpi(4), btrect.top + RescaleDpi(4), namebuf, (int)strlen(namebuf));

  int y = btrect.bottom + RescaleDpi(4);

  DeleteObject(pen);

  SelectObject(hdc, (HFONT)SendMessage(g_ui_window, WM_GETFONT, 0, 0));
  SetBkColor(hdc, GetSysColor(COLOR_3DFACE));

  TunsafeBackend::StatusCode status = g_backend->status();
  my_strlcpy(namebuf, sizeof(namebuf) - 32, StatusCodeToString(status));
  if (status == TunsafeBackend::kStatusConnected || status == TunsafeBackend::kStatusReconnecting) {
    uint64 when = g_processor_stats.first_complete_handshake_timestamp;
    uint32 seconds = (when != 0) ? (uint32)((OsGetMilliseconds() - when + 500) / 1000) : 0;
    snprintf(strchr(namebuf, 0), 32, ", %.2d:%.2d:%.2d", seconds / 3600, (seconds / 60) % 60, seconds % 60);
  }
 
  int img = (status == TunsafeBackend::kStatusConnected) ? 0 :
            g_backend->is_started() && !TunsafeBackend::IsPermanentError(status) ? 1 : 2;

  static const COLORREF kDotColors[3] = {
    0x51a600,
    0x00c0c0,
    0x0000c0,
  };
  SetBkMode(hdc, TRANSPARENT);
  COLORREF oldcolor = SetTextColor(hdc, kDotColors[img]);
  HFONT oldfont = (HFONT)SelectObject(hdc, CreateFontHelper(18, 0, "Tahoma"));
  wchar_t bullet = 0x25CF;
  TextOutW(hdc, RescaleDpi(2), y - RescaleDpi(7), &bullet, 1);
  DeleteObject(SelectObject(hdc, oldfont));
  SetTextColor(hdc, oldcolor);

  TextOut(hdc, RescaleDpi(2 + 14), y, namebuf, (int)strlen(namebuf));

  y += RescaleDpi(18);

  uint32 ip = g_backend->GetIP();
  if (ip) {
    print_ip(namebuf, ip);
    TextOut(hdc, 2, y, namebuf, (int)strlen(namebuf));
  } 
  DeleteObject(font);
  DeleteDC(memdc);
}

typedef void DrawInPaintBoxFunc(HDC dc, int w, int h);
static void HandleWmPaintPaintbox(HWND hwnd, DrawInPaintBoxFunc *func) {
  PAINTSTRUCT ps;
  BeginPaint(hwnd, &ps);

  RECT r;
  GetClientRect(hwnd, &r);

  HBITMAP bmp = CreateCompatibleBitmap(ps.hdc, r.right, r.bottom);
  HDC dc = CreateCompatibleDC(ps.hdc);
  SelectObject(dc, bmp);

  func(dc, r.right, r.bottom);

  BitBlt(ps.hdc, 0, 0, r.right, r.bottom, dc, 0, 0, SRCCOPY);
  DeleteDC(dc);
  DeleteObject(bmp);
  EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK PaintBoxWndProc(HWND  hwnd, UINT  uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_PAINT: {
    HandleWmPaintPaintbox(hwnd, &DrawInPaintBox);
    return TRUE;
  }
  case WM_LBUTTONDOWN: {
    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    if (PtInRect(&comborect, pt)) {
      // Avoid showing the menu again if clicking to close.
      if (GetTickCount() - g_timestamp_of_exit_menuloop >= 50u)
        ShowSettingsMenu(g_ui_window);
    }
    return TRUE;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void DrawGraph(HDC dc, const RECT *rr, StatsCollector::TimeSeries **sources, const COLORREF *colors, int num_source, const char *xcaption, const char *ycaption) {
  RECT r = *rr;
  FillRectColor(dc, r, 0xffffff);

  RECT margins = { 30, 10, -10, -15 };
  margins = RescaleDpiRect(margins);

  r.left += margins.left;
  r.top += margins.top;
  r.right += margins.right;
  r.bottom += margins.bottom;

  HPEN borderpen = CreatePen(PS_SOLID, 1, 0x808080);
  SelectObject(dc, borderpen);
  DrawRectOutline(dc, r);

  static const uint8 bits[4] = {0x70, 0, 0, 0};
  HBITMAP bmp = CreateBitmap(4, 1, 1, 1, &bits);
  HBRUSH brush = CreatePatternBrush(bmp);
  DeleteObject(bmp);

  // Draw horizontal dotted lines
  {
    SetTextColor(dc, 0x808080);
    SetBkColor(dc, 0xffffff);
    int inc = (r.bottom - r.top) >> 2;
    RECT r2 = {r.left + 1, r.top + inc * 1, r.right - 1, r.top + inc * 1 + 1};
    FillRect(dc, &r2, brush);
    r2.top += inc; r2.bottom += inc;
    FillRect(dc, &r2, brush);
    r2.top += inc; r2.bottom += inc;
    FillRect(dc, &r2, brush);
  }
  DeleteObject(brush);

  static const uint8 bits_vertical[16] = {
    0xff, 0x0, 0xff, 0,
    0xff, 0x0, 0x0, 0,
    0xff, 0x0, 0x0, 0,
    0x0, 0x0, 0x0, 0};
  bmp = CreateBitmap(1, 4, 1, 1, &bits_vertical);
  brush = CreatePatternBrush(bmp);
  DeleteObject(bmp);

  {
    // Draw vertical dotted lines 
    for (int i = 1; i < 12; i++) {
      int x = (r.right - r.left) * i / 12;
      RECT r2 = {r.left + x, r.top + 1, r.left + x + 1, r.bottom - 1};
      FillRect(dc, &r2, brush);
    }
  }

  {
    // Draw legend text
    HFONT font = CreateFontHelper(10, 0, "Tahoma");
    SelectObject(dc, font);
    SetTextColor(dc, 0x202020);
    SetBkMode(dc, TRANSPARENT);
    RECT r2 = {r.left + 1, r.bottom, r.right - 1, r.bottom + RescaleDpi(15)};
    DrawText(dc, xcaption, (int)strlen(xcaption), &r2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    DeleteObject(font);
  }
  DeleteObject(brush);
  DeleteObject(borderpen);

  // Determine the scaling factor
  float mx = 1;
  for (size_t j = 0; j != num_source; j++) {
    const StatsCollector::TimeSeries *src = sources[j];
    for (size_t i = 0; i != src->size; i++)
      mx = std::max(mx, src->data[i]);
  }
  int topval = (int)(mx + 0.5f);
  // round it appropriately
  if (topval >= 500)
    topval = (topval + 99) / 100 * 100;
  else if (topval >= 200)
    topval = (topval + 49) / 50 * 50;
  else if (topval >= 50)
    topval = (topval + 9) / 10 * 10;
  else if (topval >= 20)
    topval = (topval + 4) / 5 * 5;
  if (topval > mx)
    mx = (float)topval;

  {
    RECT r2 = {r.left - RescaleDpi(30), r.top - RescaleDpi(2), r.left - RescaleDpi(2), r.bottom};
    char buf[30];
    sprintf(buf, "%d", topval);
    DrawText(dc, buf, (int)strlen(buf), &r2, DT_RIGHT | DT_SINGLELINE);
    r2.top = r.bottom - RescaleDpi(12);
    DrawText(dc, "0", 1, &r2, DT_RIGHT | DT_SINGLELINE);
  }

  float mx_f = (1.0f / mx) * (r.bottom - r.top);

  for (size_t k = 0; k != num_source; k++) {
    HPEN borderpen = CreatePen(PS_SOLID, 2, colors[k]);
    SelectObject(dc, borderpen);
    const StatsCollector::TimeSeries *src = sources[k];
    POINT *points = new POINT[src->size];
    for (size_t i = 0, j = src->shift; i != src->size; i++) {
      points[i].x = (int)(r.left + (r.right - r.left) * i / (src->size - 1));
      points[i].y = r.bottom - (int)((float)src->data[j] * mx_f);
      if (++j == src->size) j = 0;
    }
    Polyline(dc, points, src->size);
    delete points;
    DeleteObject(borderpen);
  }

  if (ycaption != NULL) {
    HFONT font = CreateFontHelper(10, 0, "Tahoma", 900);
    SelectObject(dc, font);
    TextOut(dc, r.left - RescaleDpi(18), ((r.top + r.bottom) >> 1) + RescaleDpi(12), ycaption, (int)strlen(ycaption));
    DeleteObject(font);
  }
}

static const char * const kGraphStepNames[] = {
  "1 second step",
  "5 second step",
  "30 second step",
  "5 minute step",
};

static void DrawInGraphBox(HDC hdc, int w, int h) {
  RECT r = {0, 0, w, h};
  
  static const COLORREF color[4] = {
    0x00c000,
    0xc00000,
  };
  
  LinearizedGraph *graph = g_backend->GetGraph(g_selected_graph_type);
  StatsCollector::TimeSeries *time_series_ptr[4];
  StatsCollector::TimeSeries time_series[4];

  int num_charts = 0;
  if (graph && graph->num_charts <= 4) {
    uint8 *ptr = (uint8*)(graph + 1);
    for (int i = 0; i < graph->num_charts; i++) {
      time_series_ptr[i] = &time_series[i];
      time_series[i].shift = 0;

      uint32 size = *(uint32*)ptr;
      time_series[i].size = size;
      time_series[i].data = (float*)(ptr + 4);
      if ((ptr - (uint8*)graph) + 4 + (uint64)size * 4 > graph->total_size)
        break;
      ptr += 4 + size * 4;
    }
    num_charts = graph->num_charts;
  }

  char buf[256];
  snprintf(buf, sizeof(buf), "Time (%s)", kGraphStepNames[g_selected_graph_type]);

  DrawGraph(hdc, &r, time_series_ptr, color, num_charts, buf, "Mbps");

  free(graph);
}

static LRESULT CALLBACK GraphBoxWndProc(HWND  hwnd, UINT  uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_PAINT: {
    HandleWmPaintPaintbox(hwnd, &DrawInGraphBox);
    return TRUE;
  }
  case WM_RBUTTONDOWN: {
    HMENU menu = CreatePopupMenu();
    for(int i = 0; i < ARRAYSIZE(kGraphStepNames); i++)
      AppendMenu(menu, (i == g_selected_graph_type) * MF_CHECKED, i + 1, kGraphStepNames[i]);
    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ClientToScreen(hwnd, &pt);
    int rv = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(menu);
    if (rv != 0) {
      g_selected_graph_type = rv - 1;
      InvalidateRect(hwnd, NULL, FALSE);
    }
    return TRUE;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

struct AdvancedTextInfo {
  uint16 y;
  uint8 indent;
  const char *title;
};

static const AdvancedTextInfo ADVANCED_TEXT_INFOS[] = {
#define Y 26
  {Y + 19 * 0, 66, "Public Key:"},
  {Y + 19 * 1, 66, "Endpoint:"},
  {Y + 19 * 2, 66, "Transfer:"},
  {Y + 19 * 3, 66, "Handshake:"},
  {Y + 19 * 4, 66, ""},
  {Y + 19 * 5, 66, "Overhead:"},
  {Y + 19 * 6, 66, "Packet Loss:"},
  {Y + 19 * 7, 66, "Invalid:"},
#undef Y
};

static char *PrintLastHandshakeAt(char buf[256], WgProcessorStats *ps) {
  char *d = buf;
  if (ps->last_complete_handshake_timestamp) {
    uint32 ago = (uint32)((OsGetMilliseconds() - ps->last_complete_handshake_timestamp + 500) / 1000);
    uint32 hours = ago / 3600;
    uint32 minutes = (ago - hours * 3600) / 60;
    uint32 seconds = (ago - hours * 3600 - minutes * 60);
    if (hours)
      d += snprintf(d, 32, hours == 1 ? "%d hour, " : "%d hours, ", hours);
    if (minutes)
      d += snprintf(d, 32, minutes == 1 ? "%d minute, " : "%d minutes, ", minutes);
    if (d == buf || seconds)
      d += snprintf(d, 32, seconds == 1 ? "%d second, " : "%d seconds, ", seconds);
    memcpy(d - 2, " ago", 5);
  } else {
    memcpy(buf, "(never)", 8);
  }
  return buf;
}

static const char *GetAdvancedInfoValue(char buffer[256], int i) {
  char tmp[64], tmp2[64];
  WgProcessorStats *ps = &g_processor_stats;
  switch (i) {
  case 0: {
    if (IsOnlyZeros(g_backend->public_key(), 32))
      return "";
    base64_encode(g_backend->public_key(), 32, buffer, 256, NULL);
    return buffer;
  }
  case 1: {
    char ip[kSizeOfAddress];
    if (ps->endpoint.sin.sin_family == 0)
      return "";
    char *p = buffer;

    if (ps->endpoint_protocol == kPacketProtocolTcp) {
      memcpy(p, "tcp://", 6);
      p += 6;
    }

    PrintIpAddr(ps->endpoint, ip);
    if (ps->endpoint.sin.sin_family == AF_INET6) {
      snprintf(p, 256-16, "[%s]:%d", ip, htons(ps->endpoint.sin.sin_port));
    } else {
      snprintf(p, 256-16, "%s:%d", ip, htons(ps->endpoint.sin.sin_port));
    }
    return buffer;
  }
    
  case 2: 
    snprintf(buffer, 256, "%s in (%lld packets), %s out (%lld packets)",
             PrintMB(tmp, ps->total_bytes_in), ps->packets_in,
             PrintMB(tmp2, ps->total_bytes_out), ps->packets_out/*, udp_qsize2 - udp_qsize1, g_tun_reads*/);
    return buffer;
  case 3: return PrintLastHandshakeAt(buffer, ps);
  case 4: {
    snprintf(buffer, 256, "%d handshakes in (%d failed), %d handshakes out (%d failed)",
             ps->handshakes_in, ps->handshakes_in - ps->handshakes_in_success,
             ps->handshakes_out, ps->handshakes_out - ps->handshakes_out_success);
    return buffer;
  }
  case 5: {
    uint64 overhead_in = ps->total_bytes_in + ps->packets_in * 40 - ps->data_bytes_in;
    uint32 overhead_in_pct = ps->data_bytes_in ? (uint32)(overhead_in * 100000 / ps->data_bytes_in) : 0;

    uint64 overhead_out = ps->total_bytes_out + ps->packets_out * 40 - ps->data_bytes_out;
    uint32 overhead_out_pct = ps->data_bytes_out ? (uint32)(overhead_out * 100000 / ps->data_bytes_out) : 0;

    snprintf(buffer, 256, "%d.%.3d%% in, %d.%.3d%% out", overhead_in_pct / 1000, overhead_in_pct % 1000,
             overhead_out_pct / 1000, overhead_out_pct % 1000);
    return buffer;
  }
  case 6: {
    snprintf(buffer, 256, "%.3f%% (%d packets)", 
             ps->lost_packets_tot ? 100.0f * (ps->lost_packets_tot - ps->lost_packets_valid) / ps->lost_packets_tot : 0.0f,
             (int)(ps->lost_packets_tot - ps->lost_packets_valid));
    return buffer;
  }
  case 7: {
    snprintf(buffer, 256, "%s in (%lld packets)", PrintMB(tmp, ps->invalid_bytes_in), ps->invalid_packets_in);
    return buffer;
  }
  default: return "";
  }  
}

static void DrawInAdvancedBox(HDC dc, int w, int h) {
  RECT r = {0, 0, w, h};

  FillRectColor(dc, r, 0xffffff);

  SelectObject(dc, (HFONT)SendMessage(g_ui_window, WM_GETFONT, 0, 0));
  SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));
  SetBkColor(dc, GetSysColor(COLOR_WINDOW));

  const AdvancedTextInfo *tp = ADVANCED_TEXT_INFOS;
  char buffer[256];

  for (size_t i = 0; i != ARRAYSIZE(ADVANCED_TEXT_INFOS); i++, tp++) {
    int x = 8;

    RECT r = {x, tp->y, x + tp->indent, tp->y + 19};
    r = RescaleDpiRect(r);
    ::ExtTextOut(dc, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, &r, tp->title, (UINT)strlen(tp->title), NULL);

    const char *s = GetAdvancedInfoValue(buffer, (int)i);
    r.left = r.right;
    r.right = w;
    ::ExtTextOut(dc, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, &r, s, (UINT)strlen(s), NULL);
  }

  SetBkColor(dc, GetSysColor(COLOR_3DFACE));

  static const int grouptop[1] = { 
    2
  };
  static const char *grouptext[1] = {
    "General",
  };

  HFONT font = CreateFontHelper(12, 1, "Tahoma");
  SelectObject(dc, font);
  for (size_t i = 0; i != ARRAYSIZE(grouptext); i++) {
    RECT r = {RescaleDpi(4), RescaleDpi(grouptop[i]), w - RescaleDpi(4), RescaleDpi(grouptop[i] + 18)};
    ::ExtTextOut(dc, RescaleDpi(8), r.top + 1, ETO_CLIPPED | ETO_OPAQUE, &r, grouptext[i], (UINT)strlen(grouptext[i]), NULL);
  }
  DeleteFont(font);
}

static LRESULT CALLBACK AdvancedBoxWndProc(HWND  hwnd, UINT  uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_PAINT: {
    HandleWmPaintPaintbox(hwnd, &DrawInAdvancedBox);
    return TRUE;
  }
  case WM_ERASEBKGND:
    return TRUE;

  case WM_RBUTTONDOWN: {
    int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
    char buffer[256];

    const AdvancedTextInfo *tp = ADVANCED_TEXT_INFOS;
    for (size_t i = 0; i != ARRAYSIZE(ADVANCED_TEXT_INFOS); i++, tp++) {
      if (x >= RescaleDpi(tp->indent) && y >= RescaleDpi(tp->y) && y < RescaleDpi(tp->y + 19)) {
        HMENU menu = CreatePopupMenu();
        AppendMenu(menu, 0, 1, "Copy");
        POINT pt = {x, y};
        ClientToScreen(hwnd, &pt);
        int rv = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(menu);
        if (rv == 1)
          SetClipboardString(GetAdvancedInfoValue(buffer, (int)i));
        return TRUE;
      }
    }
    return TRUE;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static char twofactordig[9];
static uint8 twofactornum, twofactormax;

static void DrawInTwoFactorBox(HDC hdc, int w, int h) {
  RECT rect = {0, 0, w, h};
  FillRect(hdc, &rect, (HBRUSH)(COLOR_3DFACE + 1));

  HPEN dc_pen = (HPEN)GetStockObject(DC_PEN);
  HGDIOBJ original = SelectObject(hdc, dc_pen);

  HFONT font = CreateFontHelper(32, 0, "Tahoma");
  SelectObject(hdc, font);
  SetBkMode(hdc, TRANSPARENT);

  HPEN sel_pen = CreatePen(PS_SOLID, RescaleDpi(3), GetSysColor(COLOR_HIGHLIGHT));
  SetDCPenColor(hdc, GetSysColor(COLOR_3DSHADOW));

  int item_width = 35, item_spacing = 43, n = 6, xmarg = 15, middle_spacing = 12;
  if (twofactormax == 8) {
    item_width = 32;
    xmarg = 2;
    item_spacing = 36;
    middle_spacing = 6;
  } else if (twofactormax == 7) {
    item_width = 35;
    item_spacing = 41;
    xmarg = 6;
    middle_spacing = 0;
  }
  int radius = RescaleDpi(10);
  for (int i = 0; i < twofactormax; i++) {
    int x = xmarg + item_spacing * i + (i * 2 >= twofactormax) * middle_spacing;

    SelectObject(hdc, i == twofactornum ? sel_pen : dc_pen);
    RECT r2 = {x, 5, x + item_width, 5 + 42};
    RECT r = RescaleDpiRect(r2);
    RoundRect(hdc, r.left, r.top, r.right, r.bottom, radius, radius);
    
    if (i < twofactornum)
      DrawText(hdc, twofactordig + i, 1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);
   }

  DeleteObject(font);
  DeleteObject(sel_pen);
  SelectObject(hdc, original);
}

static LRESULT CALLBACK TwoFactorEditFieldWndProc(HWND  hwnd, UINT  uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_PAINT: {
    HandleWmPaintPaintbox(hwnd, &DrawInTwoFactorBox);
    return TRUE;
  }
  case WM_GETDLGCODE:
    return DLGC_WANTCHARS;
  case WM_KEYDOWN:
    if (wParam >= '0' && wParam <= '9' && twofactornum < twofactormax) {
      twofactordig[twofactornum++] = (char)wParam;
      if (twofactornum == twofactormax) {
        twofactordig[twofactornum] = 0;
        g_backend->SubmitToken(std::string(twofactordig));
        SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
      } else {
        InvalidateRect(hwnd, NULL, FALSE);
      }
      return FALSE;
    } else if (wParam == VK_BACK) {
      if (twofactornum > 0) {
        twofactornum--;
        InvalidateRect(hwnd, NULL, FALSE);
      }
      return FALSE;
    } else if (wParam == 'V' && GetAsyncKeyState(VK_CONTROL) < 0) {
      if (twofactornum == 0) {
        std::string digits = GetClipboardString();
        if (digits.size() == twofactormax) {
          g_backend->SubmitToken(std::move(digits));
          SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
        }
      }
      return FALSE;
    }
    break;
  case WM_CHAR:
    return FALSE;

  case WM_ERASEBKGND:
    return TRUE;

  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static INT_PTR WINAPI TwoFactorDlgProc(HWND hWnd, UINT message, WPARAM wParam,
                                       LPARAM lParam) {
  static HFONT twofactorfont;

  switch (message) {
  case WM_INITDIALOG: {
    uint32 failreason = g_twofactor_dialog_request & kTokenRequestStatus_Mask;
    if (failreason) {
      size_t index = (failreason >> 8) - 1;
      
      static const char * const kFailReasons[] = {
        "Code Not Accepted. Please try again.",
        "Incorrect code. Please try again.",
        "Account locked.",
        "Rate limited. Please wait 30 seconds.",
      };

      HWND label = GetDlgItem(hWnd, IDC_CODENOTACCEPTED);
      SetWindowText(label, kFailReasons[index >= ARRAYSIZE(kFailReasons) ? 0 : index]);
      ShowWindow(label, SW_SHOW);
    }

    int type = g_twofactor_dialog_request & kTokenRequestType_Mask;
    if (type >= kTokenRequestType_6digits && type <= kTokenRequestType_8digits) {
      twofactormax = (uint8)(type - kTokenRequestType_6digits + 6);
    } else {
      if (type != kTokenRequestType_Password)
        SendDlgItemMessage(hWnd, IDC_TWOFACTOREDIT, EM_SETPASSWORDCHAR, 0, 0);

      twofactorfont = CreateFontHelper(20, 0, "Tahoma", 0);
      SendDlgItemMessage(hWnd, IDC_TWOFACTOREDIT, WM_SETFONT, (WPARAM)twofactorfont, 0);
    }
    return TRUE;
  }

  case WM_DESTROY:
    if (twofactorfont)
      DeleteObject(exch_null(twofactorfont));
    return FALSE;


  case WM_CTLCOLORSTATIC:
    if (GetWindowLong((HWND)lParam, GWL_ID) == IDC_CODENOTACCEPTED) {
      SetTextColor((HDC)wParam, RGB(255, 0, 0));
      SetBkMode((HDC)wParam, TRANSPARENT);
      return (LRESULT)GetSysColorBrush(COLOR_3DFACE);
    }
    break;
    
  case WM_CLOSE:
    EndDialog(hWnd, 0);
    g_twofactor_dialog_shown = false;
    return TRUE;
  case WM_COMMAND:
    switch (wParam) {
    case IDCANCEL:
      EndDialog(hWnd, 0);
      g_twofactor_dialog_shown = false;
      return TRUE;
    case IDOK: {
      wchar_t buf[TunsafePlugin::kMaxTokenLen + 1];
      char utf8buf[TunsafePlugin::kMaxTokenLen + 1];
      buf[0] = 0;
      int nw = GetDlgItemTextW(hWnd, IDC_TWOFACTOREDIT, buf, ARRAYSIZE(buf)) + 1;
      int nutf8 = WideCharToMultiByte(CP_UTF8, 0, buf, nw, utf8buf, ARRAYSIZE(utf8buf), 0, NULL);
      if (nutf8) {
        g_backend->SubmitToken(std::string(utf8buf));
        EndDialog(hWnd, 0);
        g_twofactor_dialog_shown = false;
        return TRUE;
      }
    }
    }
    break;
  }
  return FALSE;
}

void ShowTwoFactorDialog() {
  twofactornum = 0;
  int type = g_twofactor_dialog_request & kTokenRequestType_Mask;
  int dialog;
  if (type >= kTokenRequestType_6digits && type <= kTokenRequestType_8digits) {
    dialog = IDD_DIALOG3;
  } else {
    dialog = IDD_DIALOG4;
  }
  DialogBox(g_hinstance, MAKEINTRESOURCE(dialog), g_ui_window, &TwoFactorDlgProc);
}

void InitializeClass(WNDPROC wndproc, const char *name) {
  WNDCLASSEX wce = {0};
  wce.cbSize = sizeof(wce);
  wce.lpfnWndProc = wndproc;
  wce.hInstance = g_hinstance;
  wce.lpszClassName = name;
  wce.style = CS_HREDRAW | CS_VREDRAW;
  wce.hCursor = LoadCursor(NULL, IDC_ARROW);
  RegisterClassEx(&wce);
}

static bool CreateMainWindow() {
  LoadLibrary(TEXT("Riched20.dll"));
  INITCOMMONCONTROLSEX ccx;
  ccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
  ccx.dwICC = ICC_TAB_CLASSES;
  InitCommonControlsEx(&ccx);

  InitializeClass(&PaintBoxWndProc, "PaintBox");
  InitializeClass(&GraphBoxWndProc, "GraphBox");
  InitializeClass(&AdvancedBoxWndProc, "AdvancedBox");
  InitializeClass(&TwoFactorEditFieldWndProc, "TwoFactorEditField");

  HDC dc = GetDC(0);
  g_large_fonts = GetDeviceCaps(dc, LOGPIXELSX);
  ReleaseDC(0, dc);

  g_message_taskbar_created = RegisterWindowMessage(TEXT("TaskbarCreated"));

  g_icons[0] = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
  g_icons[1] = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON0));
  g_ui_window = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), NULL, &DlgProc);

  if (!g_ui_window)
    return false;
  
  DragAcceptFiles(g_ui_window, TRUE);

  ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
  ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
  ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
  ChangeWindowMessageFilter(WM_USER + 10, MSGFLT_ADD);

  TCITEM tabitem;
  HWND hwnd_tab = GetDlgItem(g_ui_window, IDC_TAB);
  hwndTab = hwnd_tab;
  tabitem.mask = TCIF_TEXT;
  tabitem.pszText = "Logs";
  TabCtrl_InsertItem(hwnd_tab, 0, &tabitem);
  tabitem.pszText = "Charts";
  TabCtrl_InsertItem(hwnd_tab, 1, &tabitem);
  tabitem.pszText = "Advanced";
  TabCtrl_InsertItem(hwnd_tab, 2, &tabitem);
  SetWindowLong(hwnd_tab, GWL_EXSTYLE, GetWindowLong(hwnd_tab, GWL_EXSTYLE) | WS_EX_COMPOSITED);



  hwndEdit = GetDlgItem(g_ui_window, IDC_RICHEDIT21);
  hwndPaintBox = GetDlgItem(g_ui_window, IDC_PAINTBOX);
  hwndGraphBox = GetDlgItem(g_ui_window, IDC_GRAPHBOX);
  hwndAdvancedBox = GetDlgItem(g_ui_window, IDC_ADVANCEDBOX);

  SetWindowLong(hwndPaintBox, GWL_STYLE, GetWindowLong(hwndPaintBox, GWL_STYLE) | WS_CLIPSIBLINGS);

  SetWindowLong(hwndEdit, GWL_EXSTYLE, GetWindowLong(hwndEdit, GWL_EXSTYLE) &~ WS_EX_CLIENTEDGE);

  // Create the status bar.
  hwndStatus = CreateWindowEx(
      WS_EX_COMPOSITED, STATUSCLASSNAME, NULL,
      WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, g_ui_window,
      (HMENU)IDC_STATUSBAR, g_hinstance, NULL);

  HandleWindowSizing();
  UpdateTabSelection();
  return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  g_hinstance = hInstance;
  InitCpuFeatures();

  WSADATA wsaData = {0};
  WSAStartup(MAKEWORD(2, 2), &wsaData);

  bool minimize = false;
  bool is_autostart = false;
  const char *filename = NULL;

  for (int i = 1; i < __argc; i++) {
    const char *arg = __argv[i];
    if (strcmp(arg, "/minimize") == 0) {
      minimize = true;
    } else if (strcmp(arg, "/minimize_on_connect") == 0) {
      g_minimize_on_connect = true;
    } else if (strcmp(arg, "/allow_pre_post") == 0) {
      g_allow_pre_post = true;
    } else if (strcmp(arg, "--service") == 0) {
      RunProcessAsTunsafeServiceProcess();
      return 0;
    } else if (strcmp(arg, "--delete-service-and-start") == 0) {
      UninstallTunSafeWindowsService();
    } else if (strcmp(arg, "--autostart") == 0) {
      is_autostart = true;
    } else if (strcmp(arg, "--set-allow-pre-post") == 0) {
      bool want = i + 1 < __argc && atoi(__argv[i + 1]) != 0;
      RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\TunSafe", NULL, NULL, 0, KEY_ALL_ACCESS, NULL, &g_hklm_reg_key, NULL);
      RegWriteInt(g_hklm_reg_key, "AllowPrePost", want);
      return 0;
    } else if (strcmp(arg, "--import") == 0) {
      if (i + 1 >= __argc) return 1;
      const char *filename = __argv[i + 1];
      return ImportFile(filename, true);
    } else {
      filename = arg;
      break;
    }
  }

  SetProcessDPIAware();

  RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\TunSafe", NULL, NULL, 0, KEY_ALL_ACCESS, NULL, &g_reg_key, NULL);
  RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\TunSafe", NULL, NULL, 0, KEY_ALL_ACCESS, NULL, &g_hklm_reg_key, NULL);
  RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\TunSafe", 0, KEY_READ, &g_hklm_readonly_reg_key);
   
  g_startup_flags = RegReadInt(g_reg_key, "StartupFlags", 0);

  if (is_autostart) {
    g_disable_connect_on_start = !(g_startup_flags & kStartupFlag_ConnectWhenWindowsStarts);
    minimize = !!(g_startup_flags & kStartupFlag_MinimizeToTrayWhenWindowsStarts);
  }

  // Check if the app is already running.
  g_runonce_mutex = CreateMutexA(0, FALSE, "TunSafe-f19e092db01cbe0fb6aee132f8231e5b71c98f90");
  if (GetLastError() == ERROR_ALREADY_EXISTS&&0) {
    HWND window = FindWindow("TunSafe-f19e092db01cbe0fb6aee132f8231e5b71c98f90", NULL);
    DWORD_PTR result;
    if (!window || !SendMessageTimeout(window, WM_USER + 10, 0, 0, SMTO_BLOCK, 3000, &result) || result != 31337) {
      MessageBoxA(NULL, "It looks like TunSafe is already running, but not responding. Please kill the old process first.", "TunSafe", MB_ICONWARNING);
    }
    return 1;
  }

  TOKEN_ELEVATION_TYPE toktype;
  g_is_limited_uac_account = (GetProcessElevationType(&toktype) && toktype == TokenElevationTypeLimited);
  g_is_tunsafe_service_running = IsTunsafeServiceRunning();
  bool want_use_service = !!(g_startup_flags & (kStartupFlag_BackgroundService | kStartupFlag_ForegroundService));
  
  // Re-launch the process as administrator if the TunSafe service isn't running.
  if ((!g_is_tunsafe_service_running || !want_use_service) && g_is_limited_uac_account) {
    CloseHandle(g_runonce_mutex);
    if (!RestartProcessAsAdministrator())
      MessageBoxA(0, "TunSafe needs to run as Administrator unless the TunSafe Service is started.", "TunSafe", MB_ICONWARNING);
    return 0;
  }

  CreateNotificationWindow();

  g_backend_delegate = CreateTunsafeBackendDelegateThreaded(&my_procdel, []() {
    if (g_ui_window)
      PostMessage(g_ui_window, WM_USER + 2, 0, 0);
  });
  g_logger = &PushLine;

  if (!CreateMainWindow())
    return 1;

  g_current_filename = _strdup("");
  g_cmdline_filename = filename;

  if (!g_allow_pre_post && g_hklm_readonly_reg_key)
    g_allow_pre_post = RegReadInt(g_hklm_readonly_reg_key, "AllowPrePost", 0) != 0;

  // Attempt to start service...
  if (want_use_service && !g_is_tunsafe_service_running) {
    RINFO("Starting TunSafe service...");
    InstallTunSafeWindowsService();
  }

  CreateLocalOrRemoteBackend(want_use_service);

  if (!minimize) {
    SetUiVisibility(true);
  }
  UpdateIcon(UIW_NONE);
  EnsureConfigDirCreated();

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!IsDialogMessage(g_ui_window, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  if (!g_backend->is_remote())
    g_backend->Stop();

  delete g_backend;
  RemoveIcon();

  return 0;
}
