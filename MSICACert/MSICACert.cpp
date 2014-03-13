#include "stdafx.h"


////////////////////////////////////////////////////////////////////////////
// Local constants
////////////////////////////////////////////////////////////////////////////

#define MSICACERT_TASK_TICK_SIZE  (16*1024)


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

UINT MSICACERT_API EvaluateSequence(MSIHANDLE hInstall)
{
    UINT uiResult;
    HRESULT hr;
    BOOL bIsCoInitialized = SUCCEEDED(::CoInitialize(NULL));
    MSICA::COpList olExecute;
    BOOL bRollbackEnabled;
    PMSIHANDLE
        hDatabase,
        hRecordProg = ::MsiCreateRecord(3);
    ATL::CAtlString sValue;

    // Check and add the rollback enabled state.
    uiResult = ::MsiGetProperty(hInstall, _T("RollbackDisabled"), sValue);
    bRollbackEnabled = uiResult == ERROR_SUCCESS ?
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
            if (uiResult == ERROR_SUCCESS) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewST, NULL);
                if (uiResult == ERROR_SUCCESS) {
                    //ATL::CAtlString sComponent;
                    ATL::CAtlStringW sDisplayName;

                    for (;;) {
                        PMSIHANDLE hRecord;
                        INSTALLSTATE iInstalled, iAction;

                        // Fetch one record from the view.
                        uiResult = ::MsiViewFetch(hViewST, &hRecord);
                        if (uiResult == ERROR_NO_MORE_ITEMS) {
                            uiResult = ERROR_SUCCESS;
                            break;
                        } else if (uiResult != ERROR_SUCCESS)
                            break;

                        // Read and evaluate task's condition.
                        uiResult = ::MsiRecordGetString(hRecord, 15, sValue);
                        if (uiResult != ERROR_SUCCESS) break;
                        condition = ::MsiEvaluateCondition(hInstall, sValue);
                        if (condition == MSICONDITION_FALSE)
                            continue;
                        else if (condition == MSICONDITION_ERROR) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }

                        // Read task's Component ID.
                        uiResult = ::MsiRecordGetString(hRecord, 16, sValue);
                        if (uiResult != ERROR_SUCCESS) break;

                        // Get the component state.
                        uiResult = ::MsiGetComponentState(hInstall, sValue, &iInstalled, &iAction);
                        if (uiResult != ERROR_SUCCESS) break;

                        // Get task's DisplayName.
                        uiResult = ::MsiRecordFormatStringW(hInstall, hRecord, 2, sDisplayName);

                        if (iAction >= INSTALLSTATE_LOCAL) {
                            // Component is or should be installed. Create the task.
                            PMSIHANDLE hViewTT;
                            MSICA::COpTaskCreate *opCreateTask = new MSICA::COpTaskCreate(sDisplayName, MSICACERT_TASK_TICK_SIZE);

                            // Populate the operation with task's data.
                            uiResult = opCreateTask->SetFromRecord(hInstall, hRecord);
                            if (uiResult != ERROR_SUCCESS) break;

                            // Perform another query to get task's triggers.
                            uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Trigger,BeginDate,EndDate,StartTime,StartTimeRand,MinutesDuration,MinutesInterval,Flags,Type,DaysInterval,WeeksInterval,DaysOfTheWeek,DaysOfMonth,WeekOfMonth,MonthsOfYear FROM TaskTrigger WHERE Task_=?"), &hViewTT);
                            if (uiResult != ERROR_SUCCESS) break;

                            // Execute query!
                            uiResult = ::MsiViewExecute(hViewTT, hRecord);
                            if (uiResult == ERROR_SUCCESS) {
                                // Populate trigger list.
                                uiResult = opCreateTask->SetTriggersFromView(hViewTT);
                                ::MsiViewClose(hViewTT);
                                if (uiResult != ERROR_SUCCESS) break;
                            } else
                                break;

                            olExecute.AddTail(opCreateTask);
                        } else if (iAction >= INSTALLSTATE_REMOVED) {
                            // Component is installed, but should be degraded to advertised/removed. Delete the task.
                            olExecute.AddTail(new MSICA::COpTaskDelete(sDisplayName, MSICACERT_TASK_TICK_SIZE));
                        }

                        // The amount of tick space to add for each task to progress indicator.
                        ::MsiRecordSetInteger(hRecordProg, 1, 3                       );
                        ::MsiRecordSetInteger(hRecordProg, 2, MSICACERT_TASK_TICK_SIZE);
                        if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                    }
                    ::MsiViewClose(hViewST);

                    if (SUCCEEDED(uiResult)) {
                        ATL::CAtlString sSequenceFilename;
                        ATL::CAtlFile fSequence;

                        // Prepare our own sequence script file.
                        // The InstallCertificates is a deferred custom action, thus all this information will be unavailable to it.
                        // Therefore save all required info to file now.
                        {
                            LPTSTR szBuffer = sSequenceFilename.GetBuffer(MAX_PATH);
                            ::GetTempPath(MAX_PATH, szBuffer);
                            ::GetTempFileName(szBuffer, _T("TS"), 0, szBuffer);
                            sSequenceFilename.ReleaseBuffer();
                        }
                        // Save execute sequence to file.
                        hr = olExecute.SaveToFile(sSequenceFilename);
                        if (SUCCEEDED(hr)) {
                            // Store sequence script file names to properties for deferred custiom actions.
                            uiResult = ::MsiSetProperty(hInstall, _T("InstallCertificates"), sSequenceFilename);
                            if (uiResult == ERROR_SUCCESS) {
                                LPCTSTR pszExtension = ::PathFindExtension(sSequenceFilename);
                                ATL::CAtlString sSequenceFilename2;

                                sSequenceFilename2.Format(_T("%.*ls-rb%ls"), pszExtension - (LPCTSTR)sSequenceFilename, (LPCTSTR)sSequenceFilename, pszExtension);
                                uiResult = ::MsiSetProperty(hInstall, _T("RollbackCertificates"), sSequenceFilename2);
                                if (uiResult == ERROR_SUCCESS) {
                                    sSequenceFilename2.Format(_T("%.*ls-cm%ls"), pszExtension - (LPCTSTR)sSequenceFilename, (LPCTSTR)sSequenceFilename, pszExtension);
                                    uiResult = ::MsiSetProperty(hInstall, _T("CommitCertificates"), sSequenceFilename2);
                                    if (uiResult != ERROR_SUCCESS) {
                                        ::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_PROPERTY_SET);
                                        ::MsiRecordSetString (hRecordProg, 2, _T("CommitCertificates"));
                                        ::MsiRecordSetInteger(hRecordProg, 3, uiResult                );
                                        ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                                    }
                                } else {
                                    ::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_PROPERTY_SET  );
                                    ::MsiRecordSetString (hRecordProg, 2, _T("RollbackCertificates"));
                                    ::MsiRecordSetInteger(hRecordProg, 3, uiResult                  );
                                    ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                                }
                            } else {
                                ::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_PROPERTY_SET );
                                ::MsiRecordSetString (hRecordProg, 2, _T("InstallCertificates"));
                                ::MsiRecordSetInteger(hRecordProg, 3, uiResult                 );
                                ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                            }
                            if (uiResult != ERROR_SUCCESS) ::DeleteFile(sSequenceFilename);
                        } else {
                            uiResult = ERROR_INSTALL_SCRIPT_WRITE;
                            ::MsiRecordSetInteger(hRecordProg, 1, uiResult         );
                            ::MsiRecordSetString (hRecordProg, 2, sSequenceFilename);
                            ::MsiRecordSetInteger(hRecordProg, 3, hr               );
                            ::MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
                        }
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


UINT MSICACERT_API ExecuteSequence(MSIHANDLE hInstall)
{
    return MSICA::ExecuteSequence(hInstall);
}
