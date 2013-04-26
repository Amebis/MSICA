#ifndef __AMSICA_H__
#define __AMSICA_H__

////////////////////////////////////////////////////////////////////////////
// Version
////////////////////////////////////////////////////////////////////////////

#define AMSICA_VERSION       0x01000000

#define AMSICA_VERSION_MAJ   1
#define AMSICA_VERSION_MIN   0
#define AMSICA_VERSION_REV   0

#define AMSICA_VERSION_STR   "1.0"


#if !defined(RC_INVOKED) && !defined(MIDL_PASS)

#include <atlbase.h>
#include <atlcoll.h>
#include <atlfile.h>
#include <atlstr.h>
#include <msi.h>
#include <mstask.h>
#include <windows.h>


////////////////////////////////////////////////////////////////////
// Error codes (last unused 2561L)
////////////////////////////////////////////////////////////////////

#define ERROR_INSTALL_DELETE_FAILED                    2554L
#define ERROR_INSTALL_MOVE_FAILED                      2555L
#define ERROR_INSTALL_TASK_CREATE_FAILED               2556L
#define ERROR_INSTALL_TASK_DELETE_FAILED               2557L
#define ERROR_INSTALL_TASK_ENABLE_FAILED               2558L
#define ERROR_INSTALL_TASK_COPY_FAILED                 2559L

// Errors reported by MSITSCA
#define ERROR_INSTALL_SCHEDULED_TASKS_DATABASE_OPEN    2550L
#define ERROR_INSTALL_SCHEDULED_TASKS_OPLIST_CREATE    2551L
#define ERROR_INSTALL_SCHEDULED_TASKS_SCRIPT_WRITE     2552L
#define ERROR_INSTALL_SCHEDULED_TASKS_SCRIPT_READ      2560L
#define ERROR_INSTALL_SCHEDULED_TASKS_PROPERTY_SET     2553L


namespace AMSICA {


////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////

class CSession;


////////////////////////////////////////////////////////////////////////////
// COperation
////////////////////////////////////////////////////////////////////////////

class COperation
{
public:
    COperation(int iTicks = 0);

    virtual HRESULT Execute(CSession *pSession) = 0;

    friend class COpList;
    friend inline HRESULT operator <<(ATL::CAtlFile &f, const COperation &op);
    friend inline HRESULT operator >>(ATL::CAtlFile &f, COperation &op);

protected:
    int m_iTicks;   // Number of ticks on a progress bar required for this action execution
};


////////////////////////////////////////////////////////////////////////////
// COpTypeSingleString
////////////////////////////////////////////////////////////////////////////

class COpTypeSingleString : public COperation
{
public:
    COpTypeSingleString(LPCWSTR pszValue = L"", int iTicks = 0);

    friend inline HRESULT operator <<(ATL::CAtlFile &f, const AMSICA::COpTypeSingleString &op);
    friend inline HRESULT operator >>(ATL::CAtlFile &f, AMSICA::COpTypeSingleString &op);

protected:
    ATL::CAtlStringW m_sValue;
};


////////////////////////////////////////////////////////////////////////////
// COpDoubleStringOperation
////////////////////////////////////////////////////////////////////////////

class COpTypeSrcDstString : public COperation
{
public:
    COpTypeSrcDstString(LPCWSTR pszValue1 = L"", LPCWSTR pszValue2 = L"", int iTicks = 0);

    friend inline HRESULT operator <<(ATL::CAtlFile &f, const AMSICA::COpTypeSrcDstString &op);
    friend inline HRESULT operator >>(ATL::CAtlFile &f, AMSICA::COpTypeSrcDstString &op);

protected:
    ATL::CAtlStringW m_sValue1;
    ATL::CAtlStringW m_sValue2;
};


////////////////////////////////////////////////////////////////////////////
// COpTypeBoolean
////////////////////////////////////////////////////////////////////////////

class COpTypeBoolean : public COperation
{
public:
    COpTypeBoolean(BOOL bValue = TRUE, int iTicks = 0);

