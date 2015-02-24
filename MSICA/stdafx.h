/*
    Copyright ("1991-2015") Amebis

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

#ifdef _WINDLL
#define MSICA_DLL // This library is compiled as a DLL
#endif

#include <atlbase.h>
#include <atlfile.h>
#include <atlstr.h>

#include <corerror.h>
#include <stdlib.h>
#include <time.h>
#include <wlanapi.h>

#include "..\MSICALib\MSICALib.h"

#include "MSICA.h"
