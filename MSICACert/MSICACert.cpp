#include "stdafx.h"


////////////////////////////////////////////////////////////////////////////
// Local constants
////////////////////////////////////////////////////////////////////////////

#define MSICACERT_CERT_TICK_SIZE  (16*1024)


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
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICACert"), MB_OK);

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
    bRollbackEnabled = uiResult == ERROR_SUCCESS ?
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
            if (uiResult == ERROR_SUCCESS) {
                // Execute query!
                uiResult = ::MsiViewExecute(hViewCert, NULL);
                if (uiResult == ERROR_SUCCESS) {
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
                            uiResult = ERROR_SUCCESS;
                            break;
                        } else if (uiResult != ERROR_SUCCESS)
                            break;

                        // Read and evaluate certificate's condition.
                        uiResult = ::MsiRecordGetString(hRecord, 5, sValue);
                        if (uiResult != ERROR_SUCCESS) break;
                        condition = ::MsiEvaluateCondition(hInstall, sValue);
                        if (condition == MSICONDITION_FALSE)
                            continue;
                        else if (condition == MSICONDITION_ERROR) {
                            uiResult = ERROR_INVALID_FIELD;
                            break;
                        }

                        // Perform another query to get certificate's binary data.
                        uiResult = ::MsiDatabaseOpenView(hDatabase, _T("SELECT Data FROM Binary WHERE Name=?"), &hViewBinary);
                        if (uiResult != ERROR_SUCCESS) break;

                        // Execute query!
                        uiResult = ::MsiViewExecute(hViewBinary, hRecord);
                        if (uiResult == ERROR_SUCCESS) {
                            PMSIHANDLE hRecord;

                            // Fetch one record from the view.
                            uiResult = ::MsiViewFetch(hViewBinary, &hRecord);
                            if (uiResult == ERROR_SUCCESS)
                                uiResult = ::MsiRecordGetStream(hRecord, 1, binCert);
                            ::MsiViewClose(hViewBinary);
                            if (uiResult != ERROR_SUCCESS) break;
                        } else
                            break;

                        // Read certificate's store.
                        uiResult = ::MsiRecordGetString(hRecord, 2, sStore);
                        if (uiResult != ERROR_SUCCESS) break;

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
                        if (uiResult != ERROR_SUCCESS) break;

                        // Get the component state.
                        uiResult = ::MsiGetComponentState(hInstall, sValue, &iInstalled, &iAction);
                        if (uiResult != ERROR_SUCCESS) break;

                        if (iAction >= INSTALLSTATE_LOCAL) {
                            // Component is or should be installed. Install the certificate.
                            olExecute.AddTail(new MSICA::COpCertInstall(binCert.GetData(), binCert.GetCount(), sStore, iEncoding, iFlags, MSICACERT_CERT_TICK_SIZE));
                        } else if (iAction >= INSTALLSTATE_REMOVED) {
                            // Component is installed, but should be degraded to advertised/removed. Delete the certificate.
                            olExecute.AddTail(new MSICA::COpCertRemove(binCert.GetData(), binCert.GetCount(), sStore, iEncoding, iFlags, MSICACERT_CERT_TICK_SIZE));
                        }

                        // The amount of tick space to add for each certificate to progress indicator.
                        ::MsiRecordSetInteger(hRecordProg, 1, 3                       );
                        ::MsiRecordSetInteger(hRecordProg, 2, MSICACERT_CERT_TICK_SIZE);
                        if (::MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL) { uiResult = ERROR_INSTALL_USEREXIT; break; }
                    }
                    ::MsiViewClose(hViewCert);

                    if (uiResult == ERROR_SUCCESS) {
                        // Save the sequences.
                        uiResult = MSICA::SaveSequence(hInstall, _T("InstallCertificates"), _T("CommitCertificates"), _T("RollbackCertificates"), olExecute);
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
    //::MessageBox(NULL, _T(__FUNCTION__), _T("MSICACert"), MB_OK);

    return MSICA::ExecuteSequence(hInstall);
}
