#include "helper.h"
#include <shlobj.h>
#include <tchar.h>
#include <filesystem>
#include <fstream>
#include <tlhelp32.h>
#include <locale>
#include <codecvt>
#include <Shlwapi.h>

using namespace std::filesystem;

bool IsXP()
{
    typedef void(__stdcall* NTPROC)(DWORD*, DWORD*, DWORD*);
    HINSTANCE hinst = LoadLibrary(TEXT("ntdll.dll"));//加载DLL
    if (hinst == NULL) return true;
    NTPROC GetNtVersionNumbers = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers");//获取函数地址
    DWORD dwMajor, dwMinor, dwBuildNumber;
    GetNtVersionNumbers(&dwMajor, &dwMinor, &dwBuildNumber);

    if (dwMajor < 6) return true;
    return false;
}

wstring ConvertToWString(const string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    return wstr;
}


string ShowFolderDialog(HWND hwndOwner) {
    // 初始化 BROWSEINFO 结构
    BROWSEINFO bi = { 0 };
    bi.hwndOwner = hwndOwner;
    bi.lpszTitle = _T("选择安装目录");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    // 调用浏览文件夹对话框
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != NULL) {
        TCHAR path[MAX_PATH];
        // 获取选择的文件夹路径
        if (SHGetPathFromIDList(pidl, path)) {
            CoTaskMemFree(pidl); // 释放 PIDL
            return path; // 返回选择的路径
        }
        CoTaskMemFree(pidl); // 释放 PIDL
    }
    return ""; // 返回空字符串表示未选择
}

bool LoadResourceData(DWORD  redId, const string& resType, LPVOID& pRes, DWORD& dwResSize)
{
    HMODULE hInstance = ::GetModuleHandle(NULL);
    HRSRC hResID = ::FindResource(hInstance, MAKEINTRESOURCE(redId), resType.c_str());
    if (hResID == 0) return false;

    HGLOBAL hRes = ::LoadResource(hInstance, hResID);
    if (hRes == 0) return false;

    pRes = ::LockResource(hRes);
    if (pRes == NULL) return false;

    dwResSize = ::SizeofResource(hInstance, hResID);
    return TRUE;
}

bool FreeResFile(DWORD redId, const string& resType, const string& fileName)
{
    LPVOID pRes = NULL;
    DWORD dwResSize = 0;
    if (!LoadResourceData(redId, resType, pRes, dwResSize))return false;
    HANDLE hResFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
    if (INVALID_HANDLE_VALUE == hResFile) return false;
    DWORD dwWritten = 0;
    WriteFile(hResFile, pRes, dwResSize, &dwWritten, NULL);
    CloseHandle(hResFile);
    return (dwResSize == dwWritten);
}

DWORD GetDirectorySize(const string& directory)
{
    DWORD totalSize = 0;
    for (const auto& entry : recursive_directory_iterator(directory)) {
        if (is_regular_file(entry.status())) {
            totalSize += file_size(entry);
        }
    }
    return totalSize;
}

void CreateShortcut(const string& shortcutPath, const string& targetPath, const string& desc)
{
    HRESULT hres = CoInitialize(NULL);
    if (SUCCEEDED(hres))
    {
        IShellLink* pShellLink = NULL;
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);
        if (SUCCEEDED(hres)) {
            pShellLink->SetPath(targetPath.c_str());
            pShellLink->SetDescription(desc.c_str());

            IPersistFile* pPersistFile = NULL;
            hres = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
            if (SUCCEEDED(hres)) {
                WCHAR wszPath[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, shortcutPath.c_str(), -1, wszPath, MAX_PATH);
                hres = pPersistFile->Save(wszPath, TRUE);
                pPersistFile->Release();
            }
            pShellLink->Release();
        }
    }
    CoUninitialize();
}

int KillProcess(string processName)
{
    HANDLE hSnapshort = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshort == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    PROCESSENTRY32 stcProcessInfo;
    stcProcessInfo.dwSize = sizeof(stcProcessInfo);
    BOOL  bRet = Process32First(hSnapshort, &stcProcessInfo);
    while (bRet)
    {
        if (strcmp(stcProcessInfo.szExeFile, processName.c_str()) == 0)
        {
            HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, stcProcessInfo.th32ProcessID);
            ::TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
        bRet = Process32Next(hSnapshort, &stcProcessInfo);
    }

    CloseHandle(hSnapshort);
    return 0;
}

