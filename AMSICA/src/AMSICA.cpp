#include "StdAfx.h"


namespace AMSICA {

////////////////////////////////////////////////////////////////////////////
// COperation
////////////////////////////////////////////////////////////////////////////

COperation::COperation(int iTicks) :
    m_iTicks(iTicks)
{
}


////////////////////////////////////////////////////////////////////////////
// COpTypeSingleString
////////////////////////////////////////////////////////////////////////////

COpTypeSingleString::COpTypeSingleString(LPCWSTR pszValue, int iTicks) :
    m_sValue(pszValue),
    COperation(iTicks)
{
}


////////////////////////////////////////////////////////////////////////////
// COpDoubleStringOperation
////////////////////////////////////////////////////////////////////////////

COpTypeSrcDstString::COpTypeSrcDstString(LPCWSTR pszValue1, LPCWSTR pszValue2, int iTicks) :
    m_sValue1(pszValue1),
    m_sValue2(pszValue2),
    COperation(iTicks)
{
}


////////////////////////////////////////////////////////////////////////////
// COpTypeBoolean
////////////////////////////////////////////////////////////////////////////

COpTypeBoolean::COpTypeBoolean(BOOL bValue, int iTicks) :
    m_bValue(bValue),
    COperation(iTicks)
{
}


////////////////////////////////////////////////////////////////////////////
// COpRollbackEnable
////////////////////////////////////////////////////////////////////////////

COpRollbackEnable::COpRollbackEnable(BOOL bEnable, int iTicks) :
    COpTypeBoolean(bEnable, iTicks)
{
}


HRESULT COpRollbackEnable::Execute(CSession *pSession)
{
    assert(pSession);

    pSession->m_bRollbackEnabled = m_bValue;
    return S_OK;
}


////////////////////////////////////////////////////////////////////////////
// COpList
////////////////////////////////////////////////////////////////////////////

COpList::COpList(int iTicks) :
    COperation(iTicks),
    ATL::CAtlList<COperation*>(sizeof(COperation*))
{
}


void COpList::Free()
{
    POSITION pos;

    for (pos = GetHeadPosition(); pos;) {
        COperation *pOp = GetNext(pos);
        COpList *pOpList = dynamic_cast<COpList*>(pOp);

        if (pOpList) {
            // Recursivelly free sublists.
            pOpList->Free();
        }
        delete pOp;
    }

    RemoveAll();
}


HRESULT COpList::LoadFromFile(LPCTSTR pszFileName)
{
    assert(pszFileName);
    HRESULT hr;
    ATL::CAtlFile fSequence;

    hr = fSequence.Create(pszFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN);
    if (FAILED(hr)) return hr;

    // Load operation sequence.
    return fSequence >> *this;
}


HRESULT COpList::SaveToFile(LPCTSTR pszFileName) const
{
    assert(pszFileName);
    HRESULT hr;
    ATL::CAtlFile fSequence;

    hr = fSequence.Create(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN);
    if (FAILED(hr)) return hr;

    // Save execute sequence to file.
    hr = fSequence << *this;
    fSequence.Close();

    if (FAILED(hr)) ::DeleteFile(pszFileName);
    return hr;
}


HRESULT COpList::Execute(CSession *pSession)
{
    assert(pSession);
    POSITION pos;
    HRESULT hr;
    PMSIHANDLE hRecordProg = ::MsiCreateRecord(3);

    // Tell the installer to use explicit progress messages.
    verify(::MsiRecordSetInteger(hRecordProg, 1, 1) == ERROR_SUCCESS);
    verify(::MsiRecordSetInteger(hRecordProg, 2, 1) == ERROR_SUCCESS);
    verify(::MsiRecordSetInteger(hRecordProg, 3, 0) == ERROR_SUCCESS);
    ::MsiProcessMessage(pSession->m_hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg);

    // Prepare hRecordProg for progress messages.
    verify(::MsiRecordSetInteger(hRecordProg, 1, 2) == ERROR_SUCCESS);
    verify(::MsiRecordSetInteger(hRecordProg, 3, 0) == ERROR_SUCCESS);

    for (pos = GetHeadPosition(); pos;) {
        COperation *pOp = GetNext(pos);
        assert(pOp);

        hr = pOp->Execute(pSession);
        if (!pSession->m_bContinueOnError && FAILED(hr)) {
            // Operation failed. Its Execute() method should have sent error message to Installer.
            // Therefore, just quit here.
            return hr;
        }

        verify(::MsiRecordSetInteger(hRecordProg, 2, pOp->m_iTicks) == ERROR_SUCCESS);
        if (::MsiProcessMessage(pSession->m_hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg) == IDCANCEL)
            return AtlHresultFromWin32(ERROR_INSTALL_USEREXIT);
    }

    verify(::MsiRecordSetInteger(hRecordProg, 2, m_iTicks) == ERROR_SUCCESS);
    ::MsiProcessMessage(pSession->m_hInstall, INSTALLMESSAGE_PROGRESS, hRecordProg);

    return S_OK;
}


////////////////////////////////////////////////////////////////////////////
// CSession
////////////////////////////////////////////////////////////////////////////

CSession::CSession() :
    m_bContinueOnError(FALSE),
    m_bRollbackEnabled(FALSE)
{
}


} // namespace AMSICA