    friend inline HRESULT operator <<(ATL::CAtlFile &f, const AMSICA::COpTypeBoolean &op);
    friend inline HRESULT operator >>(ATL::CAtlFile &f, AMSICA::COpTypeBoolean &op);

protected:
    BOOL m_bValue;
};


////////////////////////////////////////////////////////////////////////////
// COpRollbackEnable
////////////////////////////////////////////////////////////////////////////

class COpRollbackEnable : public COpTypeBoolean
{
public:
    COpRollbackEnable(BOOL bEnable = TRUE, int iTicks = 0);
    virtual HRESULT Execute(CSession *pSession);
};


////////////////////////////////////////////////////////////////////////////
// COpFileDelete
////////////////////////////////////////////////////////////////////////////

class COpFileDelete : public COpTypeSingleString
{
public:
    COpFileDelete(LPCWSTR pszFileName = L"", int iTicks = 0);
    virtual HRESULT Execute(CSession *pSession);
};


////////////////////////////////////////////////////////////////////////////
// COpFileMove
////////////////////////////////////////////////////////////////////////////

class COpFileMove : public COpTypeSrcDstString
{
public:
    COpFileMove(LPCWSTR pszFileSrc = L"", LPCWSTR pszFileDst = L"", int iTicks = 0);
    virtual HRESULT Execute(CSession *pSession);
};


////////////////////////////////////////////////////////////////////////////
// COpTaskCreate
////////////////////////////////////////////////////////////////////////////

class COpTaskCreate : public COpTypeSingleString
{
public:
    COpTaskCreate(LPCWSTR pszTaskName = L"", int iTicks = 0);
    virtual ~COpTaskCreate();
    virtual HRESULT Execute(CSession *pSession);

    UINT SetFromRecord(MSIHANDLE hInstall, MSIHANDLE hRecord);
    UINT SetTriggersFromView(MSIHANDLE hView);

    friend inline HRESULT operator <<(ATL::CAtlFile &f, const AMSICA::COpTaskCreate &op);
    friend inline HRESULT operator >>(ATL::CAtlFile &f, AMSICA::COpTaskCreate &op);

protected:
    ATL::CAtlStringW m_sApplicationName;
    ATL::CAtlStringW m_sParameters;
    ATL::CAtlStringW m_sWorkingDirectory;
    ATL::CAtlStringW m_sAuthor;
    ATL::CAtlStringW m_sComment;
    DWORD            m_dwFlags;
    DWORD            m_dwPriority;
    ATL::CAtlStringW m_sAccountName;
    ATL::CAtlStringW m_sPassword;
    WORD             m_wIdleMinutes;
    WORD             m_wDeadlineMinutes;
    DWORD            m_dwMaxRuntimeMS;

    ATL::CAtlList<TASK_TRIGGER> m_lTriggers;
};


////////////////////////////////////////////////////////////////////////////
// COpTaskDelete
////////////////////////////////////////////////////////////////////////////

class COpTaskDelete : public COpTypeSingleString
{
public:
    COpTaskDelete(LPCWSTR pszTaskName = L"", int iTicks = 0);
    virtual HRESULT Execute(CSession *pSession);
};


////////////////////////////////////////////////////////////////////////////
// COpTaskEnable
////////////////////////////////////////////////////////////////////////////

class COpTaskEnable : public COpTypeSingleString
{
public:
    COpTaskEnable(LPCWSTR pszTaskName = L"", BOOL bEnable = TRUE, int iTicks = 0);
    virtual HRESULT Execute(CSession *pSession);

    friend inline HRESULT operator <<(ATL::CAtlFile &f, const COpTaskEnable &op);
    friend inline HRESULT operator >>(ATL::CAtlFile &f, COpTaskEnable &op);

protected:
    BOOL m_bEnable;
};


////////////////////////////////////////////////////////////////////////////
// COpTaskCopy
////////////////////////////////////////////////////////////////////////////

class COpTaskCopy : public COpTypeSrcDstString
{
public:
    COpTaskCopy(LPCWSTR pszTaskSrc = L"", LPCWSTR pszTaskDst = L"", int iTicks = 0);
    virtual HRESULT Execute(CSession *pSession);
};


////////////////////////////////////////////////////////////////////////////
// COpList
////////////////////////////////////////////////////////////////////////////

class COpList : public COperation, public ATL::CAtlList<COperation*>
{
public:
    COpList(int iTicks = 0);

    void Free();
    HRESULT LoadFromFile(LPCTSTR pszFileName);
    HRESULT SaveToFile(LPCTSTR pszFileName) const;

    virtual HRESULT Execute(CSession *pSession);

