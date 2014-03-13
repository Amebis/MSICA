#ifndef __MSICATS_H__
#define __MSICATS_H__

#include <msi.h>


////////////////////////////////////////////////////////////////////
// Calling declaration
////////////////////////////////////////////////////////////////////

#if defined(MSICATS_DLL)
#define MSICATS_API __declspec(dllexport)
#elif defined(MSICATS_DLLIMP)
#define MSICATS_API __declspec(dllimport)
#else
#define MSICATS_API
#endif

////////////////////////////////////////////////////////////////////
// Exported functions
////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

    UINT MSICATS_API EvaluateScheduledTasks(MSIHANDLE hInstall);
    UINT MSICATS_API InstallScheduledTasks(MSIHANDLE hInstall);

#ifdef __cplusplus
}
#endif

#endif // __MSICATS_H__
