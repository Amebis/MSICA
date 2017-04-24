/*
    Copyright 1991-2017 Amebis

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

#ifndef __MSICA_H__
#define __MSICA_H__

////////////////////////////////////////////////////////////////////
// Version
////////////////////////////////////////////////////////////////////

#define MSICA_VERSION        0x01ff0000

#define MSICA_VERSION_MAJ    1
#define MSICA_VERSION_MIN    255
#define MSICA_VERSION_REV    0
#define MSICA_VERSION_BUILD  0

#define MSICA_VERSION_STR    "2.0-pre"
#define MSICA_BUILD_YEAR_STR "2016"

#if !defined(RC_INVOKED) && !defined(MIDL_PASS)

#include <msi.h>

////////////////////////////////////////////////////////////////////
// Calling declaration
////////////////////////////////////////////////////////////////////

#if defined(MSICA_DLL)
#define MSICA_API __declspec(dllexport)
#elif defined(MSICA_DLLIMP)
#define MSICA_API __declspec(dllimport)
#else
#define MSICA_API
#endif

////////////////////////////////////////////////////////////////////
// Exported functions
////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

    UINT MSICA_API MSICAInitialize(MSIHANDLE hInstall);
    UINT MSICA_API ExecuteSequence(MSIHANDLE hInstall);

#ifdef __cplusplus
}
#endif

#endif // !RC_INVOKED && !MIDL_PASS
#endif // __MSICA_H__