    friend inline HRESULT operator <<(ATL::CAtlFile &f, const COpList &op);
    friend inline HRESULT operator >>(ATL::CAtlFile &f, COpList &op);

protected:
    enum OPERATION {
        OPERATION_ENABLE_ROLLBACK = 1,
        OPERATION_DELETE_FILE,
        OPERATION_MOVE_FILE,
        OPERATION_CREATE_TASK,
        OPERATION_DELETE_TASK,
        OPERATION_ENABLE_TASK,
        OPERATION_COPY_TASK,
        OPERATION_SUBLIST
    };

protected:
    template <class T, int ID> inline static HRESULT Save(ATL::CAtlFile &f, const COperation *p);
    template <class T> inline HRESULT LoadAndAddTail(ATL::CAtlFile &f);
};


////////////////////////////////////////////////////////////////////////////
// CSession
////////////////////////////////////////////////////////////////////////////

class CSession
{
public:
    CSession();

    MSIHANDLE m_hInstall;        // Installer handle
    BOOL m_bContinueOnError;     // Continue execution on operation error?
    BOOL m_bRollbackEnabled;     // Is rollback enabled?
    COpList m_olRollback;        // Rollback operation list
    COpList m_olCommit;          // Commit operation list
};

} // namespace AMSICA


////////////////////////////////////////////////////////////////////
// Local includes
////////////////////////////////////////////////////////////////////

#include <atlfile.h>
#include <atlstr.h>
#include <assert.h>
#include <msiquery.h>
#include <mstask.h>


////////////////////////////////////////////////////////////////////
// Inline Functions
////////////////////////////////////////////////////////////////////

inline UINT MsiGetPropertyA(MSIHANDLE hInstall, LPCSTR szName, ATL::CAtlStringA &sValue)
{
    DWORD dwSize = 0;
    UINT uiResult;

    // Query the actual string length first.
    uiResult = ::MsiGetPropertyA(hInstall, szName, "", &dwSize);
    if (uiResult == ERROR_MORE_DATA) {
        // Prepare the buffer to read the string data into and read it.
        LPSTR szBuffer = sValue.GetBuffer(dwSize++);
        if (!szBuffer) return ERROR_OUTOFMEMORY;
        uiResult = ::MsiGetPropertyA(hInstall, szName, szBuffer, &dwSize);
        sValue.ReleaseBuffer(uiResult == ERROR_SUCCESS ? dwSize : 0);
        return uiResult;
    } else if (uiResult == ERROR_SUCCESS) {
        // The string in database is empty.
        sValue.Empty();
        return ERROR_SUCCESS;
    } else {
        // Return error code.
        return uiResult;
    }
}


inline UINT MsiGetPropertyW(MSIHANDLE hInstall, LPCWSTR szName, ATL::CAtlStringW &sValue)
{
    DWORD dwSize = 0;
    UINT uiResult;

    // Query the actual string length first.
    uiResult = ::MsiGetPropertyW(hInstall, szName, L"", &dwSize);
    if (uiResult == ERROR_MORE_DATA) {
        // Prepare the buffer to read the string data into and read it.
        LPWSTR szBuffer = sValue.GetBuffer(dwSize++);
        if (!szBuffer) return ERROR_OUTOFMEMORY;
        uiResult = ::MsiGetPropertyW(hInstall, szName, szBuffer, &dwSize);
        sValue.ReleaseBuffer(uiResult == ERROR_SUCCESS ? dwSize : 0);
        return uiResult;
    } else if (uiResult == ERROR_SUCCESS) {
        // The string in database is empty.
        sValue.Empty();
        return ERROR_SUCCESS;
    } else {
        // Return error code.
        return uiResult;
    }
}


inline UINT MsiRecordGetStringA(MSIHANDLE hRecord, unsigned int iField, ATL::CAtlStringA &sValue)
{
    DWORD dwSize = 0;
    UINT uiResult;

    // Query the actual string length first.
    uiResult = ::MsiRecordGetStringA(hRecord, iField, "", &dwSize);
    if (uiResult == ERROR_MORE_DATA) {
        // Prepare the buffer to read the string data into and read it.
        LPSTR szBuffer = sValue.GetBuffer(dwSize++);
        if (!szBuffer) return ERROR_OUTOFMEMORY;
        uiResult = ::MsiRecordGetStringA(hRecord, iField, szBuffer, &dwSize);
        sValue.ReleaseBuffer(uiResult == ERROR_SUCCESS ? dwSize : 0);
        return uiResult;
    } else if (uiResult == ERROR_SUCCESS) {
        // The string in database is empty.
        sValue.Empty();
        return ERROR_SUCCESS;
    } else {
        // Return error code.
        return uiResult;
    }
}


