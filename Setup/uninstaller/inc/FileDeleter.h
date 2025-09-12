#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include "FileCollector.h"

class FileDeleter
{
public:
    // 进度回调函数类型
    // 参数：当前项索引，总项数，当前删除的文件路径，是否成功
    using ProgressCallback = std::function<void(int current, int total, const std::string& currentFile, bool success)>;
    
    // 完成回调函数类型
    // 参数：删除成功的文件数量，总文件数量
    using CompletionCallback = std::function<void()>;

    FileDeleter();
    ~FileDeleter();

    /**
     * 异步删除文件列表中的所有文件和文件夹
     * @param fileList 要删除的文件列表
     * @param progressCallback 进度回调函数（在UI线程中调用）
     * @param completionCallback 完成回调函数（在UI线程中调用）
     */
    void DeleteFilesAsync(const std::vector<FileItem>& fileList, 
                         ProgressCallback progressCallback = nullptr,
                         CompletionCallback completionCallback = nullptr);

    /**
     * 同步删除文件列表中的所有文件和文件夹（保持向后兼容）
     * @param fileList 要删除的文件列表
     * @param progressCallback 进度回调函数
     * @return 删除成功的文件数量
     */
    int DeleteFiles(const std::vector<FileItem>& fileList, ProgressCallback progressCallback = nullptr);

    /**
     * 取消当前的删除操作
     */
    void CancelDeletion();

    /**
     * 检查是否正在删除文件
     * @return 是否正在删除
     */
    bool IsDeleting() const;

    /**
     * 等待删除操作完成
     */
    void WaitForCompletion();

    /**
     * 删除单个文件或文件夹
     * @param filePath 文件路径
     * @param isDirectory 是否为目录
     * @return 是否删除成功
     */
    bool DeleteSingleItem(const std::string& filePath, bool isDirectory);

    /**
     * 强制删除文件（处理只读、隐藏等属性）
     * @param filePath 文件路径
     * @return 是否删除成功
     */
    bool ForceDeleteFile(const std::string& filePath);

    /**
     * 强制删除目录（递归删除所有内容）
     * @param dirPath 目录路径
     * @return 是否删除成功
     */
    bool ForceDeleteDirectory(const std::string& dirPath);

    /**
     * 检查文件是否正在被使用
     * @param filePath 文件路径
     * @return 是否正在被使用
     */
    bool IsFileInUse(const std::string& filePath);

private:
    /**
     * 移除文件的只读、隐藏等属性
     * @param filePath 文件路径
     * @return 是否成功
     */
    bool RemoveFileAttributes(const std::string& filePath);

    /**
     * 尝试结束占用文件的进程（谨慎使用）
     * @param filePath 文件路径
     * @return 是否成功
     */
    bool TryTerminateFileProcesses(const std::string& filePath);

    /**
     * 工作线程函数
     * @param fileList 要删除的文件列表
     * @param progressCallback 进度回调函数
     * @param completionCallback 完成回调函数
     */
    void WorkerThread(const std::vector<FileItem>& fileList, 
                     ProgressCallback progressCallback,
                     CompletionCallback completionCallback);

    /**
     * 线程安全的进度回调
     * @param current 当前项索引
     * @param total 总项数
     * @param currentFile 当前删除的文件路径
     * @param success 是否成功
     * @param callback 回调函数
     */
    void SafeProgressCallback(int current, int total, const std::string& currentFile, 
                             bool success, ProgressCallback callback);

private:
    std::atomic<bool> m_isDeleting;      // 是否正在删除
    std::atomic<bool> m_cancelRequested; // 是否请求取消
    std::thread m_workerThread;          // 工作线程
    std::mutex m_callbackMutex;          // 回调函数互斥锁
    HWND m_callbackWindow;               // 用于回调的窗口句柄
};