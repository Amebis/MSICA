#include "stdafx.h"


////////////////////////////////////////////////////////////////////////////
// Local constants
////////////////////////////////////////////////////////////////////////////

#define MSICA_CERT_TICK_SIZE            (4*1024)
#define MSICA_SVC_SET_START_TICK_SIZE   (1*1024)
#define MSICA_TASK_TICK_SIZE            (16*1024)


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

UINT MSICA_API CertificatesEval(MSIHANDLE hInstall)
{
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICA"), MB_OK);

    UINT uiResult;
    BOOL bIsCoInitialized = SUCCEEDED(::CoInitialize(NULL));
    MSICA::COpList olExecute;
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
    olExecute.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));

    // Open MSI database.
    hDatabase = ::MsiGetActiveDatabase(hInstall);
    if (hDatabase) {
        // Check if Certificate table exists. If it doesn't exist, there's nothing to do.
        MSICONDITION condition = ::MsiDatabaseIsTablePersistent(hDatabase, _T("Certificate"));
        if (condition == MSICONDITION_FALSE || condition == MSICONDITION_TRUE) {
            PMSIHANDLE hViewCert;

            // Prepare a query to get a list/view of certificates.
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Binary_,Store,Flags,Encoding,Condition,Component_ FROM Certificate"), &hViewCert);
            if (uiResult == NO_ERROR) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewCert, NULL);
                if (uiResult == NO_ERROR) {
                    //ATL::CAtlString sComponent;
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
                            olExecute.AddTail(new MSICA::COpCertInstall(binCert.GetData(), binCert.GetCount(), sStore, iEncoding, iFlags, MSICA_CERT_TICK_SIZE));
                        } else if (iAction >= INSTALLSTATE_REMOVED) {
                            // Component is installed, but should be degraded to advertised/removed. Delete the certificate.
                            olExecute.AddTail(new MSICA::COpCertRemove(binCert.GetData(), binCert.GetCount(), sStore, iEncoding, iFlags, MSICA_CERT_TICK_SIZE));
                        }

                        // The amount of tick space to add for each certificate to progress indicator.
                        ::MsiRecordSetInteger(hRecordProg, 1, 3                   );
                        ::MsiRecordSetInteger(hRecordProg, 2, MSICA_CERT_TICK_SIZE);
                        if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                    }
                    ::MsiViewClose(hViewCert);

                    if (uiResult == NO_ERROR) {
                        // Save the sequences.
                        uiResult = MSICA::SaveSequence(hInstall, _T("CertificatesExec"), _T("CertificatesCommit"), _T("CertificatesRollback"), olExecute);
                    } else if (uiResult != ERROR_INSTALL_USEREXIT) {
                        ::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_OPLIST_CREATE);
                        ::MsiRecordSetInteger(hRecordProg, 2, uiResult                   );
                        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                    }
                } else {
                    ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                    ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                }
            } else {
                ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
            }
        }
    } else {
        uiResult = ERROR_INSTALL_DATABASE_OPEN;
        ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
    }

    olExecute.Free();
    if (bIsCoInitialized) ::CoUninitialize();
    return uiResult;
}