inline UINT MsiRecordGetStringW(MSIHANDLE hRecord, unsigned int iField, ATL::CAtlStringW &sValue)
{
    DWORD dwSize = 0;
    UINT uiResult;

    // Query the actual string length first.
    uiResult = ::MsiRecordGetStringW(hRecord, iField, L"", &dwSize);
    if (uiResult == ERROR_MORE_DATA) {
        // Prepare the buffer to read the string data into and read it.
        LPWSTR szBuffer = sValue.GetBuffer(dwSize++);
        if (!szBuffer) return ERROR_OUTOFMEMORY;
        uiResult = ::MsiRecordGetStringW(hRecord, iField, szBuffer, &dwSize);
        sValue.ReleaseBuffer(uiResult == ERROR_SUCCESS ? dwSize : 0);
        return uiResult;
    } else if (uiResult == ERROR_SUCCESS) {
        // The string in database is empty.
        sValue.Empty();
        return ERROR_SUCCESS;
    } else {
        // Return error code.
        return uiResult;
    }
}


inline UINT MsiFormatRecordA(MSIHANDLE hInstall, MSIHANDLE hRecord, ATL::CAtlStringA &sValue)
{
    DWORD dwSize = 0;
    UINT uiResult;

    // Query the final string length first.
    uiResult = ::MsiFormatRecordA(hInstall, hRecord, "", &dwSize);
    if (uiResult == ERROR_MORE_DATA) {
        // Prepare the buffer to format the string data into and read it.
        LPSTR szBuffer = sValue.GetBuffer(dwSize++);
        if (!szBuffer) return ERROR_OUTOFMEMORY;
        uiResult = ::MsiFormatRecordA(hInstall, hRecord, szBuffer, &dwSize);
        sValue.ReleaseBuffer(uiResult == ERROR_SUCCESS ? dwSize : 0);
        return uiResult;
    } else if (uiResult == ERROR_SUCCESS) {
        // The result is empty.
        sValue.Empty();
        return ERROR_SUCCESS;
    } else {
        // Return error code.
        return uiResult;
    }
}


inline UINT MsiFormatRecordW(MSIHANDLE hInstall, MSIHANDLE hRecord, ATL::CAtlStringW &sValue)
{
    DWORD dwSize = 0;
    UINT uiResult;

    // Query the final string length first.
    uiResult = ::MsiFormatRecordW(hInstall, hRecord, L"", &dwSize);
    if (uiResult == ERROR_MORE_DATA) {
        // Prepare the buffer to format the string data into and read it.
        LPWSTR szBuffer = sValue.GetBuffer(dwSize++);
        if (!szBuffer) return ERROR_OUTOFMEMORY;
        uiResult = ::MsiFormatRecordW(hInstall, hRecord, szBuffer, &dwSize);
        sValue.ReleaseBuffer(uiResult == ERROR_SUCCESS ? dwSize : 0);
        return uiResult;
    } else if (uiResult == ERROR_SUCCESS) {
        // The result is empty.
        sValue.Empty();
        return ERROR_SUCCESS;
    } else {
        // Return error code.
        return uiResult;
    }
}


inline UINT MsiRecordFormatStringA(MSIHANDLE hInstall, MSIHANDLE hRecord, unsigned int iField, ATL::CAtlStringA &sValue)
{
    UINT uiResult;
    PMSIHANDLE hRecordEx;

    // Read string to format.
    uiResult = ::MsiRecordGetStringA(hRecord, iField, sValue);
    if (uiResult != ERROR_SUCCESS) return uiResult;

    // If the string is empty, there's nothing left to do.
    if (sValue.IsEmpty()) return ERROR_SUCCESS;

    // Create a record.
    hRecordEx = ::MsiCreateRecord(1);
    if (!hRecordEx) return ERROR_INVALID_HANDLE;

    // Populate record with data.
    uiResult = ::MsiRecordSetStringA(hRecordEx, 0, sValue);
    if (uiResult != ERROR_SUCCESS) return uiResult;

    // Do the formatting.
    return ::MsiFormatRecordA(hInstall, hRecordEx, sValue);
}


