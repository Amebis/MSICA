/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright © 1991-2022 Amebis
*/

#ifndef __MSICA_H__
#define __MSICA_H__

////////////////////////////////////////////////////////////////////
// Version
////////////////////////////////////////////////////////////////////

#define MSICA_VERSION        0x02000000

#define MSICA_VERSION_MAJ    2
#define MSICA_VERSION_MIN    0
#define MSICA_VERSION_REV    0
#define MSICA_VERSION_BUILD  0

#define MSICA_VERSION_STR    "2.0"
#define MSICA_BUILD_YEAR_STR "2018"

#if !defined(RC_INVOKED) && !defined(MIDL_PASS)

#include <msi.h>

////////////////////////////////////////////////////////////////////
// Exported functions
////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

    UINT __declspec(dllexport) MSICAInitialize(MSIHANDLE hInstall);
    UINT __declspec(dllexport) ExecuteSequence(MSIHANDLE hInstall);

#ifdef __cplusplus
}
#endif

#endif // !RC_INVOKED && !MIDL_PASS
#endif // __MSICA_H__
