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

#include "pch.h"


////////////////////////////////////////////////////////////////////////////
// Local constants
////////////////////////////////////////////////////////////////////////////

#define MSICA_CERT_TICK_SIZE            ( 4*1024)
#define MSICA_SVC_SET_START_TICK_SIZE   ( 1*1024)
#define MSICA_SVC_START_TICK_SIZE       ( 1*1024)
#define MSICA_SVC_STOP_TICK_SIZE        ( 1*1024)
#define MSICA_TASK_TICK_SIZE            (16*1024)
#define MSICA_WLAN_PROFILE_TICK_SIZE    ( 2*1024)


////////////////////////////////////////////////////////////////////////////
// WLAN API
////////////////////////////////////////////////////////////////////////////

static HMODULE hWlanapi = NULL;
VOID  (WINAPI *pfnWlanFreeMemory)(__in PVOID pMemory) = NULL;
DWORD (WINAPI *pfnWlanOpenHandle)(__in DWORD dwClientVersion, __reserved PVOID pReserved, __out PDWORD pdwNegotiatedVersion, __out PHANDLE phClientHandle) = NULL;
DWORD (WINAPI *pfnWlanCloseHandle)(__in HANDLE hClientHandle, __reserved PVOID pReserved) = NULL;
DWORD (WINAPI *pfnWlanEnumInterfaces)(__in HANDLE hClientHandle, __reserved PVOID pReserved, __deref_out PWLAN_INTERFACE_INFO_LIST *ppInterfaceList) = NULL;
DWORD (WINAPI *pfnWlanGetProfile)(__in HANDLE hClientHandle, __in CONST GUID *pInterfaceGuid, __in LPCWSTR strProfileName, __reserved PVOID pReserved, __deref_out LPWSTR *pstrProfileXml, __inout_opt DWORD *pdwFlags, __out_opt DWORD *pdwGrantedAccess) = NULL;
DWORD (WINAPI *pfnWlanDeleteProfile)(__in HANDLE hClientHandle, __in CONST GUID *pInterfaceGuid, __in LPCWSTR strProfileName, __reserved PVOID pReserved) = NULL;
DWORD (WINAPI *pfnWlanSetProfile)(__in HANDLE hClientHandle, __in CONST GUID *pInterfaceGuid, __in DWORD dwFlags, __in LPCWSTR strProfileXml, __in_opt LPCWSTR strAllUserProfileSecurity, __in BOOL bOverwrite, __reserved PVOID pReserved, __out DWORD *pdwReasonCode) = NULL;
DWORD (WINAPI *pfnWlanReasonCodeToString)(__in DWORD dwReasonCode, __in DWORD dwBufferSize, __in_ecount(dwBufferSize) PWCHAR pStringBuffer, __reserved PVOID pReserved) = NULL;


