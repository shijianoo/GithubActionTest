#include <shlobj.h>
#include <algorithm>
#include "FileCollector.h"

std::vector<FileItem> FileCollector::CollectFilesToDelete(const std::string& softwareName, const std::string& installPath)
{
    std::vector<FileItem> fileList;

    // 1. �ռ���װĿ¼�µ������ļ�
    if (!installPath.empty() && PathExists(installPath))
    {
        CollectDirectoryFiles(installPath, fileList);
    }

    // 2. �ռ������ݷ�ʽ
    AddDesktopShortcuts(softwareName, fileList);

    // 3. �ռ���ʼ�˵���
    AddStartMenuItems(softwareName, fileList);

    // 4. �ռ����������ļ�
    AddProgramDataFiles(softwareName, fileList);

    FileItem root;
    root.path = installPath;
    root.isDirectory = true;
    root.isShortcut = false;
    fileList.push_back(root);
    changeNotifies.push_back(installPath);
    return fileList;
}

void FileCollector::AddDesktopShortcuts(const std::string& softwareName, std::vector<FileItem>& fileList)
{
    // ��������
    std::string publicDesktop = GetSpecialFolderPath(CSIDL_COMMON_DESKTOPDIRECTORY);
    if (!publicDesktop.empty())
    {
        std::string shortcutPath = publicDesktop + "\\" + softwareName + ".lnk";
        if (PathExists(shortcutPath))
        {
            FileItem item;
            item.path = shortcutPath;
            item.isDirectory = false;
            item.isShortcut = true;
            fileList.push_back(item);
            changeNotifies.push_back(publicDesktop);
        }
    }
   

    // ��ǰ�û�����
    std::string userDesktop = GetSpecialFolderPath(CSIDL_DESKTOPDIRECTORY);
    if (!userDesktop.empty())
    {
        std::string shortcutPath = userDesktop + "\\" + softwareName + ".lnk";
        if (PathExists(shortcutPath))
        {
            FileItem item;
            item.path = shortcutPath;
            item.isDirectory = false;
            item.isShortcut = true;
            fileList.push_back(item);
            changeNotifies.push_back(userDesktop);
        }
    }
}

void FileCollector::AddStartMenuItems(const std::string& softwareName, std::vector<FileItem>& fileList)
{
    // ������ʼ�˵�
    std::string publicStartMenu = GetSpecialFolderPath(CSIDL_COMMON_PROGRAMS);
    if (!publicStartMenu.empty())
    {
        std::string menuFolder = publicStartMenu + "\\" + softwareName;
        if (PathExists(menuFolder))
        {
            CollectDirectoryFiles(menuFolder, fileList);
            FileItem item;
            item.path = menuFolder;
            item.isDirectory = true;
            item.isShortcut = false;
            fileList.push_back(item);
            changeNotifies.push_back(menuFolder);
        }

        // ���ֱ�ӵĿ�ݷ�ʽ
        std::string shortcutPath = publicStartMenu + "\\" + softwareName + ".lnk";
        if (PathExists(shortcutPath))
        {
            FileItem item;
            item.path = shortcutPath;
            item.isDirectory = false;
            item.isShortcut = true;
            fileList.push_back(item);
            changeNotifies.push_back(publicStartMenu);
        }
    }
    
    // ��ǰ�û���ʼ�˵�
    std::string userStartMenu = GetSpecialFolderPath(CSIDL_PROGRAMS);
    if (!userStartMenu.empty())
    {
        std::string menuFolder = userStartMenu + "\\" + softwareName;
        if (PathExists(menuFolder))
        {
            CollectDirectoryFiles(menuFolder, fileList);
            FileItem item;
            item.path = menuFolder;
            item.isDirectory = true;
            item.isShortcut = false;
            fileList.push_back(item);
            changeNotifies.push_back(menuFolder);
        }

        // ���ֱ�ӵĿ�ݷ�ʽ
        std::string shortcutPath = userStartMenu + "\\" + softwareName + ".lnk";
        if (PathExists(shortcutPath))
        {
            FileItem item;
            item.path = shortcutPath;
            item.isDirectory = false;
            item.isShortcut = true;
            fileList.push_back(item);
            changeNotifies.push_back(userStartMenu);
        }
    }
}

