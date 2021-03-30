/*
    Copyright © 1991-2021 Amebis

    This file is part of MSICA.

    MSICA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MSICA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MSICA. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <WinStd\COM.h>

#include <Windows.h>

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