void RegisterFileRelation(string strExt, string strAppName, string strAppKey, string strDefaultIcon, string strDescribe)
{
    HKEY hKey;

    RegCreateKey(HKEY_CLASSES_ROOT, strExt.c_str(), &hKey);
    RegSetValue(hKey, NULL, REG_SZ, strAppKey.c_str(), strAppKey.length() * sizeof(CHAR));
    RegCloseKey(hKey);

    RegCreateKey(HKEY_CLASSES_ROOT, strAppKey.c_str(), &hKey);
    RegSetValue(hKey, NULL, REG_SZ, strDescribe.c_str(), strDescribe.length() * sizeof(CHAR));
    RegCloseKey(hKey);

    string icon;
    icon.append(strAppKey);
    icon.append("\\DefaultIcon");
    RegCreateKey(HKEY_CLASSES_ROOT, icon.c_str(), &hKey);
    RegSetValue(hKey, NULL, REG_SZ, strDefaultIcon.c_str(), strDefaultIcon.length() * sizeof(CHAR));
    RegCloseKey(hKey);

    string shell;
    shell.append(strAppKey);
    shell.append("\\Shell");
    RegCreateKey(HKEY_CLASSES_ROOT, shell.c_str(), &hKey);
    RegCloseKey(hKey);

    string command = shell;
    command.append("\\open\\command");
    RegCreateKey(HKEY_CLASSES_ROOT, command.c_str(), &hKey);
    RegSetValue(hKey, NULL, REG_SZ, strAppName.c_str(), strAppName.length() * sizeof(CHAR));
    RegCloseKey(hKey);
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

void NotifyShell(string path)
{
    SHChangeNotify(SHCNE_MKDIR | SHCNE_INTERRUPT, SHCNF_FLUSH | SHCNF_PATH, path.c_str(), 0);
    SHChangeNotify(SHCNE_UPDATEDIR | SHCNE_INTERRUPT, SHCNF_FLUSH | SHCNF_PATH, path.c_str(), 0);
}

string GetErrorMessage(DWORD errorCode)
{
    LPSTR messageBuffer = nullptr;
    DWORD dwSize = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorCode, 0, (LPSTR)&messageBuffer, 0, nullptr);

    std::string errorMessage = "未知错误";

    if (dwSize) {
        // 将返回的错误信息转换为 std::string
        errorMessage = messageBuffer;
        LocalFree(messageBuffer);
    }
    return errorMessage;
}

