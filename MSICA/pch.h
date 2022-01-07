/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright © 1991-2022 Amebis
*/

#pragma once

#include <WinStd\COM.h>

#include <corerror.h>
#include <stdlib.h>
#include <time.h>
#include <wlanapi.h>

// Must not statically link to Wlanapi.dll as it is not available on Windows
// without a WLAN interface.
extern VOID  (WINAPI *pfnWlanFreeMemory)(__in PVOID pMemory);
extern DWORD (WINAPI *pfnWlanOpenHandle)(__in DWORD dwClientVersion, __reserved PVOID pReserved, __out PDWORD pdwNegotiatedVersion, __out PHANDLE phClientHandle);
extern DWORD (WINAPI *pfnWlanCloseHandle)(__in HANDLE hClientHandle, __reserved PVOID pReserved);
extern DWORD (WINAPI *pfnWlanEnumInterfaces)(__in HANDLE hClientHandle, __reserved PVOID pReserved, __deref_out PWLAN_INTERFACE_INFO_LIST *ppInterfaceList);

#include <MSICALib.h>

#include "MSICA.h"
