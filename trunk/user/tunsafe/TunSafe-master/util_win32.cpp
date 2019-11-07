// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "util_win32.h"
#include <stdlib.h>
#include <string.h>
#include <shldisp.h>
#include <shlobj.h>
#include <exdisp.h>
#include <atlbase.h>

const char *FindFilenameComponent(const char *s) {
  size_t len = strlen(s);
  for (;;) {
    if (len == 0)
      return "";
    len--;
    if (s[len] == '\\' || s[len] == '/')
      break;
  }
  return s + len + 1;
}

void str_set(char **x, const char *s) {
  free(*x);
  *x = _strdup(s);
}

char *str_cat_alloc(const char * const *a, size_t n) {
  if (n > 32) return NULL;
  size_t len[32], totlen = 0;
  for (size_t i = 0; i < n; i++) {
    len[i] = strlen(a[i]);
    totlen += len[i];
  }
  char *r = (char *)malloc(totlen + 1);
  totlen = 0;
  for (size_t i = 0; i < n; i++) {
    size_t n = len[i];
    memcpy(r + totlen, a[i], n);
    totlen += n;
  }
  r[totlen] = 0;
  return r;
}

char *str_cat_alloc(const char *a, const char *b) {
  const char * x[2] = {a, b};
  return str_cat_alloc(x, 2);
}

char *str_cat_alloc(const char *a, const char *b, const char *c) {
  const char * x[3] = {a, b, c};
  return str_cat_alloc(x, 3);
}


int RegReadInt(HKEY hkey, const char *key, int def) {
  DWORD value = def, n = sizeof(value);
  RegQueryValueEx(hkey, key, NULL, NULL, (BYTE*)&value, &n);
  return value;
}

void RegWriteInt(HKEY hkey, const char *key, int value) {
  RegSetValueEx(hkey, key, NULL, REG_DWORD, (BYTE*)&value, sizeof(value));
}

char *RegReadStr(HKEY hkey, const char *key, const char *def) {
  char buf[1024];
  DWORD n = sizeof(buf) - 1;
  DWORD type = 0;
  if (RegQueryValueEx(hkey, key, NULL, &type, (BYTE*)buf, &n) != ERROR_SUCCESS || type != REG_SZ)
    return def ? _strdup(def) : NULL;
  if (n && buf[n - 1] == 0)
    n--;
  buf[n] = 0;
  return _strdup(buf);
}

void RegWriteStr(HKEY hkey, const char *key, const char *v) {
  RegSetValueEx(hkey, key, NULL, REG_SZ, (BYTE*)v, (DWORD)strlen(v) + 1);
}

bool GetProcessElevationType(TOKEN_ELEVATION_TYPE *pOutElevationType) {
  *pOutElevationType = TokenElevationTypeDefault;
  bool fResult = false;
  HANDLE hProcToken = NULL;
  if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hProcToken)) {
    DWORD dwSize = 0;
    TOKEN_ELEVATION_TYPE elevationType = TokenElevationTypeDefault;
    if (::GetTokenInformation(hProcToken, TokenElevationType, &elevationType, sizeof(elevationType), &dwSize)
        && dwSize == sizeof(elevationType)) {
      *pOutElevationType = elevationType;
      fResult = true;
    }
    ::CloseHandle(hProcToken);
  }
  return fResult;
}

/*++
Routine Description: This routine returns TRUE if the caller's
process is a member of the Administrators local group. Caller is NOT
expected to be impersonating anyone and is expected to be able to
open its own process and process token.
Arguments: None.
Return Value:
TRUE - Caller has Administrators local group.
FALSE - Caller does not have Administrators local group. --
*/

BOOL IsUserAdmin(VOID) {
  BOOL b;
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  PSID AdministratorsGroup;
  b = AllocateAndInitializeSid(
    &NtAuthority,
    2,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_ADMINS,
    0, 0, 0, 0, 0, 0,
    &AdministratorsGroup);
  if (b) {
    if (!CheckTokenMembership(NULL, AdministratorsGroup, &b)) {
      b = FALSE;
    }
    FreeSid(AdministratorsGroup);
  }

  return(b);
}


