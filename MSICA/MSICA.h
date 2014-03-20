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

    UINT MSICA_API EvaluateCertificates(MSIHANDLE hInstall);
    UINT MSICA_API EvaluateServiceConfiguration(MSIHANDLE hInstall);
    UINT MSICA_API EvaluateScheduledTasks(MSIHANDLE hInstall);
    UINT MSICA_API ExecuteSequence(MSIHANDLE hInstall);

#ifdef __cplusplus
}
#endif

#endif // __MSICA_H__