void FileCollector::AddProgramDataFiles(const std::string& softwareName, std::vector<FileItem>& fileList)
{
    // ProgramDataĿ¼
    std::string programData = GetSpecialFolderPath(CSIDL_COMMON_APPDATA);
    if (!programData.empty())
    {
        std::string dataFolder = programData + "\\" + softwareName;
        if (PathExists(dataFolder))
        {
            CollectDirectoryFiles(dataFolder, fileList);
            FileItem item;
            item.path = dataFolder;
            item.isDirectory = true;
            item.isShortcut = false;
            fileList.push_back(item);
            changeNotifies.push_back(dataFolder);
        }
    }
    
    // �û�AppDataĿ¼
    std::string userAppData = GetSpecialFolderPath(CSIDL_APPDATA);
    if (!userAppData.empty())
    {
        std::string dataFolder = userAppData + "\\" + softwareName;
        if (PathExists(dataFolder))
        {
            CollectDirectoryFiles(dataFolder, fileList);
            FileItem item;
            item.path = dataFolder;
            item.isDirectory = true;
            item.isShortcut = false;
            fileList.push_back(item);
            changeNotifies.push_back(dataFolder);
        }
    }

    // ����AppDataĿ¼
    std::string localAppData = GetSpecialFolderPath(CSIDL_LOCAL_APPDATA);
    if (!localAppData.empty())
    {
        std::string dataFolder = localAppData + "\\" + softwareName;
        if (PathExists(dataFolder))
        {
            CollectDirectoryFiles(dataFolder, fileList);
            FileItem item;
            item.path = dataFolder;
            item.isDirectory = true;
            item.isShortcut = false;
            fileList.push_back(item);
            changeNotifies.push_back(dataFolder);
        }
    }

    // �����ĵ�
    std::string commonDocs = GetSpecialFolderPath(CSIDL_COMMON_DOCUMENTS);
    if (!commonDocs.empty()) 
    {
        std::string dataFolder = commonDocs + "\\" + softwareName + " Files";
        if (PathExists(dataFolder))
        {
            CollectDirectoryFiles(dataFolder, fileList);
            FileItem item;
            item.path = dataFolder;
            item.isDirectory = true;
            item.isShortcut = false;
            fileList.push_back(item);
            changeNotifies.push_back(dataFolder);
        }
    }

    //�û��ĵ�
    std::string userDocs = GetSpecialFolderPath(CSIDL_MYDOCUMENTS);
    if (!userDocs.empty())
    {
        std::string dataFolder = userDocs + "\\" + softwareName + " Files";
        if (PathExists(dataFolder))
        {
            CollectDirectoryFiles(dataFolder, fileList);
            FileItem item;
            item.path = dataFolder;
            item.isDirectory = true;
            item.isShortcut = false;
            fileList.push_back(item);
            changeNotifies.push_back(dataFolder);
        }
    }
}

void FileCollector::CollectDirectoryFiles(const std::string& dirPath, std::vector<FileItem>& fileList)
{
    if (dirPath.empty() || !PathExists(dirPath))
    {
        return;
    }

    WIN32_FIND_DATAA findData;
    std::string searchPath = dirPath + "\\*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }

    std::vector<FileItem> subItems;

    do
    {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
        {
            continue;
        }

        std::string fullPath = dirPath + "\\" + findData.cFileName;
        FileItem item;
        item.path = fullPath;
        item.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        item.isShortcut = false;

        // ����Ƿ�Ϊ��ݷ�ʽ
        if (!item.isDirectory)
        {
            std::string fileName = findData.cFileName;
            std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
            if (fileName.length() > 4 && fileName.substr(fileName.length() - 4) == ".lnk")
            {
                item.isShortcut = true;
            }
        }

        subItems.push_back(item);

        // �����Ŀ¼���ݹ��ռ�
        if (item.isDirectory)
        {
            CollectDirectoryFiles(fullPath, fileList);
        }

    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);

    // ��������ӵ��б��У��ļ���ǰ��Ŀ¼�ں󣬱���ɾ��ʱ��˳��
    for (const auto& item : subItems)
    {
        if (!item.isDirectory)
        {
            fileList.push_back(item);
        }
    }
    for (const auto& item : subItems)
    {
        if (item.isDirectory)
        {
            fileList.push_back(item);
        }
    }
}

std::string FileCollector::GetSpecialFolderPath(int csidl)
{
    TCHAR path[MAX_PATH];
    if (SHGetSpecialFolderPathA(HWND_DESKTOP, path, csidl, 0))
    {
        return std::string(path);
    }
    return "";
}

bool FileCollector::PathExists(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }

    DWORD attributes = GetFileAttributesA(path.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES);
}

bool FileCollector::IsDirectory(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }

    DWORD attributes = GetFileAttributesA(path.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}