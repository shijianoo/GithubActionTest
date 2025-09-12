#pragma once
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "IprogressObserver.h"

#define MMIO_SUFFICIENT_SIZE_FOR_FIELD(size, field) \
    (m_dwDataSize >= (offsetof(MmioDataStructure, field) + sizeof(MmioDataStructure().field)))

// MMIO data structure for interprocess communication
struct MmioDataStructure
{
    bool m_downloadFinished;               // Is download complete?
    bool m_installFinished;                // Is installation complete?
    bool m_downloadAbort;                  // Set to cause downloader to abort.
    bool m_installAbort;                   // Set to cause installer to abort.
    HRESULT m_hrDownloadFinished;          // Resulting HRESULT for download.
    HRESULT m_hrInstallFinished;           // Resulting HRESULT for installation.
    HRESULT m_hrInternalError;
    WCHAR m_szCurrentItemStep[MAX_PATH];
    unsigned char m_downloadProgressSoFar;         // Download progress 0-255 (0-100% done).
    unsigned char m_installProgressSoFar;          // Installation progress 0-255 (0-100% done).
    WCHAR m_szEventName[MAX_PATH];         // Event that chainer creates and chainee opens to sync communications.

    BYTE m_version;                        // Version of the data structure, set by chainer:
    // 0x0: .NET Framework 4
    // 0x1: .NET Framework 4.5

    DWORD m_messageCode;                   // Current message sent by the chainee; 0 if no message is active.
    DWORD m_messageResponse;               // Chainer's response to current message; 0 if not yet handled.
    DWORD m_messageDataLength;             // Length of the m_messageData field, in bytes.
    BYTE m_messageData[1];                 // Variable-length buffer; content depends on m_messageCode.
};

// MmioChainerBase class manages the communication and synchronization data 
    // structures. It also implements common get accessors (for chainer) and set accessors(for chainee).
class MmioChainerBase
{
    HANDLE m_section;
    HANDLE m_event;
    MmioDataStructure* m_pData;

protected:
    MmioChainerBase(HANDLE section, HANDLE hevent)
        : m_section(section)
        , m_event(hevent)
        , m_pData(MapView(section))
    {
    }

    virtual ~MmioChainerBase()
    {
        if (m_pData)
        {
            ::UnmapViewOfFile(m_pData);
        }
    }

public:
    HANDLE GetEventHandle() const
    {
        return m_event;
    }

    HANDLE GetMmioHandle()  const
    {
        return m_section;
    }

    void Init(LPCWSTR eventName)
    {
        if (NULL == m_pData)
        {
            return;
        }

        wcscpy_s(m_pData->m_szEventName, MAX_PATH, eventName);

        m_pData->m_downloadFinished = false;
        m_pData->m_downloadProgressSoFar = 0;
        m_pData->m_hrDownloadFinished = E_PENDING;
        m_pData->m_downloadAbort = false;

        m_pData->m_installFinished = false;
        m_pData->m_installProgressSoFar = 0;
        m_pData->m_hrInstallFinished = E_PENDING;
        m_pData->m_installAbort = false;

        m_pData->m_hrInternalError = S_OK;
    }

    // This is called by the chainer to force the chained setup to be canceled.
    void Abort()
    {
        if (NULL == m_pData)
        {
            return;
        }

        m_pData->m_downloadAbort = true;
        m_pData->m_installAbort = true;
    }

    // Called when chainer wants to know if chained setup has finished both download and installation.
    bool IsDone() const
    {
        if (NULL == m_pData)
        {
            return true;
        }

        return m_pData->m_downloadFinished && m_pData->m_installFinished;
    }

    // Called by the chainer to get the overall progress, i.e., the combination of the download and installation.
    int GetProgress() const
    {
        if (NULL == m_pData)
        {
            return 255;
        }
        return (m_pData->m_downloadProgressSoFar + m_pData->m_installProgressSoFar);
    }


    // Get download progress.
    unsigned char GetDownloadProgress() const
    {
        if (NULL == m_pData)
        {
            return 255;
        }

        return m_pData->m_downloadProgressSoFar;
    }

