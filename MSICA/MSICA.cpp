#include "stdafx.h"

#pragma comment(lib, "wlanapi.lib")


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
// Global functions
////////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(lpReserved);

    return TRUE;
}


////////////////////////////////////////////////////////////////////
// Exported functions
////////////////////////////////////////////////////////////////////

UINT MSICA_API MSICAInitialize(MSIHANDLE hInstall)
{
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICA"), MB_OK);

    UINT uiResult;
    BOOL bIsCoInitialized = SUCCEEDED(::CoInitialize(NULL));
    MSICA::COpList
        olInstallCertificates, olRemoveCertificates,
        olInstallWLANProfiles, olRemoveWLANProfiles,
        olInstallScheduledTask, olRemoveScheduledTask,
        olStopServices, olSetServiceStarts, olStartServices;
    BOOL bRollbackEnabled;
    PMSIHANDLE
        hDatabase,
        hRecordProg = ::MsiCreateRecord(3);
    ATL::CAtlString sValue;

    // Check and add the rollback enabled state.
    uiResult = ::MsiGetProperty(hInstall, _T("RollbackDisabled"), sValue);
    bRollbackEnabled = uiResult == NO_ERROR ?
        _ttoi(sValue) || !sValue.IsEmpty() && _totlower(sValue.GetAt(0)) == _T('y') ? FALSE : TRUE :
        TRUE;
    olRemoveScheduledTask.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olStopServices.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olRemoveWLANProfiles.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olRemoveCertificates.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olInstallCertificates.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olInstallWLANProfiles.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olSetServiceStarts.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olStartServices.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));
    olInstallScheduledTask.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));

    // Open MSI database.
    hDatabase = ::MsiGetActiveDatabase(hInstall);
    if (hDatabase) {
        MSICONDITION condition;

        // Check if Certificate table exists. If it doesn't exist, there's nothing to do.
        condition = ::MsiDatabaseIsTablePersistent(hDatabase, _T("Certificate"));
        if (condition == MSICONDITION_FALSE || condition == MSICONDITION_TRUE) {
            PMSIHANDLE hViewCert;

            // Prepare a query to get a list/view of certificates.
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Binary_,Store,Flags,Encoding,Condition,Component_ FROM Certificate"), &hViewCert);
            if (uiResult == NO_ERROR) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewCert, NULL);
                if (uiResult == NO_ERROR) {
                    ATL::CAtlStringW sStore;
                    int iFlags, iEncoding;
                    ATL::CAtlArray<BYTE> binCert;

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
                        condition = ::MsiEvaluateCondition(hInstall, sValue);
                        if (condition == MSICONDITION_FALSE)
                            continue;
                        else if (condition == MSICONDITION_ERROR) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }

                        // Perform another query to get certificate's binary data.
                        uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Data FROM Binary WHERE Name=?"), &hViewBinary);
                        if (uiResult != NO_ERROR) break;

                        // Execute query!
                        uiResult = ::MsiViewExecute(hViewBinary, hRecord);
                        if (uiResult == NO_ERROR) {
                            PMSIHANDLE hRecord;

                            // Fetch one record from the view.
                            uiResult = ::MsiViewFetch(hViewBinary, &hRecord);
                            if (uiResult == NO_ERROR)
                                uiResult = ::MsiRecordGetStream(hRecord, 1, binCert);
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
                        uiResult = ::MsiGetComponentState(hInstall, sValue, &iInstalled, &iAction);
                        if (uiResult != NO_ERROR) break;

                        if (iAction >= INSTALLSTATE_LOCAL) {
                            // Component is or should be installed. Install the certificate.
                            olInstallCertificates.AddTail(new MSICA::COpCertInstall(binCert.GetData(), binCert.GetCount(), sStore, iEncoding, iFlags, MSICA_CERT_TICK_SIZE));
                        } else if (iAction >= INSTALLSTATE_REMOVED) {
                            // Component is installed, but should be degraded to advertised/removed. Delete the certificate.
                            olRemoveCertificates.AddTail(new MSICA::COpCertRemove(binCert.GetData(), binCert.GetCount(), sStore, iEncoding, iFlags, MSICA_CERT_TICK_SIZE));
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
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Name,StartType,Control,Condition FROM ServiceConfigure ORDER BY Sequence"), &hViewSC);
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
                        condition = ::MsiEvaluateCondition(hInstall, sValue);
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
                            olSetServiceStarts.AddTail(new MSICA::COpSvcSetStart(sValue, iValue, MSICA_SVC_SET_START_TICK_SIZE));

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
                            olStopServices.AddTail(new MSICA::COpSvcStop(sValue, (iValue & 1) ? TRUE : FALSE, MSICA_SVC_STOP_TICK_SIZE));

                            // The amount of tick space to add to progress indicator.
                            ::MsiRecordSetInteger(hRecordProg, 1, 3                       );
                            ::MsiRecordSetInteger(hRecordProg, 2, MSICA_SVC_STOP_TICK_SIZE);
                            if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                        } else if ((iValue & 2) != 0) {
                            // Start service.
                            olStartServices.AddTail(new MSICA::COpSvcStart(sValue, (iValue & 1) ? TRUE : FALSE, MSICA_SVC_START_TICK_SIZE));

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
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Task,DisplayName,Application,Parameters,Directory_,Flags,Priority,User,Password,Author,Description,IdleMin,IdleDeadline,MaxRuntime,Condition,Component_ FROM ScheduledTask"), &hViewST);
            if (uiResult == NO_ERROR) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewST, NULL);
                if (uiResult == NO_ERROR) {
                    //ATL::CAtlString sComponent;
                    ATL::CAtlStringW sDisplayName;

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
                        condition = ::MsiEvaluateCondition(hInstall, sValue);
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
                        uiResult = ::MsiGetComponentState(hInstall, sValue, &iInstalled, &iAction);
                        if (uiResult != NO_ERROR) break;

                        // Get task's DisplayName.
                        uiResult = ::MsiRecordFormatStringW(hInstall, hRecord, 2, sDisplayName);

                        if (iAction >= INSTALLSTATE_LOCAL) {
                            // Component is or should be installed. Create the task.
                            PMSIHANDLE hViewTT;
                            MSICA::COpTaskCreate *opCreateTask = new MSICA::COpTaskCreate(sDisplayName, MSICA_TASK_TICK_SIZE);

                            // Populate the operation with task's data.
                            uiResult = opCreateTask->SetFromRecord(hInstall, hRecord);
                            if (uiResult != NO_ERROR) break;

                            // Perform another query to get task's triggers.
                            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Trigger,BeginDate,EndDate,StartTime,StartTimeRand,MinutesDuration,MinutesInterval,Flags,Type,DaysInterval,WeeksInterval,DaysOfTheWeek,DaysOfMonth,WeekOfMonth,MonthsOfYear FROM TaskTrigger WHERE Task_=?"), &hViewTT);
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

                            olInstallScheduledTask.AddTail(opCreateTask);
                        } else if (iAction >= INSTALLSTATE_REMOVED) {
                            // Component is installed, but should be degraded to advertised/removed. Delete the task.
                            olRemoveScheduledTask.AddTail(new MSICA::COpTaskDelete(sDisplayName, MSICA_TASK_TICK_SIZE));
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
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Binary_,Name,Condition,Component_ FROM WLANProfile"), &hViewProfile);
            if (uiResult == NO_ERROR) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewProfile, NULL);
                if (uiResult == NO_ERROR) {
                    DWORD dwError, dwNegotiatedVersion;
                    HANDLE hClientHandle;

                    // Open WLAN handle.
                    dwError = ::WlanOpenHandle(2, NULL, &dwNegotiatedVersion, &hClientHandle);
                    if (dwError == NO_ERROR) {
                        WLAN_INTERFACE_INFO_LIST *pInterfaceList;

                        // Get a list of WLAN interfaces.
                        dwError = ::WlanEnumInterfaces(hClientHandle, NULL, &pInterfaceList);
                        if (dwError == NO_ERROR) {
                            ATL::CAtlStringW sName, sProfileXML;
                            ATL::CAtlArray<BYTE> binProfile;

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
                                condition = ::MsiEvaluateCondition(hInstall, sValue);
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
                                uiResult = ::MsiGetComponentState(hInstall, sValue, &iInstalled, &iAction);
                                if (uiResult != NO_ERROR) break;

                                if (iAction >= INSTALLSTATE_LOCAL) {
                                    PMSIHANDLE hViewBinary, hRecordBin;
                                    LPCWSTR pProfileStr;
                                    SIZE_T nCount;

                                    // Perform another query to get profile's binary data.
                                    uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Data FROM Binary WHERE Name=?"), &hViewBinary);
                                    if (uiResult != NO_ERROR) break;

                                    // Execute query!
                                    uiResult = ::MsiViewExecute(hViewBinary, hRecord);
                                    if (uiResult != NO_ERROR) break;

                                    // Fetch one record from the view.
                                    uiResult = ::MsiViewFetch(hViewBinary, &hRecordBin);
                                    if (uiResult == NO_ERROR)
                                        uiResult = ::MsiRecordGetStream(hRecordBin, 1, binProfile);
                                    ::MsiViewClose(hViewBinary);
                                    if (uiResult != NO_ERROR) break;

                                    // Convert CAtlArray<BYTE> to CAtlStringW.
                                    pProfileStr = (LPCWSTR)binProfile.GetData();
                                    nCount = binProfile.GetCount()/sizeof(WCHAR);

                                    if (nCount < 1 || pProfileStr[0] != 0xfeff) {
                                        // The profile XML is not UTF-16 encoded.
                                        ::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_WLAN_PROFILE_NOT_UTF16);
                                        ::MsiRecordSetStringW(hRecordProg, 2, sName                               );
                                        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                                        uiResult = ERROR_INSTALL_USEREXIT;
                                    }
                                    sProfileXML.SetString(pProfileStr + 1, (int)nCount - 1);

                                    for (i = 0; i < pInterfaceList->dwNumberOfItems; i++) {
                                        // Check for not ready state in interface.
                                        if (pInterfaceList->InterfaceInfo[i].isState != wlan_interface_state_not_ready) {
                                            olInstallWLANProfiles.AddTail(new MSICA::COpWLANProfileSet(pInterfaceList->InterfaceInfo[i].InterfaceGuid, 0, sName, sProfileXML, MSICA_WLAN_PROFILE_TICK_SIZE));
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
                                            olRemoveWLANProfiles.AddTail(new MSICA::COpWLANProfileDelete(pInterfaceList->InterfaceInfo[i].InterfaceGuid, sName, MSICA_WLAN_PROFILE_TICK_SIZE));
                                            iTick += MSICA_WLAN_PROFILE_TICK_SIZE;
                                        }
                                    }

                                    // The amount of tick space to add for each profile to progress indicator.
                                    ::MsiRecordSetInteger(hRecordProg, 1, 3    );
                                    ::MsiRecordSetInteger(hRecordProg, 2, iTick);
                                    if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                                }
                            }
                            ::WlanFreeMemory(pInterfaceList);
                        }
                        ::WlanCloseHandle(hClientHandle, NULL);
                    } else {
                        uiResult = ERROR_INSTALL_WLAN_HANDLE_OPEN;
                        ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                        ::MsiRecordSetInteger(hRecordProg, 2, dwError);
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

    olInstallScheduledTask.Free();
    olStartServices.Free();
    olSetServiceStarts.Free();
    olInstallWLANProfiles.Free();
    olInstallCertificates.Free();
    olRemoveCertificates.Free();
    olRemoveWLANProfiles.Free();
    olStopServices.Free();
    olRemoveScheduledTask.Free();

    if (bIsCoInitialized) ::CoUninitialize();
    return uiResult;
}


UINT MSICA_API ExecuteSequence(MSIHANDLE hInstall)
{
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICA"), MB_OK);

    return MSICA::ExecuteSequence(hInstall);
}
