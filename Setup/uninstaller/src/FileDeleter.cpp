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
    // 如果已经在删除，先取消当前操作
    if (m_isDeleting.load())
    {
        CancelDeletion();
        WaitForCompletion();
    }

    // 重置状态
    m_cancelRequested.store(false);
    m_isDeleting.store(true);

    // 启动工作线程
    m_workerThread = std::thread(&FileDeleter::WorkerThread, this, 
                                fileList, progressCallback, completionCallback);
}

int FileDeleter::DeleteFiles(const std::vector<FileItem>& fileList, ProgressCallback progressCallback)
{
    int successCount = 0;
    int total = static_cast<int>(fileList.size());

    for (int i = 0; i < total; ++i)
    {
        // 检查是否请求取消
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

        // 调用进度回调
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

    // 检查是否请求取消
    if (m_cancelRequested.load())
    {
        return false;
    }

    // 检查文件/目录是否存在
    DWORD attributes = GetFileAttributesA(filePath.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return true; // 文件不存在，认为删除成功
    }

    // 移除文件属性（只读、隐藏等）
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
    // 检查是否请求取消
    if (m_cancelRequested.load())
    {
        return false;
    }

    // 首先尝试普通删除
    if (DeleteFileA(filePath.c_str()))
    {
        return true;
    }

    // 如果失败，检查是否被占用
    if (IsFileInUse(filePath))
    {
        //退出所有与软件名称相关的进程
        KillProcessByExactName(SOFT_NAME);
        
        // 短暂等待进程结束，但不阻塞太久
        for (int i = 0; i < 10 && !m_cancelRequested.load(); ++i)
        {
            Sleep(10);
        }
    }

    // 再次尝试删除
    if (DeleteFileA(filePath.c_str()))
    {
        return true;
    }

    // 尝试移动到临时目录后删除（处理某些顽固文件）
    char tempPath[MAX_PATH];
    char tempFile[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath) && GetTempFileNameA(tempPath, "del", 0, tempFile))
    {
        if (MoveFileA(filePath.c_str(), tempFile))
        {
            // 标记为删除（重启后删除）
            MoveFileExA(tempFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
            return true;
        }
    }

    return false;
}

bool FileDeleter::ForceDeleteDirectory(const std::string& dirPath)
{
    // 检查是否请求取消
    if (m_cancelRequested.load())
    {
        return false;
    }

    // 首先尝试普通删除
    if (RemoveDirectoryA(dirPath.c_str()))
    {
        return true;
    }

    // 如果失败，递归删除目录内容
    WIN32_FIND_DATAA findData;
    std::string searchPath = dirPath + "\\*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            // 检查是否请求取消
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

    // 再次尝试删除目录
    if (RemoveDirectoryA(dirPath.c_str()))
    {
        return true;
    }

    // 最后尝试标记为重启后删除
    MoveFileExA(dirPath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    return true; // 认为成功，因为已经标记删除
}

bool FileDeleter::IsFileInUse(const std::string& filePath)
{
    HANDLE hFile = CreateFileA(
        filePath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, // 不共享
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

    // 移除只读、隐藏、系统等属性
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
        // 检查是否请求取消
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

        // 线程安全的进度回调
        SafeProgressCallback(i + 1, total, item.path, success, progressCallback);
    }

    // 线程安全的完成回调
    if (completionCallback)
    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        completionCallback();
    }

    // 标记删除完成
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