    // Get installation progress.
    unsigned char GetInstallProgress() const
    {
        if (NULL == m_pData)
        {
            return 255;
        }

        return m_pData->m_installProgressSoFar;
    }

    // Get installation progress.
    unsigned char GetVersion() const
    {
        if (NULL == m_pData)
        {
            return 255;
        }

        return m_pData->m_version;
    }

    // Get the combined setup result, installation taking priority over download if both failed.
    HRESULT GetResult() const
    {
        if (NULL == m_pData)
        {
            return S_FALSE;
        }

        if (m_pData->m_hrInstallFinished != S_OK)
        {
            return m_pData->m_hrInstallFinished;
        }
        else
        {
            return m_pData->m_hrDownloadFinished;
        }
    }

    HRESULT GetDownloadResult() const
    {
        if (NULL == m_pData)
        {
            return S_FALSE;
        }

        return m_pData->m_hrDownloadFinished;
    }

    HRESULT GetInstallResult() const
    {
        if (NULL == m_pData)
        {
            return S_FALSE;
        }

        return m_pData->m_hrInstallFinished;
    }

    const HRESULT GetInternalResult() const
    {
        if (NULL == m_pData)
        {
            return S_FALSE;
        }

        return m_pData->m_hrInternalError;
    }

    LPCWSTR GetCurrentItemStep() const
    {
        if (NULL == m_pData)
        {
            return L"";
        }

        return m_pData->m_szCurrentItemStep;
    }

protected:
    static MmioDataStructure* MapView(HANDLE section)
    {
        if (NULL == section)
        {
            return reinterpret_cast<MmioDataStructure*>(NULL);
        }

        return reinterpret_cast<MmioDataStructure*>(::MapViewOfFile(section,
            FILE_MAP_WRITE,
            0, 0, // offsets
            sizeof(MmioDataStructure)));
    }
};

// This is the class that the consumer (chainer) should derive from.
class MmioChainer : protected MmioChainerBase
{
public:
    // Constructor
    MmioChainer(LPCWSTR sectionName, LPCWSTR eventName): MmioChainerBase(CreateSection(sectionName), CreateEvent(eventName))
    {
        Init(eventName);
    }

    // Destructor
    virtual ~MmioChainer()
    {
        ::CloseHandle(GetEventHandle());
        ::CloseHandle(GetMmioHandle());
    }

public: // The public methods:  Abort and Run
    using MmioChainerBase::Abort;
    using MmioChainerBase::GetInstallResult;
    using MmioChainerBase::GetInstallProgress;
    using MmioChainerBase::GetDownloadResult;
    using MmioChainerBase::GetDownloadProgress;
    using MmioChainerBase::GetCurrentItemStep;

    HRESULT GetInternalErrorCode()
    {
        return GetInternalResult();
    }

    void Run(HANDLE process, IProgressObserver& observer)
    {
        HANDLE handles[2] = { process, GetEventHandle() };
        while (!IsDone())
        {
            DWORD ret = ::WaitForMultipleObjects(2, handles, FALSE, 1000);
            switch (ret)
            {
                case WAIT_OBJECT_0:
                {
                    if (IsDone() == false)
                    {
                        HRESULT hr = GetResult();
                        return;
                    }
                    break;
                }
                case WAIT_OBJECT_0 + 1:
                    observer.OnInstallProgress((GetProgress() * 100) / 510);
                    break;
                default:
                    break;
            }
        }
    }

private:
    static HANDLE CreateSection(LPCWSTR sectionName)
    {
        return ::CreateFileMappingW(INVALID_HANDLE_VALUE,
            NULL, // Security attributes
            PAGE_READWRITE,
            0, // high-order DWORD of maximum size
            sizeof(MmioDataStructure), // Low-order DWORD of maximum size.
            sectionName);
    }
    static HANDLE CreateEvent(LPCWSTR eventName)
    {
        return ::CreateEventW(NULL, FALSE, FALSE, eventName);
    }
};