const wchar_t *SkipAppNameInCommandLineArgs(const wchar_t *s) {
  if (*s == '\"') {
    for (;;) {
      s++;
      if (*s == 0) return s;
      if (*s == '\"') return s + 1;
    }
  } else {
    for (;;) {
      if (*s == 0) return s;
      if (*s == ' ') return s + 1;
      s++;
    }
  }
}


uint8* LoadFileSane(const char *name, size_t *size) {
  FILE *f = fopen(name, "rb");
  uint8 *new_file = NULL, *file = NULL;
  size_t j, i, n;
  if (!f) return false;
  fseek(f, 0, SEEK_END);
  long x = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (x < 0 || x >= 65536) goto error;
  file = (uint8*)malloc(x + 1);
  if (!file) goto error;
  n = fread(file, 1, x + 1, f);
  if (n != x || !SanityCheckBuf(file, n))
    goto error;
  // Convert the file to DOS new lines
  for (i = j = 0; i < n; i++)
    j += (file[i] == '\n');
  new_file = (uint8*)malloc(n + 1 + j);
  if (!new_file) goto error;
  for (i = j = 0; i < n; i++) {
    uint8 c = file[i];
    if (c == '\r')
      continue;
    if (c == '\n')
      new_file[j++] = '\r';
    new_file[j++] = c;
  }
  new_file[j] = 0;
  *size = j;

error:
  fclose(f);
  free(file);
  return new_file;
}

int WriteOutFile(const char *filename, uint8 *filedata, size_t filesize) {
  FILE *f = fopen(filename, "wb");
  if (!f) return kWriteOutFile_AccessError;
  if (fwrite(filedata, 1, filesize, f) != filesize) {
    fclose(f);
    return kWriteOutFile_OtherError;
  }
  fclose(f);
  return kWriteOutFile_Ok;
}

bool FileExists(const CHAR *fileName) {
  DWORD fileAttr = GetFileAttributes(fileName);
  return (0xFFFFFFFF != fileAttr);
}

__int64 FileSize(const char* name) {
  WIN32_FILE_ATTRIBUTE_DATA fad;
  if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
    return -1; // error condition, could call GetLastError to find out more
  LARGE_INTEGER size;
  size.HighPart = fad.nFileSizeHigh;
  size.LowPart = fad.nFileSizeLow;
  return size.QuadPart;
}

static bool is_space(uint8_t c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

static bool is_valid(uint8_t c) {
  return c >= ' ' || c == '\r' || c == '\n' || c == '\t';
}

bool SanityCheckBuf(uint8 *buf, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (!is_space(buf[i])) {
      if (buf[i] != '[' && buf[i] != '#')
        return false;
      for (; i < n; i++)
        if (!is_valid(buf[i]))
          return false;
      return true;
    }
  }
  return false;
}

void FindDesktopFolderView(REFIID riid, void **ppv) {
  CComPtr<IShellWindows> spShellWindows;
  spShellWindows.CoCreateInstance(CLSID_ShellWindows);

  CComVariant vtLoc(CSIDL_DESKTOP);
  CComVariant vtEmpty;
  long lhwnd;
  CComPtr<IDispatch> spdisp;
  spShellWindows->FindWindowSW(
    &vtLoc, &vtEmpty,
    SWC_DESKTOP, &lhwnd, SWFO_NEEDDISPATCH, &spdisp);

  CComPtr<IShellBrowser> spBrowser;
  CComQIPtr<IServiceProvider>(spdisp)->
    QueryService(SID_STopLevelBrowser,
                 IID_PPV_ARGS(&spBrowser));

  CComPtr<IShellView> spView;
  spBrowser->QueryActiveShellView(&spView);

  spView->QueryInterface(riid, ppv);
}

void GetDesktopAutomationObject(REFIID riid, void **ppv) {
  CComPtr<IShellView> spsv;
  FindDesktopFolderView(IID_PPV_ARGS(&spsv));
  CComPtr<IDispatch> spdispView;
  spsv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARGS(&spdispView));
  spdispView->QueryInterface(riid, ppv);
}

