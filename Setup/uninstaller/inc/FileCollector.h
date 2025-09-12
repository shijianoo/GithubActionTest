#pragma once
#include <windows.h>
#include <string>
#include <vector>

struct FileItem
{
    std::string path;           // 文件或文件夹路径
    bool isDirectory;           // 是否为目录
    bool isShortcut;           // 是否为快捷方式
};

class FileCollector
{
public:

    std::vector<std::string> changeNotifies;

    /**
     * 收集所有需要删除的文件和文件夹
     * @param softwareName 软件名称
     * @param installPath 安装路径
     * @return 收集到的文件列表
     */
    std::vector<FileItem> CollectFilesToDelete(const std::string& softwareName, const std::string& installPath);

    /**
     * 添加桌面快捷方式
     * @param softwareName 软件名称
     * @param fileList 文件列表
     */
    void AddDesktopShortcuts(const std::string& softwareName, std::vector<FileItem>& fileList);

    /**
     * 添加开始菜单文件夹及其内容
     * @param softwareName 软件名称
     * @param fileList 文件列表
     */
    void AddStartMenuItems(const std::string& softwareName, std::vector<FileItem>& fileList);

    /**
     * 添加程序数据文件夹
     * @param softwareName 软件名称
     * @param fileList 文件列表
     */
    void AddProgramDataFiles(const std::string& softwareName, std::vector<FileItem>& fileList);

    /**
     * 检查文件或目录是否存在
     * @param path 路径
     * @return 是否存在
     */
    bool PathExists(const std::string& path);

    /**
     * 检查路径是否为目录
     * @param path 路径
     * @return 是否为目录
     */
    bool IsDirectory(const std::string& path);

private:
    /**
     * 递归收集目录下的所有文件
     * @param dirPath 目录路径
     * @param fileList 文件列表
     * @param description 描述前缀
     */
    void CollectDirectoryFiles(const std::string& dirPath, std::vector<FileItem>& fileList);

    /**
     * 获取特殊文件夹路径
     * @param csidl 文件夹ID
     * @return 文件夹路径
     */
    std::string GetSpecialFolderPath(int csidl);
};