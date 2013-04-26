#include "StdAfx.h"


namespace AMSICA {


////////////////////////////////////////////////////////////////////////////
// COpRegKeySingle
////////////////////////////////////////////////////////////////////////////

COpRegKeySingle::COpRegKeySingle(HKEY hKeyRoot, LPCWSTR pszKeyName, int iTicks) :
    m_hKeyRoot(hKeyRoot),
    COpTypeSingleString(pszKeyName, iTicks)
{
}


////////////////////////////////////////////////////////////////////////////
// COpRegKeySrcDst
////////////////////////////////////////////////////////////////////////////

COpRegKeySrcDst::COpRegKeySrcDst(HKEY hKeyRoot, LPCWSTR pszKeyNameSrc, LPCWSTR pszKeyNameDst, int iTicks) : 
    m_hKeyRoot(hKeyRoot),
    COpTypeSrcDstString(pszKeyNameSrc, pszKeyNameDst, iTicks)
{
}


////////////////////////////////////////////////////////////////////////////
// COpRegKeyCreate
////////////////////////////////////////////////////////////////////////////

COpRegKeyCreate::COpRegKeyCreate(HKEY hKeyRoot, LPCWSTR pszKeyName, int iTicks) : COpRegKeySingle(hKeyRoot, pszKeyName, iTicks)
{
}


HRESULT COpRegKeyCreate::Execute(CSession *pSession)
{
    assert(pSession);
    LONG lResult;
    REGSAM samAdditional = 0;
    ATL::CAtlStringW sPartialName;
    int iStart = 0;

    assert(0); // TODO: Preizkusi ta del kode.

#ifndef _WIN64
    if (IsWow64Process()) {
        // 32-bit processes run as WOW64 should use 64-bit registry too.
        samAdditional |= KEY_WOW64_64KEY;
    }
#endif

    for (;;) {
        HKEY hKey;

        int iStartNext = m_sValue.Find(L'\\', iStart);
        if (iStartNext >= 0)
            sPartialName.SetString(m_sValue, iStartNext);
        else
            sPartialName = m_sValue;

        // Try to open the key, to see if it exists.
        lResult = ::RegOpenKeyExW(m_hKeyRoot, sPartialName, 0, KEY_ENUMERATE_SUB_KEYS | samAdditional, &hKey);
        if (lResult == ERROR_FILE_NOT_FOUND) {
            // The key doesn't exist yet. Create it.

            if (pSession->m_bRollbackEnabled) {
                // Order rollback action to delete the key. ::RegCreateEx() might create a key but return failure.
                pSession->m_olRollback.AddHead(new COpRegKeyDelete(m_hKeyRoot, sPartialName));
            }

            // Create the key.
            lResult = ::RegCreateKeyExW(m_hKeyRoot, sPartialName, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ENUMERATE_SUB_KEYS | samAdditional, NULL, &hKey, NULL);
            if (lResult != ERROR_SUCCESS) break;
            verify(::RegCloseKey(hKey) == ERROR_SUCCESS);
        } else if (lResult == ERROR_SUCCESS) {
            // This key already exists. Release its handle and continue.
            verify(::RegCloseKey(hKey) == ERROR_SUCCESS);
        } else
            break;

        if (iStartNext < 0) break;
        iStart = iStartNext + 1;
    }

    if (lResult == ERROR_SUCCESS)
        return S_OK;
    else {
        PMSIHANDLE hRecordProg = ::MsiCreateRecord(4);
        verify(::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_REGKEY_CREATE_FAILED) == ERROR_SUCCESS);
        verify(::MsiRecordSetInteger(hRecordProg, 2, (UINT)m_hKeyRoot & 0x7fffffff     ) == ERROR_SUCCESS);
        verify(::MsiRecordSetStringW(hRecordProg, 3, m_sValue                          ) == ERROR_SUCCESS);
        verify(::MsiRecordSetInteger(hRecordProg, 4, lResult                           ) == ERROR_SUCCESS);
        ::MsiProcessMessage(pSession->m_hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
        return AtlHresultFromWin32(lResult);
    }
}


////////////////////////////////////////////////////////////////////////////
// COpRegKeyCopy
////////////////////////////////////////////////////////////////////////////

COpRegKeyCopy::COpRegKeyCopy(HKEY hKeyRoot, LPCWSTR pszKeyNameSrc, LPCWSTR pszKeyNameDst, int iTicks) : COpRegKeySrcDst(hKeyRoot, pszKeyNameSrc, pszKeyNameDst, iTicks)
{
}


HRESULT COpRegKeyCopy::Execute(CSession *pSession)
{
    assert(pSession);
    LONG lResult;
    REGSAM samAdditional = 0;

    assert(0); // TODO: Preizkusi ta del kode.

    {
        // Delete existing destination key first.
        // Since deleting registry key is a complicated job (when rollback/commit support is required), and we do have an operation just for that, we use it.
        // Don't worry, COpRegKeyDelete::Execute() returns S_OK if key doesn't exist.
        COpRegKeyDelete opDelete(m_hKeyRoot, m_sValue2);
        HRESULT hr = opDelete.Execute(pSession);
        if (FAILED(hr)) return hr;
    }

#ifndef _WIN64
    if (IsWow64Process()) {
        // 32-bit processes run as WOW64 should use 64-bit registry too.
        samAdditional |= KEY_WOW64_64KEY;
    }
#endif

    if (pSession->m_bRollbackEnabled) {
        // Order rollback action to delete the destination key.
        pSession->m_olRollback.AddHead(new COpRegKeyDelete(m_hKeyRoot, m_sValue2));
    }

    // Copy the registry key.
    lResult = CopyKeyRecursively(m_hKeyRoot, m_sValue1, m_sValue2, samAdditional);
    if (lResult == ERROR_SUCCESS)
        return S_OK;
    else {
        PMSIHANDLE hRecordProg = ::MsiCreateRecord(5);
        verify(::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_REGKEY_COPY_FAILED) == ERROR_SUCCESS);
        verify(::MsiRecordSetInteger(hRecordProg, 2, (UINT)m_hKeyRoot & 0x7fffffff   ) == ERROR_SUCCESS);
        verify(::MsiRecordSetStringW(hRecordProg, 3, m_sValue1                       ) == ERROR_SUCCESS);
        verify(::MsiRecordSetStringW(hRecordProg, 4, m_sValue2                       ) == ERROR_SUCCESS);
        verify(::MsiRecordSetInteger(hRecordProg, 5, lResult                         ) == ERROR_SUCCESS);
        ::MsiProcessMessage(pSession->m_hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
        return AtlHresultFromWin32(lResult);
    }
}


////////////////////////////////////////////////////////////////////////////
// COpRegKeyDelete
////////////////////////////////////////////////////////////////////////////

COpRegKeyDelete::COpRegKeyDelete(HKEY hKeyRoot, LPCWSTR m_hKeyRoot, int iTicks) : COpRegKeySingle(hKeyRoot, m_hKeyRoot, iTicks)
{
}


HRESULT COpRegKeyDelete::Execute(CSession *pSession)
{
    assert(pSession);
    LONG lResult;
    REGSAM samAdditional = 0;

    assert(0); // TODO: Preizkusi ta del kode.

#ifndef _WIN64
    if (IsWow64Process()) {
        // 32-bit processes run as WOW64 should use 64-bit registry too.
        samAdditional |= KEY_WOW64_64KEY;
    }
#endif

    if (pSession->m_bRollbackEnabled) {
        // Make a backup of the key first.
        ATL::CAtlStringW sBackupName;
        UINT uiCount = 0;

        for (;;) {
            HKEY hKey;
            sBackupName.Format(L"%ls (orig %u)", (LPCWSTR)m_sValue, ++uiCount);
            lResult = ::RegOpenKeyExW(m_hKeyRoot, sBackupName, 0, KEY_ENUMERATE_SUB_KEYS | samAdditional, &hKey);
            if (lResult != ERROR_SUCCESS) break;
            verify(::RegCloseKey(hKey));
        }
        if (lResult == ERROR_FILE_NOT_FOUND) {
            // Since copying registry key is a complicated job (when rollback/commit support is required), and we do have an operation just for that, we use it.
            COpRegKeyCopy opCopy(m_hKeyRoot, m_sValue, sBackupName);
            HRESULT hr = opCopy.Execute(pSession);
            if (FAILED(hr)) return hr;

            // Order rollback action to restore the key from backup copy.
            pSession->m_olRollback.AddHead(new COpRegKeyCopy(m_hKeyRoot, sBackupName, m_sValue));

            // Order commit action to delete backup copy.
            pSession->m_olCommit.AddTail(new COpRegKeyDelete(m_hKeyRoot, sBackupName, TRUE));
        } else {
            PMSIHANDLE hRecordProg = ::MsiCreateRecord(4);
            verify(::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_REGKEY_PROBING_FAILED) == ERROR_SUCCESS);
            verify(::MsiRecordSetInteger(hRecordProg, 2, (UINT)m_hKeyRoot & 0x7fffffff      ) == ERROR_SUCCESS);
            verify(::MsiRecordSetStringW(hRecordProg, 3, sBackupName                        ) == ERROR_SUCCESS);
            verify(::MsiRecordSetInteger(hRecordProg, 4, lResult                            ) == ERROR_SUCCESS);
            ::MsiProcessMessage(pSession->m_hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
            return AtlHresultFromWin32(lResult);
        }
    }

    // Delete the registry key.
    lResult = DeleteKeyRecursively(m_hKeyRoot, m_sValue, KEY_ENUMERATE_SUB_KEYS | samAdditional);
    if (lResult == ERROR_SUCCESS)
        return S_OK;
    else {
        PMSIHANDLE hRecordProg = ::MsiCreateRecord(4);
        verify(::MsiRecordSetInteger(hRecordProg, 1, ERROR_INSTALL_REGKEY_DELETE_FAILED) == ERROR_SUCCESS);
        verify(::MsiRecordSetInteger(hRecordProg, 2, (UINT)m_hKeyRoot & 0x7fffffff     ) == ERROR_SUCCESS);
        verify(::MsiRecordSetStringW(hRecordProg, 3, m_sValue                          ) == ERROR_SUCCESS);
        verify(::MsiRecordSetInteger(hRecordProg, 4, lResult                           ) == ERROR_SUCCESS);
        ::MsiProcessMessage(pSession->m_hInstall, INSTALLMESSAGE_ERROR, hRecordProg);
        return AtlHresultFromWin32(lResult);
    }
}


LONG COpRegKeyDelete::DeleteKeyRecursively(HKEY hKeyRoot, LPCWSTR pszKeyName, REGSAM sam)
{
    HKEY hKey;
    LONG lResult;

    // Open the key.
    lResult = ::RegOpenKeyEx(hKeyRoot, pszKeyName, 0, sam, &hKey);
    if (lResult == ERROR_SUCCESS) {
        DWORD dwMaxSubKeyLen;

        // Determine the largest subkey name.
        lResult = ::RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, &dwMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL);
        if (lResult == ERROR_SUCCESS) {
            LPWSTR pszSubKeyName;

            // Prepare buffer to hold the subkey names (including zero terminator).
            dwMaxSubKeyLen++;
            pszSubKeyName = new TCHAR[dwMaxSubKeyLen];
            if (pszSubKeyName) {
                DWORD dwIndex;

                // Iterate over all subkeys and delete them. Skip failed.
                for (dwIndex = 0; lResult != ERROR_NO_MORE_ITEMS ;) {
                    lResult = ::RegEnumKeyEx(hKey, dwIndex, pszSubKeyName, &dwMaxSubKeyLen, NULL, NULL, NULL, NULL);
                    if (lResult == ERROR_SUCCESS)
                        lResult = DeleteKeyRecursively(hKey, pszSubKeyName, sam);
                    else
                        dwIndex++;
                }

                delete [] pszSubKeyName;
            } else
                lResult = ERROR_OUTOFMEMORY;
        }

        verify(::RegCloseKey(hKey) == ERROR_SUCCESS);

        // Finally try to delete the key.
        lResult = ::RegDeleteKeyW(hKeyRoot, pszKeyName);
    } else if (lResult == ERROR_FILE_NOT_FOUND) {
        // The key doesn't exist. Not really an error in this case.
        lResult = ERROR_SUCCESS;
    }

    return lResult;
}


} // namespace AMSICA
