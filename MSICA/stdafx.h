// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

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
