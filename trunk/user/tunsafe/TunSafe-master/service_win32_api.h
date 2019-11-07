// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once

#include "network_win32_api.h"

enum StartupFlags {
  kStartupFlag_ForegroundService = 1,
  kStartupFlag_BackgroundService = 2,
  kStartupFlag_ConnectWhenWindowsStarts = 4,
  kStartupFlag_MinimizeToTrayWhenWindowsStarts = 8,
};

BOOL RunProcessAsTunsafeServiceProcess();

void StopTunsafeService();

bool IsTunSafeServiceInstalled();

bool IsTunsafeServiceRunning();
void InstallTunSafeWindowsService();
bool UninstallTunSafeWindowsService();

TunsafeBackend *CreateTunsafeServiceClient(TunsafeBackend::Delegate *delegate);
