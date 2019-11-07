// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WINVER 0x0A00  
#define _WIN32_WINNT _WIN32_WINNT_VISTA  
#define NTDDI_VERSION NTDDI_VISTA

#include "build_config.h"

#if defined(OS_WIN)
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#define _HAS_EXCEPTIONS 0
#define _CRT_SECURE_NO_WARNINGS 1
#define NOMINMAX

//#include <Winsock2.h>
#include <Ws2tcpip.h>

#include <Windows.h>
//#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <Windowsx.h>

#include <tchar.h>
#else
#define override
#endif

#include <stdio.h>
#include <stddef.h>

#include <vector>
#include <string>

