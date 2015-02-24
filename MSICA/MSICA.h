/*
    MSICA, Copyright (C) Amebis, Slovenia

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the GNU General Public License for more details, included in the file
    LICENSE.rtf which you should have received along with this program.

    If you did not receive a copy of the GNU General Public License,
    write to the Free Software Foundation, Inc., 675 Mass Ave,
    Cambridge, MA 02139, USA.

    Amebis can be contacted at http://www.amebis.si
*/

#ifndef __MSICA_H__
#define __MSICA_H__

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

#endif // __MSICA_H__