void ShellExecuteFromExplorer(
  PCSTR pszFile,
  PCSTR pszParameters,
  PCSTR pszDirectory,
  PCSTR pszOperation,
  int nShowCmd) {
  CComPtr<IShellFolderViewDual> spFolderView;
  GetDesktopAutomationObject(IID_PPV_ARGS(&spFolderView));
  CComPtr<IDispatch> spdispShell;
  spFolderView->get_Application(&spdispShell);

  CComQIPtr<IShellDispatch2>(spdispShell)
    ->ShellExecute(CComBSTR(pszFile),
                   CComVariant(pszParameters ? pszParameters : ""),
                   CComVariant(pszDirectory ? pszDirectory : ""),
                   CComVariant(pszOperation ? pszOperation : ""),
                   CComVariant(nShowCmd));
}

size_t GetConfigPath(char *path, size_t path_size) {

  if (!GetModuleFileName(NULL, path, (DWORD)path_size)) {
    *path = 0;
    return 0;
  }
  char *last = (char *)FindFilenameComponent(path);
  if (!*last || last + 8 > path + path_size) {
    *path = 0;
    return 0;
  }
  memcpy(last, "Config\\", 8 * sizeof(last[0]));
  return last + 7 - path;
}

bool ExpandConfigPath(const char *basename, char *fullname, size_t fullname_size) {
  size_t len = strlen(basename);

  if (FindFilenameComponent(basename)[0]) {
    if (len >= fullname_size)
      return false;
    memcpy(fullname, basename, len + 1);
    return true;
  }
  size_t clen = GetConfigPath(fullname, fullname_size);
  if (clen == 0 || clen + len >= fullname_size)
    return false;
  memcpy(fullname + clen, basename, (len + 1) * sizeof(fullname[0]));
  return true;
}


static bool ContainsDotDot(const char *path) {
  for (uint8 last = 0, cur; (cur = path[0]) != '\0'; last = cur, path++)
    if (cur == '.' && last == cur)
      return true;
  return false;
}

bool EnsureValidConfigPath(const char *path) {
  char buf[1024];

  size_t len = GetConfigPath(buf, sizeof(buf));
  return (len != 0) && (strlen(path) > len && _strnicmp(path, buf, len) == 0 && !ContainsDotDot(path + len));
}

bool RunProcessAsAdminWithArgs(const char *args, bool wait_for_exit) {
  SHELLEXECUTEINFO shExecInfo = {0};
  char buf[1024];

  if (!GetModuleFileName(NULL, buf, 1024))
    return false;
  shExecInfo.cbSize = sizeof(shExecInfo);
  shExecInfo.lpVerb = "runas";
  shExecInfo.lpFile = buf;
  shExecInfo.lpParameters = args;
  shExecInfo.nShow = SW_SHOW;
  shExecInfo.fMask = SEE_MASK_NOASYNC | wait_for_exit * SEE_MASK_NOCLOSEPROCESS;
  if (!ShellExecuteExA(&shExecInfo))
    return false;
  if (shExecInfo.hProcess) {
    WaitForSingleObject(shExecInfo.hProcess, 10000);
    CloseHandle(shExecInfo.hProcess);
  }
  return true;
}

bool RestartProcessAsAdministrator() {
  SHELLEXECUTEINFOW shExecInfo = {0};
  wchar_t buf[1024];

  if (!GetModuleFileNameW(NULL, buf, 1024))
    return false;

//  shExecInfo.hwnd = window;
  shExecInfo.cbSize = sizeof(shExecInfo);
  shExecInfo.lpVerb = L"runas";
  shExecInfo.lpFile = buf;
  shExecInfo.lpParameters = SkipAppNameInCommandLineArgs(GetCommandLineW());
  shExecInfo.nShow = SW_SHOW;

  return ShellExecuteExW(&shExecInfo) != 0;
}