inline UINT MsiRecordFormatStringW(MSIHANDLE hInstall, MSIHANDLE hRecord, unsigned int iField, ATL::CAtlStringW &sValue)
{
    UINT uiResult;
    PMSIHANDLE hRecordEx;

    // Read string to format.
    uiResult = ::MsiRecordGetStringW(hRecord, iField, sValue);
    if (uiResult != ERROR_SUCCESS) return uiResult;

    // If the string is empty, there's nothing left to do.
    if (sValue.IsEmpty()) return ERROR_SUCCESS;

    // Create a record.
    hRecordEx = ::MsiCreateRecord(1);
    if (!hRecordEx) return ERROR_INVALID_HANDLE;

    // Populate record with data.
    uiResult = ::MsiRecordSetStringW(hRecordEx, 0, sValue);
    if (uiResult != ERROR_SUCCESS) return uiResult;

    // Do the formatting.
    return ::MsiFormatRecordW(hInstall, hRecordEx, sValue);
}

#ifdef UNICODE
#define MsiRecordFormatString  MsiRecordFormatStringW
#else
#define MsiRecordFormatString  MsiRecordFormatStringA
#endif // !UNICODE


inline UINT MsiGetTargetPathA(MSIHANDLE hInstall, LPCSTR szFolder, ATL::CAtlStringA &sValue)
{
    DWORD dwSize = 0;
    UINT uiResult;

    // Query the final string length first.
    uiResult = ::MsiGetTargetPathA(hInstall, szFolder, "", &dwSize);
    if (uiResult == ERROR_MORE_DATA) {
        // Prepare the buffer to format the string data into and read it.
        LPSTR szBuffer = sValue.GetBuffer(dwSize++);
        if (!szBuffer) return ERROR_OUTOFMEMORY;
        uiResult = ::MsiGetTargetPathA(hInstall, szFolder, szBuffer, &dwSize);
        sValue.ReleaseBuffer(uiResult == ERROR_SUCCESS ? dwSize : 0);
        return uiResult;
    } else if (uiResult == ERROR_SUCCESS) {
        // The result is empty.
        sValue.Empty();
        return ERROR_SUCCESS;
    } else {
        // Return error code.
        return uiResult;
    }
}


inline UINT MsiGetTargetPathW(MSIHANDLE hInstall, LPCWSTR szFolder, ATL::CAtlStringW &sValue)
{
    DWORD dwSize = 0;
    UINT uiResult;

    // Query the final string length first.
    uiResult = ::MsiGetTargetPathW(hInstall, szFolder, L"", &dwSize);
    if (uiResult == ERROR_MORE_DATA) {
        // Prepare the buffer to format the string data into and read it.
        LPWSTR szBuffer = sValue.GetBuffer(dwSize++);
        if (!szBuffer) return ERROR_OUTOFMEMORY;
        uiResult = ::MsiGetTargetPathW(hInstall, szFolder, szBuffer, &dwSize);
        sValue.ReleaseBuffer(uiResult == ERROR_SUCCESS ? dwSize : 0);
        return uiResult;
    } else if (uiResult == ERROR_SUCCESS) {
        // The result is empty.
        sValue.Empty();
        return ERROR_SUCCESS;
    } else {
        // Return error code.
        return uiResult;
    }
}


