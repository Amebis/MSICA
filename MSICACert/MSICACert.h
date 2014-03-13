#ifndef __MSICACERT_H__
#define __MSICACERT_H__

#include <msi.h>


////////////////////////////////////////////////////////////////////
// Calling declaration
////////////////////////////////////////////////////////////////////

#if defined(MSICACERT_DLL)
#define MSICACERT_API __declspec(dllexport)
#elif defined(MSICACERT_DLLIMP)
#define MSICACERT_API __declspec(dllimport)
#else
#define MSICACERT_API
#endif

////////////////////////////////////////////////////////////////////
// Exported functions
////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

    UINT MSICACERT_API EvaluateSequence(MSIHANDLE hInstall);
    UINT MSICACERT_API ExecuteSequence(MSIHANDLE hInstall);

#ifdef __cplusplus
}
#endif

#endif // __MSICACERT_H__