bool SetClipboardString(const char *string) {
  bool ok = false;
  if (OpenClipboard(NULL)) {
    HGLOBAL hglb;
    size_t len = strlen(string);
    hglb = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, (len + 1) * sizeof(char));
    LPSTR lptstr = (LPSTR)GlobalLock(hglb);
    memcpy(lptstr, string, len + 1);
    GlobalUnlock(hglb);
    EmptyClipboard();
    ok = SetClipboardData(CF_TEXT, hglb) != 0;
    CloseClipboard();
  }
  return ok;
}

std::string GetClipboardString() {
  std::string rv;
  if (OpenClipboard(NULL)) {
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData != NULL) {
      char *pszText = static_cast<char*>(GlobalLock(hData));
      if (pszText != NULL)
        rv = pszText;
      GlobalUnlock(hData);
    }
    CloseClipboard();
  }
  return rv;
}


RECT GetParentRect(HWND wnd) {
  RECT btrect;
  GetClientRect(wnd, &btrect);
  MapWindowPoints(wnd, GetParent(wnd), (LPPOINT)&btrect, 2);
  return btrect;
}

RECT MakeRect(int l, int t, int r, int b) {
  RECT rr = { l, t, r, b };
  return rr;
}

// Retrieve the device path to the TAP adapter.

#define kAdapterKeyName "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"
#define kNetworkConnectionsKeyName "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}"
#define kTapComponentId "tap0901"


void GetTapAdapterInfo(std::vector<GuidAndDevName> *result) {
  LONG err;
  HKEY adapter_key, device_key, network_connections_key;
  bool retval = false;
  GuidAndDevName gn;

  err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, kAdapterKeyName, 0, KEY_READ, &adapter_key);
  if (err != ERROR_SUCCESS) {
    RERROR("GetTapAdapterName: RegOpenKeyEx failed: 0x%X", GetLastError());
    return;
  }
  for (int i = 0; !retval; i++) {
    char keyname[64 + sizeof(kAdapterKeyName) + 1 + 32 /* some margin */];
    char value[64];
    DWORD len = sizeof(value), type;
    err = RegEnumKeyEx(adapter_key, i, value, &len, NULL, NULL, NULL, NULL);
    if (err == ERROR_NO_MORE_ITEMS)
      break;
    if (err != ERROR_SUCCESS) {
      RERROR("GetTapAdapterName: RegEnumKeyEx failed: 0x%X", GetLastError());
      break;
    }
    snprintf(keyname, sizeof(keyname), "%s\\%s", kAdapterKeyName, value);
    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyname, 0, KEY_READ, &device_key);
    if (err == ERROR_SUCCESS) {
      len = sizeof(value);
      err = RegQueryValueEx(device_key, "ComponentId", NULL, &type, (LPBYTE)value, &len);
      if (err == ERROR_SUCCESS && type == REG_SZ && !memcmp(value, kTapComponentId, sizeof(kTapComponentId))) {
        len = sizeof(gn.guid);
        err = RegQueryValueEx(device_key, "NetCfgInstanceId", NULL, &type, (LPBYTE)gn.guid, &len);
        if (err == ERROR_SUCCESS && type == REG_SZ) {
          gn.guid[sizeof(gn.guid) - 1] = 0;
          gn.name[0] = 0;

          snprintf(keyname, sizeof(keyname), "%s\\%s\\Connection", kNetworkConnectionsKeyName, gn.guid);
          err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyname, 0, KEY_READ, &network_connections_key);
          if (err == ERROR_SUCCESS) {
            len = sizeof(gn.guid);
            err = RegQueryValueEx(network_connections_key, "Name", NULL, &type, (LPBYTE)gn.name, &len);
            if (err == ERROR_SUCCESS && type == REG_SZ) {
              gn.name[sizeof(gn.guid) - 1] = 0;
            } else {
              gn.name[0] = 0;
            }
            RegCloseKey(network_connections_key);
          }
          result->push_back(gn);
        }
      }
      RegCloseKey(device_key);
    }
  }
  RegCloseKey(adapter_key);
}