UINT MSICA_API ServiceConfigEval(MSIHANDLE hInstall)
{
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICA"), MB_OK);

    UINT uiResult;
    BOOL bIsCoInitialized = SUCCEEDED(::CoInitialize(NULL));
    MSICA::COpList olExecute;
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
    olExecute.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));

    // Open MSI database.
    hDatabase = ::MsiGetActiveDatabase(hInstall);
    if (hDatabase) {
        // Check if ServiceConfigure table exists. If it doesn't exist, there's nothing to do.
        MSICONDITION condition = ::MsiDatabaseIsTablePersistent(hDatabase, _T("ServiceConfigure"));
        if (condition == MSICONDITION_FALSE || condition == MSICONDITION_TRUE) {
            PMSIHANDLE hViewSC;

            // Prepare a query to get a list/view of service configurations.
            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Name,StartType,Condition FROM ServiceConfigure ORDER BY Sequence"), &hViewSC);
            if (uiResult == NO_ERROR) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewSC, NULL);
                if (uiResult == NO_ERROR) {
                    int iStartType;

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
                        uiResult = ::MsiRecordGetString(hRecord, 3, sValue);
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
                        iStartType = ::MsiRecordGetInteger(hRecord, 2);
                        if (iStartType == MSI_NULL_INTEGER) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }
                        if (iStartType >= 0) {
                            // Set service start type.
                            olExecute.AddTail(new MSICA::COpSvcSetStart(sValue, iStartType, MSICA_SVC_SET_START_TICK_SIZE));

                            // The amount of tick space to add to progress indicator.
                            ::MsiRecordSetInteger(hRecordProg, 1, 3                            );
                            ::MsiRecordSetInteger(hRecordProg, 2, MSICA_SVC_SET_START_TICK_SIZE);
                            if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                        }
                    }
                    ::MsiViewClose(hViewSC);

                    if (uiResult == NO_ERROR) {
                        // Save the sequences.
                        uiResult = MSICA::SaveSequence(hInstall, _T("ServiceConfigExec"), _T("ServiceConfigCommit"), _T("ServiceConfigRollback"), olExecute);
                    } else if (uiResult != ERROR_INSTALL_USEREXIT) {
                        ::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_OPLIST_CREATE);
                        ::MsiRecordSetInteger(hRecordProg, 2, uiResult                   );
                        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                    }
                } else {
                    ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                    ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                }
            } else {
                ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
            }
        }
    } else {
        uiResult = ERROR_INSTALL_DATABASE_OPEN;
        ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
    }

    olExecute.Free();
    if (bIsCoInitialized) ::CoUninitialize();
    return uiResult;
}


UINT MSICA_API ScheduledTasksEval(MSIHANDLE hInstall)
{
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICA"), MB_OK);

    UINT uiResult;
    BOOL bIsCoInitialized = SUCCEEDED(::CoInitialize(NULL));
    MSICA::COpList olExecute;
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
    olExecute.AddTail(new MSICA::COpRollbackEnable(bRollbackEnabled));

    // Open MSI database.
    hDatabase = ::MsiGetActiveDatabase(hInstall);
    if (hDatabase) {
        // Check if ScheduledTask table exists. If it doesn't exist, there's nothing to do.
        MSICONDITION condition = ::MsiDatabaseIsTablePersistent(hDatabase, _T("ScheduledTask"));
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

                            olExecute.AddTail(opCreateTask);
                        } else if (iAction >= INSTALLSTATE_REMOVED) {
                            // Component is installed, but should be degraded to advertised/removed. Delete the task.
                            olExecute.AddTail(new MSICA::COpTaskDelete(sDisplayName, MSICA_TASK_TICK_SIZE));
                        }

                        // The amount of tick space to add for each task to progress indicator.
                        ::MsiRecordSetInteger(hRecordProg, 1, 3                   );
                        ::MsiRecordSetInteger(hRecordProg, 2, MSICA_TASK_TICK_SIZE);
                        if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                    }
                    ::MsiViewClose(hViewST);

                    if (uiResult == NO_ERROR) {
                        // Save the sequences.
                        uiResult = MSICA::SaveSequence(hInstall, _T("ScheduledTasksExec"), _T("ScheduledTasksCommit"), _T("ScheduledTasksRollback"), olExecute);
                    } else if (uiResult != ERROR_INSTALL_USEREXIT) {
                        ::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_OPLIST_CREATE);
                        ::MsiRecordSetInteger(hRecordProg, 2, uiResult                   );
                        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                    }
                } else {
                    ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                    ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                }
            } else {
                ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
                ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
            }
        }
    } else {
        uiResult = ERROR_INSTALL_DATABASE_OPEN;
        ::MsiRecordSetInteger(hRecordProg, 1, uiResult);
        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
    }

    olExecute.Free();
    if (bIsCoInitialized) ::CoUninitialize();
    return uiResult;
}


UINT MSICA_API ExecuteSequence(MSIHANDLE hInstall)
{
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICA"), MB_OK);

    return MSICA::ExecuteSequence(hInstall);
}
