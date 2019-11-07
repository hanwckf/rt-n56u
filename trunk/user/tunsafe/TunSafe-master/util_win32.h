// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "tunsafe_types.h"
#include <vector>
#include <string>

#pragma once
const char *FindFilenameComponent(const char *s);
void str_set(char **x, const char *s);

char *str_cat_alloc(const char * const *a, size_t n);
char *str_cat_alloc(const char *a, const char *b);
char *str_cat_alloc(const char *a, const char *b, const char *c);

int RegReadInt(HKEY hkey, const char *key, int def);
void RegWriteInt(HKEY hkey, const char *key, int value);
char *RegReadStr(HKEY hkey, const char *key, const char *def);
void RegWriteStr(HKEY hkey, const char *key, const char *v);

// TokenElevationTypeDefault -- User is not using a split token. (e.g. UAC disabled or local admin "Administrator" account which UAC may not apply to.)
// TokenElevationTypeFull    -- User has a split token, and the process is running elevated.
// TokenElevationTypeLimited -- User has a split token, but the process is not running elevated.
bool GetProcessElevationType(TOKEN_ELEVATION_TYPE *pOutElevationType);


const wchar_t *SkipAppNameInCommandLineArgs(const wchar_t *s);

uint8* LoadFileSane(const char *name, size_t *size);

enum {
  kWriteOutFile_Ok = 0,
  kWriteOutFile_AccessError = 1,
  kWriteOutFile_OtherError = 2,
};

int WriteOutFile(const char *filename, uint8 *filedata, size_t filesize);

bool SanityCheckBuf(uint8 *buf, size_t n);

__int64 FileSize(const char* name);

bool FileExists(const CHAR *fileName);

void ShellExecuteFromExplorer(
  PCSTR pszFile,
  PCSTR pszParameters = nullptr,
  PCSTR pszDirectory = nullptr,
  PCSTR pszOperation = nullptr,
  int nShowCmd = SW_SHOWNORMAL);

size_t GetConfigPath(char *path, size_t path_size);
bool ExpandConfigPath(const char *basename, char *fullname, size_t fullname_size);
bool EnsureValidConfigPath(const char *path);

bool RunProcessAsAdminWithArgs(const char *args, bool wait_for_exit);
bool RestartProcessAsAdministrator();
bool SetClipboardString(const char *string);
std::string GetClipboardString();
RECT GetParentRect(HWND wnd);
RECT MakeRect(int l, int t, int r, int b);
struct GuidAndDevName {
  char guid[40];
  char name[64];
};
void GetTapAdapterInfo(std::vector<GuidAndDevName> *result);