namespace AMSICA {

////////////////////////////////////////////////////////////////////////////
// Inline operators
////////////////////////////////////////////////////////////////////////////

inline HRESULT operator <<(ATL::CAtlFile &f, int i)
{
    return f.Write(&i, sizeof(int));
}


inline HRESULT operator >>(ATL::CAtlFile &f, int &i)
{
    return f.Read(&i, sizeof(int));
}


inline HRESULT operator <<(ATL::CAtlFile &f, const ATL::CAtlStringA &str)
{
    HRESULT hr;
    int iLength = str.GetLength();

    // Write string length (in characters) as 32-bit integer.
    hr =  f.Write(&iLength, sizeof(int));
    if (FAILED(hr)) return hr;

    // Write string data (without terminator).
    return f.Write((LPCSTR)str, sizeof(CHAR) * iLength);
}


inline HRESULT operator >>(ATL::CAtlFile &f, ATL::CAtlStringA &str)
{
    HRESULT hr;
    int iLength;
    LPSTR buf;

    // Read string length (in characters) as 32-bit integer.
    hr =  f.Read(&iLength, sizeof(int));
    if (FAILED(hr)) return hr;

    // Allocate the buffer.
    buf = str.GetBuffer(iLength);
    if (!buf) return E_OUTOFMEMORY;

    // Read string data (without terminator).
    hr = f.Read(buf, sizeof(CHAR) * iLength);
    str.ReleaseBuffer(SUCCEEDED(hr) ? iLength : 0);
    return hr;
}


inline HRESULT operator <<(ATL::CAtlFile &f, const ATL::CAtlStringW &str)
{
    HRESULT hr;
    int iLength = str.GetLength();

    // Write string length (in characters) as 32-bit integer.
    hr = f.Write(&iLength, sizeof(int));
    if (FAILED(hr)) return hr;

    // Write string data (without terminator).
    return f.Write((LPCWSTR)str, sizeof(WCHAR) * iLength);
}


inline HRESULT operator >>(ATL::CAtlFile &f, ATL::CAtlStringW &str)
{
    HRESULT hr;
    int iLength;
    LPWSTR buf;

    // Read string length (in characters) as 32-bit integer.
    hr =  f.Read(&iLength, sizeof(int));
    if (FAILED(hr)) return hr;

    // Allocate the buffer.
    buf = str.GetBuffer(iLength);
    if (!buf) return E_OUTOFMEMORY;

    // Read string data (without terminator).
    hr = f.Read(buf, sizeof(WCHAR) * iLength);
    str.ReleaseBuffer(SUCCEEDED(hr) ? iLength : 0);
    return hr;
}


inline HRESULT operator <<(ATL::CAtlFile &f, const TASK_TRIGGER &ttData)
{
    return f.Write(&ttData, sizeof(TASK_TRIGGER));
}


inline HRESULT operator >>(ATL::CAtlFile &f, TASK_TRIGGER &ttData)
{
    return f.Read(&ttData, sizeof(TASK_TRIGGER));
}


inline HRESULT operator <<(ATL::CAtlFile &f, const AMSICA::COperation &op)
{
    return f << op.m_iTicks;
}


inline HRESULT operator >>(ATL::CAtlFile &f, COperation &op)
{
    return f >> op.m_iTicks;
}


inline HRESULT operator <<(ATL::CAtlFile &f, const COpTypeSingleString &op)
{
    HRESULT hr;

    hr = f << (const COperation &)op;
    if (FAILED(hr)) return hr;

    return f << op.m_sValue;
}


inline HRESULT operator >>(ATL::CAtlFile &f, COpTypeSingleString &op)
{
    HRESULT hr;

    hr = f >> (COperation &)op;
    if (FAILED(hr)) return hr;

    return f >> op.m_sValue;
}


inline HRESULT operator <<(ATL::CAtlFile &f, const COpTypeSrcDstString &op)
{
    HRESULT hr;

    hr = f << (const COperation &)op;
    if (FAILED(hr)) return hr;

    hr = f << op.m_sValue1;
    if (FAILED(hr)) return hr;

    return f << op.m_sValue2;
}


inline HRESULT operator >>(ATL::CAtlFile &f, COpTypeSrcDstString &op)
{
    HRESULT hr;

    hr = f >> (COperation &)op;
    if (FAILED(hr)) return hr;

    hr = f >> op.m_sValue1;
    if (FAILED(hr)) return hr;

    return f >> op.m_sValue2;
}


inline HRESULT operator <<(ATL::CAtlFile &f, const COpTypeBoolean &op)
{
    HRESULT hr;

    hr = f << (const COperation &)op;
    if (FAILED(hr)) return hr;

    return f << (int)op.m_bValue;
}


inline HRESULT operator >>(ATL::CAtlFile &f, COpTypeBoolean &op)
{
    int iValue;
    HRESULT hr;

    hr = f >> (COperation &)op;
    if (FAILED(hr)) return hr;

    hr = f >> iValue;
    if (FAILED(hr)) return hr;
    op.m_bValue = iValue ? TRUE : FALSE;

    return S_OK;
}


inline HRESULT operator <<(ATL::CAtlFile &f, const COpTaskCreate &op)
{
    HRESULT hr;
    POSITION pos;

    hr = f << (const COpTypeSingleString&)op;                  if (FAILED(hr)) return hr;
    hr = f << op.m_sApplicationName;                                   if (FAILED(hr)) return hr;
    hr = f << op.m_sParameters;                                        if (FAILED(hr)) return hr;
    hr = f << op.m_sWorkingDirectory;                                  if (FAILED(hr)) return hr;
    hr = f << op.m_sAuthor;                                            if (FAILED(hr)) return hr;
    hr = f << op.m_sComment;                                           if (FAILED(hr)) return hr;
    hr = f << (int)op.m_dwFlags;                                       if (FAILED(hr)) return hr;
    hr = f << (int)op.m_dwPriority;                                    if (FAILED(hr)) return hr;
    hr = f << op.m_sAccountName;                                       if (FAILED(hr)) return hr;
    hr = f << op.m_sPassword;                                          if (FAILED(hr)) return hr;
    hr = f << (int)MAKELONG(op.m_wDeadlineMinutes, op.m_wIdleMinutes); if (FAILED(hr)) return hr;
    hr = f << (int)op.m_dwMaxRuntimeMS;                                if (FAILED(hr)) return hr;
    hr = f << (int)op.m_lTriggers.GetCount();                          if (FAILED(hr)) return hr;
    for (pos = op.m_lTriggers.GetHeadPosition(); pos;) {
        hr = f << op.m_lTriggers.GetNext(pos);
        if (FAILED(hr)) return hr;
    }

    return S_OK;
}


inline HRESULT operator >>(ATL::CAtlFile &f, COpTaskCreate &op)
{
    HRESULT hr;
    DWORD dwValue;

    hr = f >> (COpTypeSingleString&)op;     if (FAILED(hr)) return hr;
    hr = f >> op.m_sApplicationName;                if (FAILED(hr)) return hr;
    hr = f >> op.m_sParameters;                     if (FAILED(hr)) return hr;
    hr = f >> op.m_sWorkingDirectory;               if (FAILED(hr)) return hr;
    hr = f >> op.m_sAuthor;                         if (FAILED(hr)) return hr;
    hr = f >> op.m_sComment;                        if (FAILED(hr)) return hr;
    hr = f >> (int&)op.m_dwFlags;                   if (FAILED(hr)) return hr;
    hr = f >> (int&)op.m_dwPriority;                if (FAILED(hr)) return hr;
    hr = f >> op.m_sAccountName;                    if (FAILED(hr)) return hr;
    hr = f >> op.m_sPassword;                       if (FAILED(hr)) return hr;
    hr = f >> (int&)dwValue;                        if (FAILED(hr)) return hr; op.m_wIdleMinutes = HIWORD(dwValue); op.m_wDeadlineMinutes = LOWORD(dwValue);
    hr = f >> (int&)op.m_dwMaxRuntimeMS;            if (FAILED(hr)) return hr;
    hr = f >> (int&)dwValue;                        if (FAILED(hr)) return hr;
    while (dwValue--) {
        TASK_TRIGGER ttData;
        hr = f >> ttData;
        if (FAILED(hr)) return hr;
        op.m_lTriggers.AddTail(ttData);
    }

    return S_OK;
}


inline HRESULT operator <<(ATL::CAtlFile &f, const COpTaskEnable &op)
{
    HRESULT hr;

    hr = f << (const COpTypeSingleString&)op;
    if (FAILED(hr)) return hr;

    return f << (int)op.m_bEnable;
}


inline HRESULT operator >>(ATL::CAtlFile &f, COpTaskEnable &op)
{
    HRESULT hr;
    int iTemp;

    hr = f >> (COpTypeSingleString&)op;
    if (FAILED(hr)) return hr;

    hr = f >> iTemp;
    if (FAILED(hr)) return hr;
    op.m_bEnable = iTemp ? TRUE : FALSE;

    return S_OK;
}


inline HRESULT operator <<(ATL::CAtlFile &f, const COpList &list)
{
    POSITION pos;
    HRESULT hr;

    hr = f << (const COperation &)list;
    if (FAILED(hr)) return hr;

    hr = f << (int)list.GetCount();
    if (FAILED(hr)) return hr;

    for (pos = list.GetHeadPosition(); pos;) {
        const COperation *pOp = list.GetNext(pos);
        if (dynamic_cast<const COpRollbackEnable*>(pOp))
            hr = list.Save<COpRollbackEnable, COpList::OPERATION_ENABLE_ROLLBACK>(f, pOp);
        else if (dynamic_cast<const COpFileDelete*>(pOp))
            hr = list.Save<COpFileDelete, COpList::OPERATION_DELETE_FILE>(f, pOp);
        else if (dynamic_cast<const COpFileMove*>(pOp))
            hr = list.Save<COpFileMove, COpList::OPERATION_MOVE_FILE>(f, pOp);
        else if (dynamic_cast<const COpTaskCreate*>(pOp))
            hr = list.Save<COpTaskCreate, COpList::OPERATION_CREATE_TASK>(f, pOp);
        else if (dynamic_cast<const COpTaskDelete*>(pOp))
            hr = list.Save<COpTaskDelete, COpList::OPERATION_DELETE_TASK>(f, pOp);
        else if (dynamic_cast<const COpTaskEnable*>(pOp))
            hr = list.Save<COpTaskEnable, COpList::OPERATION_ENABLE_TASK>(f, pOp);
        else if (dynamic_cast<const COpTaskCopy*>(pOp))
            hr = list.Save<COpTaskCopy, COpList::OPERATION_COPY_TASK>(f, pOp);
        else if (dynamic_cast<const COpList*>(pOp))
            hr = list.Save<COpList, COpList::OPERATION_SUBLIST>(f, pOp);
        else {
            // Unsupported type of operation.
            assert(0);
            hr = E_UNEXPECTED;
        }

        if (FAILED(hr)) return hr;
    }

    return S_OK;
}


inline HRESULT operator >>(ATL::CAtlFile &f, COpList &list)
{
    HRESULT hr;
    DWORD dwCount;

    hr = f >> (COperation &)list;
    if (FAILED(hr)) return hr;

    hr = f >> (int&)dwCount;
    if (FAILED(hr)) return hr;

    while (dwCount--) {
        int iTemp;

        hr = f >> iTemp;
        if (FAILED(hr)) return hr;

        switch ((COpList::OPERATION)iTemp) {
        case COpList::OPERATION_ENABLE_ROLLBACK:
            hr = list.LoadAndAddTail<COpRollbackEnable>(f);
            break;
        case COpList::OPERATION_DELETE_FILE:
            hr = list.LoadAndAddTail<COpFileDelete>(f);
            break;
        case COpList::OPERATION_MOVE_FILE:
            hr = list.LoadAndAddTail<COpFileMove>(f);
            break;
        case COpList::OPERATION_CREATE_TASK:
            hr = list.LoadAndAddTail<COpTaskCreate>(f);
            break;
        case COpList::OPERATION_DELETE_TASK:
            hr = list.LoadAndAddTail<COpTaskDelete>(f);
            break;
        case COpList::OPERATION_ENABLE_TASK:
            hr = list.LoadAndAddTail<COpTaskEnable>(f);
            break;
        case COpList::OPERATION_COPY_TASK:
            hr = list.LoadAndAddTail<COpTaskCopy>(f);
            break;
        case COpList::OPERATION_SUBLIST:
            hr = list.LoadAndAddTail<COpList>(f);
            break;
        default:
            // Unsupported type of operation.
            assert(0);
            hr = E_UNEXPECTED;
        }

        if (FAILED(hr)) return hr;
    }

    return S_OK;
}


////////////////////////////////////////////////////////////////////////////
// Inline methods
////////////////////////////////////////////////////////////////////////////

template <class T, int ID> inline static HRESULT COpList::Save(ATL::CAtlFile &f, const COperation *p)
{
    assert(p);
    HRESULT hr;
    const T *pp = dynamic_cast<const T*>(p);
    if (!pp) return E_UNEXPECTED;

    hr = f << (int)ID;
    if (FAILED(hr)) return hr;

    return f << *pp;
}


template <class T> inline HRESULT COpList::LoadAndAddTail(ATL::CAtlFile &f)
{
    HRESULT hr;

    // Create element.
    T *p = new T();
    if (!p) return E_OUTOFMEMORY;

    // Load element from file.
    hr = f >> *p;
    if (FAILED(hr)) {
        delete p;
        return hr;
    }

    // Add element.
    AddTail(p);
    return S_OK;
}

} // namespace AMSICA


#endif // RC_INVOKED
#endif // __AMSICA_H__
