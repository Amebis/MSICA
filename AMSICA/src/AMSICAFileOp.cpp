#include "StdAfx.h"


namespace AMSICA {

////////////////////////////////////////////////////////////////////////////
// COpFileDelete
////////////////////////////////////////////////////////////////////////////

COpFileDelete::COpFileDelete(LPCWSTR pszFileName, int iTicks) :
    COpTypeSingleString(pszFileName, iTicks)
{
}


HRESULT COpFileDelete::Execute(CSession *pSession)
{
    assert(pSession);
    DWORD dwError;

    if (pSession->m_bRollbackEnabled) {
        ATL::CAtlStringW sBackupName;
        UINT uiCount = 0;

        do {
            // Rename the file to make a backup.
            sBackupName.Format(L"%ls (orig %u)", (LPCWSTR)m_sValue, ++uiCount);
            dwError = ::MoveFileW(m_sValue, sBackupName) ? ERROR_SUCCESS : ::GetLastError();
        } while (dwError == ERROR_ALREADY_EXISTS);
        if (dwError == ERROR_SUCCESS) {
            // Order rollback action to restore from backup copy.
            pSession->m_olRollback.AddHead(new COpFileMove(sBackupName, m_sValue));

            // Order commit action to delete backup copy.
            pSession->m_olCommit.AddTail(new COpFileDelete(sBackupName));
        }
    } else {
        // Delete the file.
        dwError = ::DeleteFileW(m_sValue) ? ERROR_SUCCESS : ::GetLastError();
    }

    if (dwError == ERROR_SUCCESS || dwError == ERROR_FILE_NOT_FOUND)
        return S_OK;
    else {
        PMSIHANDLE hRecordProg = ::MsiCreateRecord(3);
        verify(::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_DELETE_FAILED) == ERROR_SUCCESS);
        verify(::MsiRecordSetStringW(hRecordProg, 2, m_sValue                   ) == ERROR_SUCCESS);
        verify(::MsiRecordSetInteger(hRecordProg, 3, dwError                    ) == ERROR_SUCCESS);
        ::MsiProcessMessage(pSession->m_hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
        return AtlHresultFromWin32(dwError);
    }
}


////////////////////////////////////////////////////////////////////////////
// COpFileMove
////////////////////////////////////////////////////////////////////////////

COpFileMove::COpFileMove(LPCWSTR pszFileSrc, LPCWSTR pszFileDst, int iTicks) :
    COpTypeSrcDstString(pszFileSrc, pszFileDst, iTicks)
{
}


HRESULT COpFileMove::Execute(CSession *pSession)
{
    assert(pSession);
    DWORD dwError;

    // Move the file.
    dwError = ::MoveFileW(m_sValue1, m_sValue2) ? ERROR_SUCCESS : ::GetLastError();
    if (dwError == ERROR_SUCCESS) {
        if (pSession->m_bRollbackEnabled) {
            // Order rollback action to move it back.
            pSession->m_olRollback.AddHead(new COpFileMove(m_sValue2, m_sValue1));
        }

        return S_OK;
    } else {
        PMSIHANDLE hRecordProg = ::MsiCreateRecord(4);
        verify(::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_MOVE_FAILED) == ERROR_SUCCESS);
        verify(::MsiRecordSetStringW(hRecordProg, 2, m_sValue1                ) == ERROR_SUCCESS);
        verify(::MsiRecordSetStringW(hRecordProg, 3, m_sValue2                ) == ERROR_SUCCESS);
        verify(::MsiRecordSetInteger(hRecordProg, 4, dwError                  ) == ERROR_SUCCESS);
        ::MsiProcessMessage(pSession->m_hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
        return AtlHresultFromWin32(dwError);
    }
}

} // namespace AMSICA