////////////////////////////////////////////////////////////////////////////
// Global functions
////////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD dwReason, _In_ LPVOID pReserved)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(pReserved);

    if (dwReason == DLL_PROCESS_ATTACH) {
        // Load Wlanapi.dll
        hWlanapi = ::LoadLibrary(_T("Wlanapi.dll"));
        if (hWlanapi) {
            pfnWlanFreeMemory         = (VOID (WINAPI*)(PVOID))                                                            ::GetProcAddress(hWlanapi, "pfnWlanFreeMemory"        );
            pfnWlanOpenHandle         = (DWORD(WINAPI*)(DWORD, PVOID, PDWORD, PHANDLE))                                    ::GetProcAddress(hWlanapi, "pfnWlanOpenHandle"        );
            pfnWlanCloseHandle        = (DWORD(WINAPI*)(HANDLE, PVOID))                                                    ::GetProcAddress(hWlanapi, "pfnWlanCloseHandle"       );
            pfnWlanEnumInterfaces     = (DWORD(WINAPI*)(HANDLE, PVOID, PWLAN_INTERFACE_INFO_LIST*))                        ::GetProcAddress(hWlanapi, "pfnWlanEnumInterfaces"    );
            pfnWlanGetProfile         = (DWORD(WINAPI*)(HANDLE, CONST GUID*, LPCWSTR, PVOID, LPWSTR*, DWORD*, DWORD*))     ::GetProcAddress(hWlanapi, "pfnWlanGetProfile"        );
            pfnWlanDeleteProfile      = (DWORD(WINAPI*)(HANDLE, CONST GUID*, LPCWSTR, PVOID))                              ::GetProcAddress(hWlanapi, "pfnWlanDeleteProfile"     );
            pfnWlanSetProfile         = (DWORD(WINAPI*)(HANDLE, CONST GUID*, DWORD, LPCWSTR, LPCWSTR, BOOL, PVOID, DWORD*))::GetProcAddress(hWlanapi, "pfnWlanSetProfile"        );
            pfnWlanReasonCodeToString = (DWORD(WINAPI*)(DWORD, DWORD, PWCHAR, PVOID))                                      ::GetProcAddress(hWlanapi, "pfnWlanReasonCodeToString");
        }
    } else if (dwReason == DLL_PROCESS_DETACH) {
        if (hWlanapi)
            ::FreeLibrary(hWlanapi);
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////
// Exported functions
////////////////////////////////////////////////////////////////////

UINT MSICA_API MSICAInitialize(MSIHANDLE hInstall)
{
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICA"), MB_OK);

    UINT uiResult;
    winstd::com_initializer com_init(NULL);
    MSICA::COpList
        olInstallCertificates, olRemoveCertificates,
        olInstallWLANProfiles, olRemoveWLANProfiles,
        olInstallScheduledTask, olRemoveScheduledTask,
        olStopServices, olSetServiceStarts, olStartServices;
    BOOL bRollbackEnabled;
    PMSIHANDLE
        hDatabase,
        hRecordProg = ::MsiCreateRecord(3);
    winstd::tstring sValue;

    // Check and add the rollback enabled state.
    uiResult = ::MsiGetProperty(hInstall, _T("RollbackDisabled"), sValue);
    bRollbackEnabled = uiResult == NO_ERROR ?
        _ttoi(sValue.c_str()) || !sValue.empty() && _totlower(sValue[0]) == _T('y') ? FALSE : TRUE :
        TRUE;
    olRemoveScheduledTask.push_back(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olStopServices.push_back(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olRemoveWLANProfiles.push_back(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olRemoveCertificates.push_back(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olInstallCertificates.push_back(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olInstallWLANProfiles.push_back(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olSetServiceStarts.push_back(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olStartServices.push_back(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olInstallScheduledTask.push_back(new MSICA::COpRollbackEnable(bRollbackEnabled));

    // Open MSI database.
    hDatabase = ::MsiGetActiveDatabase(hInstall);
    if (hDatabase) {
        MSICONDITION condition;

        // Check if Certificate table exists. If it doesn't exist, there's nothing to do.
        condition = ::MsiDatabaseIsTablePersistent(hDatabase, _T("Certificate"));
        if (condition == MSICONDITION_FALSE || condition == MSICONDITION_TRUE) {
            PMSIHANDLE hViewCert;

            // Prepare a query to get a list/view of certificates.
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT `Binary_`,`Store`,`Flags`,`Encoding`,`Condition`,`Component_` FROM `Certificate`"), &hViewCert);
            if (uiResult == NO_ERROR) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewCert, NULL);
                if (uiResult == NO_ERROR) {
                    std::wstring sStore;
                    int iFlags, iEncoding;
                    std::vector<BYTE> binCert;

                    for (;;) {
                        PMSIHANDLE hRecord;
                        INSTALLSTATE iInstalled, iAction;
                        PMSIHANDLE hViewBinary;

                        // Fetch one record from the view.
                        uiResult = ::MsiViewFetch(hViewCert, &hRecord);
                        if (uiResult == ERROR_NO_MORE_ITEMS) {
                            uiResult = NO_ERROR;
                            break;
                        } else if (uiResult != NO_ERROR)
                            break;

                        // Read and evaluate certificate's condition.
                        uiResult = ::MsiRecordGetString(hRecord, 5, sValue);
                        if (uiResult != NO_ERROR) break;
                        condition = ::MsiEvaluateCondition(hInstall, sValue.c_str());
                        if (condition == MSICONDITION_FALSE)
                            continue;
                        else if (condition == MSICONDITION_ERROR) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }

                        // Perform another query to get certificate's binary data.
                        uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT `Data` FROM `Binary` WHERE `Name`=?"), &hViewBinary);
                        if (uiResult != NO_ERROR) break;

                        // Execute query!
                        uiResult = ::MsiViewExecute(hViewBinary, hRecord);
                        if (uiResult == NO_ERROR) {
                            PMSIHANDLE hRecordCert;

                            // Fetch one record from the view.
                            uiResult = ::MsiViewFetch(hViewBinary, &hRecordCert);
                            if (uiResult == NO_ERROR)
                                uiResult = ::MsiRecordReadStream(hRecordCert, 1, binCert);
                            ::MsiViewClose(hViewBinary);
                            if (uiResult != NO_ERROR) break;
                        } else
                            break;

                        // Read certificate's store.
                        uiResult = ::MsiRecordGetString(hRecord, 2, sStore);
                        if (uiResult != NO_ERROR) break;

                        // Read certificate's flags.
                        iFlags = ::MsiRecordGetInteger(hRecord, 3);
                        if (iFlags == MSI_NULL_INTEGER) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }

                        // Read certificate's encoding.
                        iEncoding = ::MsiRecordGetInteger(hRecord, 4);
                        if (iEncoding == MSI_NULL_INTEGER) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }

                        // Read certificate's Component ID.
                        uiResult = ::MsiRecordGetString(hRecord, 6, sValue);
                        if (uiResult != NO_ERROR) break;

                        // Get the component state.
                        uiResult = ::MsiGetComponentState(hInstall, sValue.c_str(), &iInstalled, &iAction);
                        if (uiResult != NO_ERROR) break;

                        if (iAction >= INSTALLSTATE_LOCAL) {
                            // Component is or should be installed. Install the certificate.
                            olInstallCertificates.push_back(new MSICA::COpCertInstall(binCert.data(), binCert.size(), sStore.c_str(), iEncoding, iFlags, MSICA_CERT_TICK_SIZE));
                        } else if (iAction >= INSTALLSTATE_REMOVED) {
                            // Component is installed, but should be degraded to advertised/removed. Delete the certificate.
                            olRemoveCertificates.push_back(new MSICA::COpCertRemove(binCert.data(), binCert.size(), sStore.c_str(), iEncoding, iFlags, MSICA_CERT_TICK_SIZE));
                        }

                        // The amount of tick space to add for each certificate to progress indicator.
                        ::MsiRecordSetInteger(hRecordProg, 1, 3                   );
                        ::MsiRecordSetInteger(hRecordProg, 2, MSICA_CERT_TICK_SIZE);
                        if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                    }
                    ::MsiViewClose(hViewCert);
                } else {
                    ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                    ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                }
            } else {
                ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
            }
        }

        // Check if ServiceConfigure table exists. If it doesn't exist, there's nothing to do.
        condition = ::MsiDatabaseIsTablePersistent(hDatabase, _T("ServiceConfigure"));
        if (condition == MSICONDITION_FALSE || condition == MSICONDITION_TRUE) {
            PMSIHANDLE hViewSC;

            // Prepare a query to get a list/view of service configurations.
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT `Name`,`StartType`,`Control`,`Condition` FROM `ServiceConfigure` ORDER BY `Sequence`"), &hViewSC);
            if (uiResult == NO_ERROR) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewSC, NULL);
                if (uiResult == NO_ERROR) {
                    int iValue;

                    for (;;) {
                        PMSIHANDLE hRecord;

                        // Fetch one record from the view.
                        uiResult = ::MsiViewFetch(hViewSC, &hRecord);
                        if (uiResult == ERROR_NO_MORE_ITEMS) {
                            uiResult = NO_ERROR;
                            break;
                        } else if (uiResult != NO_ERROR)
                            break;

                        // Read and evaluate service configuration condition.
                        uiResult = ::MsiRecordGetString(hRecord, 4, sValue);
                        if (uiResult != NO_ERROR) break;
                        condition = ::MsiEvaluateCondition(hInstall, sValue.c_str());
                        if (condition == MSICONDITION_FALSE)
                            continue;
                        else if (condition == MSICONDITION_ERROR) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }

                        // Read service name.
                        uiResult = ::MsiRecordGetString(hRecord, 1, sValue);
                        if (uiResult != NO_ERROR) break;

                        // Read service start type.
                        iValue = ::MsiRecordGetInteger(hRecord, 2);
                        if (iValue == MSI_NULL_INTEGER) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }
                        if (iValue >= 0) {
                            // Set service start type.
                            olSetServiceStarts.push_back(new MSICA::COpSvcSetStart(sValue.c_str(), iValue, MSICA_SVC_SET_START_TICK_SIZE));

                            // The amount of tick space to add to progress indicator.
                            ::MsiRecordSetInteger(hRecordProg, 1, 3                            );
                            ::MsiRecordSetInteger(hRecordProg, 2, MSICA_SVC_SET_START_TICK_SIZE);
                            if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                        }

                        // Read service control.
                        iValue = ::MsiRecordGetInteger(hRecord, 3);
                        if (iValue == MSI_NULL_INTEGER) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }
                        if ((iValue & 4) != 0) {
                            // Stop service.
                            olStopServices.push_back(new MSICA::COpSvcStop(sValue.c_str(), (iValue & 1) ? TRUE : FALSE, MSICA_SVC_STOP_TICK_SIZE));

                            // The amount of tick space to add to progress indicator.
                            ::MsiRecordSetInteger(hRecordProg, 1, 3                       );
                            ::MsiRecordSetInteger(hRecordProg, 2, MSICA_SVC_STOP_TICK_SIZE);
                            if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                        } else if ((iValue & 2) != 0) {
                            // Start service.
                            olStartServices.push_back(new MSICA::COpSvcStart(sValue.c_str(), (iValue & 1) ? TRUE : FALSE, MSICA_SVC_START_TICK_SIZE));

                            // The amount of tick space to add to progress indicator.
                            ::MsiRecordSetInteger(hRecordProg, 1, 3                        );
                            ::MsiRecordSetInteger(hRecordProg, 2, MSICA_SVC_START_TICK_SIZE);
                            if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                        }
                    }
                    ::MsiViewClose(hViewSC);
                } else {
                    ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                    ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                }
            } else {
                ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
            }
        }

        // Check if ScheduledTask table exists. If it doesn't exist, there's nothing to do.
        condition = ::MsiDatabaseIsTablePersistent(hDatabase, _T("ScheduledTask"));
        if (condition == MSICONDITION_FALSE || condition == MSICONDITION_TRUE) {
            PMSIHANDLE hViewST;

            // Prepare a query to get a list/view of tasks.
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT `Task`,`DisplayName`,`Application`,`Parameters`,`Directory_`,`Flags`,`Priority`,`User`,`Password`,`Author`,`Description`,`IdleMin`,`IdleDeadline`,`MaxRuntime`,`Condition`,`Component_` FROM `ScheduledTask`"), &hViewST);
            if (uiResult == NO_ERROR) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewST, NULL);
                if (uiResult == NO_ERROR) {
                    //winstd::tstring sComponent;
                    std::wstring sDisplayName;

                    for (;;) {
                        PMSIHANDLE hRecord;
                        INSTALLSTATE iInstalled, iAction;

                        // Fetch one record from the view.
                        uiResult = ::MsiViewFetch(hViewST, &hRecord);
                        if (uiResult == ERROR_NO_MORE_ITEMS) {
                            uiResult = NO_ERROR;
                            break;
                        } else if (uiResult != NO_ERROR)
                            break;

                        // Read and evaluate task's condition.
                        uiResult = ::MsiRecordGetString(hRecord, 15, sValue);
                        if (uiResult != NO_ERROR) break;
                        condition = ::MsiEvaluateCondition(hInstall, sValue.c_str());
                        if (condition == MSICONDITION_FALSE)
                            continue;
                        else if (condition == MSICONDITION_ERROR) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }

                        // Read task's Component ID.
                        uiResult = ::MsiRecordGetString(hRecord, 16, sValue);
                        if (uiResult != NO_ERROR) break;

                        // Get the component state.
                        uiResult = ::MsiGetComponentState(hInstall, sValue.c_str(), &iInstalled, &iAction);
                        if (uiResult != NO_ERROR) break;

                        // Get task's DisplayName.
                        uiResult = ::MsiRecordFormatStringW(hInstall, hRecord, 2, sDisplayName);

                        if (iAction >= INSTALLSTATE_LOCAL) {
                            // Component is or should be installed. Create the task.
                            PMSIHANDLE hViewTT;
                            MSICA::COpTaskCreate *opCreateTask = new MSICA::COpTaskCreate(sDisplayName.c_str(), MSICA_TASK_TICK_SIZE);

                            // Populate the operation with task's data.
                            uiResult = opCreateTask->SetFromRecord(hInstall, hRecord);
                            if (uiResult != NO_ERROR) break;

                            // Perform another query to get task's triggers.
                            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT `Trigger`,`BeginDate`,`EndDate`,`StartTime`,`StartTimeRand`,`MinutesDuration`,`MinutesInterval`,`Flags`,`Type`,`DaysInterval`,`WeeksInterval`,`DaysOfTheWeek`,`DaysOfMonth`,`WeekOfMonth`,`MonthsOfYear` FROM `TaskTrigger` WHERE `Task_`=?"), &hViewTT);
                            if (uiResult != NO_ERROR) break;

                            // Execute query!
                            uiResult = ::MsiViewExecute(hViewTT, hRecord);
                            if (uiResult == NO_ERROR) {
                                // Populate trigger list.
                                uiResult = opCreateTask->SetTriggersFromView(hViewTT);
                                ::MsiViewClose(hViewTT);
                                if (uiResult != NO_ERROR) break;
                            } else
                                break;

                            olInstallScheduledTask.push_back(opCreateTask);
                        } else if (iAction >= INSTALLSTATE_REMOVED) {
                            // Component is installed, but should be degraded to advertised/removed. Delete the task.
                            olRemoveScheduledTask.push_back(new MSICA::COpTaskDelete(sDisplayName.c_str(), MSICA_TASK_TICK_SIZE));
                        }

                        // The amount of tick space to add for each task to progress indicator.
                        ::MsiRecordSetInteger(hRecordProg, 1, 3                   );
                        ::MsiRecordSetInteger(hRecordProg, 2, MSICA_TASK_TICK_SIZE);
                        if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                    }
                    ::MsiViewClose(hViewST);
                } else {
                    ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                    ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                }
            } else {
                ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
            }
        }

        // Check if WLANProfile table exists. If it doesn't exist, there's nothing to do.
        condition = ::MsiDatabaseIsTablePersistent(hDatabase, _T("WLANProfile"));
        if (condition == MSICONDITION_FALSE || condition == MSICONDITION_TRUE) {
            PMSIHANDLE hViewProfile;

            // Prepare a query to get a list/view of profiles.
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT `Binary_`,`Name`,`Condition`,`Component_` FROM `WLANProfile`"), &hViewProfile);
            if (uiResult == NO_ERROR) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewProfile, NULL);
                if (uiResult == NO_ERROR) {
                    if (::pfnWlanOpenHandle && ::pfnWlanCloseHandle && ::pfnWlanEnumInterfaces && ::pfnWlanFreeMemory) {
                        DWORD dwError, dwNegotiatedVersion;
                        HANDLE hClientHandle;

                        // Open WLAN handle.
                        dwError = ::pfnWlanOpenHandle(2, NULL, &dwNegotiatedVersion, &hClientHandle);
                        if (dwError == NO_ERROR) {
                            WLAN_INTERFACE_INFO_LIST *pInterfaceList;

                            // Get a list of WLAN interfaces.
                            dwError = ::pfnWlanEnumInterfaces(hClientHandle, NULL, &pInterfaceList);
                            if (dwError == NO_ERROR) {
                                std::wstring sName, sProfileXML;
                                std::vector<BYTE> binProfile;

                                for (;;) {
                                    PMSIHANDLE hRecord;
                                    INSTALLSTATE iInstalled, iAction;
                                    int iTick = 0;
                                    DWORD i;

                                    // Fetch one record from the view.
                                    uiResult = ::MsiViewFetch(hViewProfile, &hRecord);
                                    if (uiResult == ERROR_NO_MORE_ITEMS) {
                                        uiResult = NO_ERROR;
                                        break;
                                    } else if (uiResult != NO_ERROR)
                                        break;

                                    // Read and evaluate profile's condition.
                                    uiResult = ::MsiRecordGetString(hRecord, 3, sValue);
                                    if (uiResult != NO_ERROR) break;
                                    condition = ::MsiEvaluateCondition(hInstall, sValue.c_str());
                                    if (condition == MSICONDITION_FALSE)
                                        continue;
                                    else if (condition == MSICONDITION_ERROR) {
                                        uiResult = ERROR_INVALID_FIELD;
                                        break;
                                    }

                                    // Read profile's name.
                                    uiResult = ::MsiRecordGetString(hRecord, 2, sName);
                                    if (uiResult != NO_ERROR) break;

                                    // Read profile's component ID.
                                    uiResult = ::MsiRecordGetString(hRecord, 4, sValue);
                                    if (uiResult != NO_ERROR) break;

                                    // Get the component state.
                                    uiResult = ::MsiGetComponentState(hInstall, sValue.c_str(), &iInstalled, &iAction);
                                    if (uiResult != NO_ERROR) break;

                                    if (iAction >= INSTALLSTATE_LOCAL) {
                                        PMSIHANDLE hViewBinary, hRecordBin;
                                        LPCWSTR pProfileStr;
                                        SIZE_T nCount;

                                        // Perform another query to get profile's binary data.
                                        uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT `Data` FROM `Binary` WHERE `Name`=?"), &hViewBinary);
                                        if (uiResult != NO_ERROR) break;

                                        // Execute query!
                                        uiResult = ::MsiViewExecute(hViewBinary, hRecord);
                                        if (uiResult != NO_ERROR) break;

                                        // Fetch one record from the view.
                                        uiResult = ::MsiViewFetch(hViewBinary, &hRecordBin);
                                        if (uiResult == NO_ERROR)
                                            uiResult = ::MsiRecordReadStream(hRecordBin, 1, binProfile);
                                        ::MsiViewClose(hViewBinary);
                                        if (uiResult != NO_ERROR) break;

                                        // Convert std::vector<BYTE> to std::wstring.
                                        pProfileStr = (LPCWSTR)binProfile.data();
                                        nCount = binProfile.size()/sizeof(WCHAR);

                                        if (nCount < 1 || pProfileStr[0] != 0xfeff) {
                                            // The profile XML is not UTF-16 encoded.
                                            ::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_WLAN_PROFILE_NOT_UTF16);
                                            ::MsiRecordSetStringW(hRecordProg, 2, sName.c_str()                       );
                                            ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                                            uiResult = ERROR_INSTALL_USEREXIT;
                                        }
                                        sProfileXML.assign(pProfileStr + 1, (int)nCount - 1);

                                        for (i = 0; i < pInterfaceList->dwNumberOfItems; i++) {
                                            // Check for not ready state in interface.
                                            if (pInterfaceList->InterfaceInfo[i].isState != wlan_interface_state_not_ready) {
                                                olInstallWLANProfiles.push_back(new MSICA::COpWLANProfileSet(pInterfaceList->InterfaceInfo[i].InterfaceGuid, 0, sName.c_str(), sProfileXML.c_str(), MSICA_WLAN_PROFILE_TICK_SIZE));
                                                iTick += MSICA_WLAN_PROFILE_TICK_SIZE;
                                            }
                                        }

                                        // The amount of tick space to add for each profile to progress indicator.
                                        ::MsiRecordSetInteger(hRecordProg, 1, 3    );
                                        ::MsiRecordSetInteger(hRecordProg, 2, iTick);
                                        if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                                    } else if (iAction >= INSTALLSTATE_REMOVED) {
                                        // Component is installed, but should be degraded to advertised/removed. Delete the profile from all interfaces.
                                        for (i = 0; i < pInterfaceList->dwNumberOfItems; i++) {
                                            // Check for not ready state in interface.
                                            if (pInterfaceList->InterfaceInfo[i].isState != wlan_interface_state_not_ready) {
                                                olRemoveWLANProfiles.push_back(new MSICA::COpWLANProfileDelete(pInterfaceList->InterfaceInfo[i].InterfaceGuid, sName.c_str(), MSICA_WLAN_PROFILE_TICK_SIZE));
                                                iTick += MSICA_WLAN_PROFILE_TICK_SIZE;
                                            }
                                        }

                                        // The amount of tick space to add for each profile to progress indicator.
                                        ::MsiRecordSetInteger(hRecordProg, 1, 3    );
                                        ::MsiRecordSetInteger(hRecordProg, 2, iTick);
                                        if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                                    }
                                }
                                ::pfnWlanFreeMemory(pInterfaceList);
                            }
                            ::pfnWlanCloseHandle(hClientHandle, NULL);
                        } else if (dwError == ERROR_SERVICE_NOT_ACTIVE) {
                            uiResult = ERROR_INSTALL_WLAN_SVC_NOT_STARTED;
                            ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                            ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                        } else {
                            uiResult = ERROR_INSTALL_WLAN_HANDLE_OPEN;
                            ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                            ::MsiRecordSetInteger(hRecordProg, 2, dwError);
                            ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                        }
                    } else {
                        uiResult = ERROR_INSTALL_WLAN_NOT_INSTALLED;
                        ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                    }
                    ::MsiViewClose(hViewProfile);
                } else {
                    ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                    ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                }
            } else {
                ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
            }
        }

        // Save the sequences.
        if (uiResult == NO_ERROR) uiResult = MSICA::SaveSequence(hInstall, _T("RemoveScheduledTasksExec"),  _T("RemoveScheduledTasksCommit"),  _T("RemoveScheduledTasksRollback"),  olRemoveScheduledTask);
        if (uiResult == NO_ERROR) uiResult = MSICA::SaveSequence(hInstall, _T("StopServicesExec"),          _T("StopServicesCommit"),          _T("StopServicesRollback"),          olStopServices);
        if (uiResult == NO_ERROR) uiResult = MSICA::SaveSequence(hInstall, _T("RemoveWLANProfilesExec"),    _T("RemoveWLANProfilesCommit"),    _T("RemoveWLANProfilesRollback"),    olRemoveWLANProfiles);
        if (uiResult == NO_ERROR) uiResult = MSICA::SaveSequence(hInstall, _T("RemoveCertificatesExec"),    _T("RemoveCertificatesCommit"),    _T("RemoveCertificatesRollback"),    olRemoveCertificates);
        if (uiResult == NO_ERROR) uiResult = MSICA::SaveSequence(hInstall, _T("InstallCertificatesExec"),   _T("InstallCertificatesCommit"),   _T("InstallCertificatesRollback"),   olInstallCertificates);
        if (uiResult == NO_ERROR) uiResult = MSICA::SaveSequence(hInstall, _T("InstallWLANProfilesExec"),   _T("InstallWLANProfilesCommit"),   _T("InstallWLANProfilesRollback"),   olInstallWLANProfiles);
        if (uiResult == NO_ERROR) uiResult = MSICA::SaveSequence(hInstall, _T("SetServiceStartExec"),       _T("SetServiceStartCommit"),       _T("SetServiceStartRollback"),       olSetServiceStarts);
        if (uiResult == NO_ERROR) uiResult = MSICA::SaveSequence(hInstall, _T("StartServicesExec"),         _T("StartServicesCommit"),         _T("StartServicesRollback"),         olStartServices);
        if (uiResult == NO_ERROR) uiResult = MSICA::SaveSequence(hInstall, _T("InstallScheduledTasksExec"), _T("InstallScheduledTasksCommit"), _T("InstallScheduledTasksRollback"), olInstallScheduledTask);

        if (uiResult != NO_ERROR && uiResult != ERROR_INSTALL_USEREXIT) {
            ::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_OPLIST_CREATE);
            ::MsiRecordSetInteger(hRecordProg, 2, uiResult                   );
            ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
        }
    } else {
        uiResult = ERROR_INSTALL_DATABASE_OPEN;
        ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
    }

    return uiResult;
}


UINT MSICA_API ExecuteSequence(MSIHANDLE hInstall)
{
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICA"), MB_OK);

    return MSICA::ExecuteSequence(hInstall);
}
