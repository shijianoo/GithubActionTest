#include <shlwapi.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <shlobj.h>
#include <windows.h>
#include "Helper.h"
#include "FileDeleter.h"

FileDeleter::FileDeleter()
    : m_isDeleting(false)
    , m_cancelRequested(false)
    , m_callbackWindow(nullptr)
{
}

FileDeleter::~FileDeleter()
{
    CancelDeletion();
    WaitForCompletion();
}

void FileDeleter::DeleteFilesAsync(const std::vector<FileItem>& fileList, 
                                  ProgressCallback progressCallback,
                                  CompletionCallback completionCallback)
{
    // ����Ѿ���ɾ������ȡ����ǰ����
    if (m_isDeleting.load())
    {
        CancelDeletion();
        WaitForCompletion();
    }

    // ����״̬
    m_cancelRequested.store(false);
    m_isDeleting.store(true);

    // ���������߳�
    m_workerThread = std::thread(&FileDeleter::WorkerThread, this, 
                                fileList, progressCallback, completionCallback);
}

int FileDeleter::DeleteFiles(const std::vector<FileItem>& fileList, ProgressCallback progressCallback)
{
    int successCount = 0;
    int total = static_cast<int>(fileList.size());

    for (int i = 0; i < total; ++i)
    {
        // ����Ƿ�����ȡ��
        if (m_cancelRequested.load())
        {
            break;
        }

        const FileItem& item = fileList[i];
        bool success = DeleteSingleItem(item.path, item.isDirectory);
        
        if (success)
        {
            successCount++;
        }

        // ���ý��Ȼص�
        if (progressCallback)
        {
            progressCallback(i + 1, total, item.path, success);
        }
    }

    return successCount;
}

void FileDeleter::CancelDeletion()
{
    m_cancelRequested.store(true);
}

bool FileDeleter::IsDeleting() const
{
    return m_isDeleting.load();
}

void FileDeleter::WaitForCompletion()
{
    if (m_workerThread.joinable())
    {
        m_workerThread.join();
    }
}

bool FileDeleter::DeleteSingleItem(const std::string& filePath, bool isDirectory)
{
    if (filePath.empty())
    {
        return false;
    }

    // ����Ƿ�����ȡ��
    if (m_cancelRequested.load())
    {
        return false;
    }

    // ����ļ�/Ŀ¼�Ƿ����
    DWORD attributes = GetFileAttributesA(filePath.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return true; // �ļ������ڣ���Ϊɾ���ɹ�
    }

    // �Ƴ��ļ����ԣ�ֻ�������صȣ�
    RemoveFileAttributes(filePath);

    bool success = false;
    if (isDirectory)
    {
        success = ForceDeleteDirectory(filePath);
    }
    else
    {
        success = ForceDeleteFile(filePath);
    }

    return success;
}

bool FileDeleter::ForceDeleteFile(const std::string& filePath)
{
    // ����Ƿ�����ȡ��
    if (m_cancelRequested.load())
    {
        return false;
    }

    // ���ȳ�����ͨɾ��
    if (DeleteFileA(filePath.c_str()))
    {
        return true;
    }

    // ���ʧ�ܣ�����Ƿ�ռ��
    if (IsFileInUse(filePath))
    {
        //�˳����������������صĽ���
        KillProcessByExactName(SOFT_NAME);
        
        // ���ݵȴ����̽�������������̫��
        for (int i = 0; i < 10 && !m_cancelRequested.load(); ++i)
        {
            Sleep(10);
        }
    }

    // �ٴγ���ɾ��
    if (DeleteFileA(filePath.c_str()))
    {
        return true;
    }

    // �����ƶ�����ʱĿ¼��ɾ��������ĳЩ����ļ���
    char tempPath[MAX_PATH];
    char tempFile[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath) && GetTempFileNameA(tempPath, "del", 0, tempFile))
    {
        if (MoveFileA(filePath.c_str(), tempFile))
        {
            // ���Ϊɾ����������ɾ����
            MoveFileExA(tempFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
            return true;
        }
    }

    return false;
}

bool FileDeleter::ForceDeleteDirectory(const std::string& dirPath)
{
    // ����Ƿ�����ȡ��
    if (m_cancelRequested.load())
    {
        return false;
    }

    // ���ȳ�����ͨɾ��
    if (RemoveDirectoryA(dirPath.c_str()))
    {
        return true;
    }

    // ���ʧ�ܣ��ݹ�ɾ��Ŀ¼����
    WIN32_FIND_DATAA findData;
    std::string searchPath = dirPath + "\\*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            // ����Ƿ�����ȡ��
            if (m_cancelRequested.load())
            {
                FindClose(hFind);
                return false;
            }

            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            {
                continue;
            }

            std::string fullPath = dirPath + "\\" + findData.cFileName;
            bool isDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

            if (isDir)
            {
                ForceDeleteDirectory(fullPath);
            }
            else
            {
                ForceDeleteFile(fullPath);
            }

        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    }

    // �ٴγ���ɾ��Ŀ¼
    if (RemoveDirectoryA(dirPath.c_str()))
    {
        return true;
    }

    // ����Ա��Ϊ������ɾ��
    MoveFileExA(dirPath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    return true; // ��Ϊ�ɹ�����Ϊ�Ѿ����ɾ��
}

bool FileDeleter::IsFileInUse(const std::string& filePath)
{
    HANDLE hFile = CreateFileA(
        filePath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, // ������
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        CloseHandle(hFile);
        return (error == ERROR_SHARING_VIOLATION || error == ERROR_ACCESS_DENIED);
    }

    CloseHandle(hFile);
    return false;
}

bool FileDeleter::RemoveFileAttributes(const std::string& filePath)
{
    DWORD attributes = GetFileAttributesA(filePath.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    // �Ƴ�ֻ�������ء�ϵͳ������
    DWORD newAttributes = attributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    return SetFileAttributesA(filePath.c_str(), newAttributes);
}

void FileDeleter::WorkerThread(const std::vector<FileItem>& fileList, 
                              ProgressCallback progressCallback,
                              CompletionCallback completionCallback)
{
    int successCount = 0;
    int total = static_cast<int>(fileList.size());

    for (int i = 0; i < total; ++i)
    {
        // ����Ƿ�����ȡ��
        if (m_cancelRequested.load())
        {
            break;
        }

        const FileItem& item = fileList[i];
        bool success = DeleteSingleItem(item.path, item.isDirectory);
        
        if (success)
        {
            successCount++;
        }

        // �̰߳�ȫ�Ľ��Ȼص�
        SafeProgressCallback(i + 1, total, item.path, success, progressCallback);
    }

    // �̰߳�ȫ����ɻص�
    if (completionCallback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        completionCallback();
    }

    // ���ɾ�����
    m_isDeleting.store(false);
}

void FileDeleter::SafeProgressCallback(int current, int total, const std::string& currentFile, 
                                      bool success, ProgressCallback callback)
{
    if (callback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        callback(current, total, currentFile, success);
    }
}