bool IsFileExist(const string& file)
{
    DWORD dwAttrib = GetFileAttributes(file.c_str());
    return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

bool IsDirExist(const string& dir)
{
    DWORD dwAttrib = GetFileAttributes(dir.c_str());
    return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

wstring StringToWstring(const string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}


bool CreateMultipleDirectory(HWND hWnd, const string& directory)
{
    if(IsDirExist(directory)) return true;
    DWORD dwError = SHCreateDirectoryEx(hWnd, directory.c_str(), NULL);
    if (dwError == ERROR_SUCCESS) 
    {
        SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, directory.c_str(), directory.c_str());
        return true;
    }

    if(dwError == ERROR_CANCELLED)  return false;

    auto error = GetErrorMessage(dwError);
    MessageBox(hWnd, error.c_str(), NULL, MB_OK | MB_ICONERROR);
    return false;
}

// 结束所有进程名包含指定字符串的进程
bool KillProcessByExactName(const std::string& processName) {
    if (processName.empty()) {
        return true;
    }

    // 自动添加 .exe 后缀（如果没有的话）
    std::string exactName = processName;
    if (exactName.length() < 4 ||
        exactName.substr(exactName.length() - 4) != ".exe") {
        exactName += ".exe";
    }

    // 转换为宽字符
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, exactName.c_str(), -1, NULL, 0);
    std::wstring wExactName(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, exactName.c_str(), -1, &wExactName[0], size_needed);
    wExactName.pop_back(); // 移除末尾的null字符

    // 转换为小写用于比较
    std::transform(wExactName.begin(), wExactName.end(), wExactName.begin(), ::towlower);

    // 创建进程快照
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    std::vector<DWORD> targetPids;
    DWORD currentPid = GetCurrentProcessId();

    // 遍历所有进程
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            // 跳过当前进程，避免自杀
            if (pe32.th32ProcessID == currentPid) {
                continue;
            }

            std::wstring processName = pe32.szExeFile;
            std::transform(processName.begin(), processName.end(), processName.begin(), ::towlower);

            // 精确匹配进程名
            if (processName == wExactName) {
                targetPids.push_back(pe32.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    if (targetPids.empty()) {
        return true;
    }

    bool success = true;

    // 结束所有目标进程
    for (DWORD pid : targetPids) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (!hProcess) {
            success = false;
            continue;
        }

        if (!TerminateProcess(hProcess, 1)) {
            success = false;
        }

        CloseHandle(hProcess);
    }

    return success;
}

bool DeleteApplicationSelfWithBat()
{
    // 获取当前exe路径
    char exePath[MAX_PATH]{};
    DWORD result = GetModuleFileNameA(NULL, exePath, MAX_PATH);
    if (result == 0 || result >= MAX_PATH) {
        return false;
    }

    // 获取exe所在目录
    char exeDir[MAX_PATH]{};
    lstrcpyA(exeDir, exePath);
    if (!PathRemoveFileSpecA(exeDir)) {
        return false;
    }

    char batPath[MAX_PATH]{};
    sprintf_s(batPath, "%s\\selfDel.bat", exeDir);

    // 创建批处理文件
    std::ofstream batFile(batPath, std::ios::out);
    if (!batFile.is_open()) {
        return false;
    }

    // 写入批处理内容
    std::string batContent =
        "@echo off\r\n"
        "title Self Delete Batch\r\n"
        "echo Waiting for process to terminate...\r\n"
        "timeout /t 2 /nobreak >nul\r\n"  // 等待2秒确保exe完全退出
        "\r\n"
        ":DeleteLoop\r\n"
        "del /f /q \"" + std::string(exePath) + "\"\r\n"
        "if exist \"" + std::string(exePath) + "\" (\r\n"
        "    timeout /t 1 /nobreak >nul\r\n"
        "    goto DeleteLoop\r\n"
        ")\r\n"
        "\r\n"
        "echo File deleted successfully.\r\n"
        "del /f /q \"%~f0\"\r\n"  // 删除bat自身
        "exit\r\n";

    batFile << batContent;
    batFile.close();

    // 执行批处理文件（隐藏窗口）
    HINSTANCE shellResult = ShellExecuteA(
        NULL,
        "open",
        batPath,
        NULL,
        NULL,
        SW_HIDE
    );

    // 检查ShellExecute是否成功
    return (intptr_t)shellResult > 32;
}

bool IsValidInstallPath(const std::string& path, HWND hWnd) {
    auto ShowError = [hWnd](const std::string& message) {
        if (hWnd) {
            MessageBoxA(hWnd, message.c_str(), "安装路径错误", MB_OK | MB_ICONERROR);
        }
    };

    // 检查空路径
    if (path.empty()) {
        ShowError("安装路径不能为空，请选择一个有效的安装目录。");
        return false;
    }

    // 检查路径长度
    if (path.length() > 260) {
        ShowError("安装路径过长，请选择一个较短的路径（不超过260个字符）。");
        return false;
    }

    // 检查绝对路径格式：必须是 C:\ 格式
    if (path.length() < 3) {
        ShowError("安装路径格式不正确，请选择完整的路径（如：C:\\Program Files\\AppName）。");
        return false;
    }

    // 检查盘符
    if (!std::isalpha(path[0])) {
        ShowError("安装路径必须以有效的盘符开头（如：C、D、E等）。");
        return false;
    }

    // 检查冒号
    if (path[1] != ':') {
        ShowError("安装路径格式不正确，盘符后必须跟冒号（如：C:）。");
        return false;
    }

    // 检查必须有路径分隔符（这是关键检查，避免 C:Program Files 这种情况）
    if (path[2] != '\\' && path[2] != '/') {
        ShowError("安装路径格式不正确，应为 \"C:\\\" 而不是 \"C:\"。\n请确保盘符后有反斜杠。");
        return false;
    }

    // 检查无效字符
    const std::string invalidChars = "<>:\"|?*";
    for (size_t i = 3; i < path.length(); ++i) {  // 从第4个字符开始检查（跳过 C:\）
        char c = path[i];

        // 检查控制字符
        if (static_cast<unsigned char>(c) < 32) {
            ShowError("安装路径包含无效的控制字符，请选择其他路径。");
            return false;
        }

        // 检查无效字符
        if (invalidChars.find(c) != std::string::npos) {
            std::string msg = "安装路径包含无效字符：\"" + std::string(1, c) + "\"\n";
            msg += "路径中不能包含以下字符：< > : \" | ? *";
            ShowError(msg);
            return false;
        }
    }

    // 检查路径不能以空格或点结尾
    if (path.back() == ' ') {
        ShowError("安装路径不能以空格结尾，请修改路径。");
        return false;
    }

    if (path.back() == '.') {
        ShowError("安装路径不能以点号结尾，请修改路径。");
        return false;
    }

    return true;
}