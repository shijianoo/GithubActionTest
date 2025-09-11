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
    HINSTANCE hinst = LoadLibrary(TEXT("ntdll.dll"));//����DLL
    if (hinst == NULL) return true;
    NTPROC GetNtVersionNumbers = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers");//��ȡ������ַ
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
    // ��ʼ�� BROWSEINFO �ṹ
    BROWSEINFO bi = { 0 };
    bi.hwndOwner = hwndOwner;
    bi.lpszTitle = _T("ѡ��װĿ¼");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    // ��������ļ��жԻ���
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != NULL) {
        TCHAR path[MAX_PATH];
        // ��ȡѡ����ļ���·��
        if (SHGetPathFromIDList(pidl, path)) {
            CoTaskMemFree(pidl); // �ͷ� PIDL
            return path; // ����ѡ���·��
        }
        CoTaskMemFree(pidl); // �ͷ� PIDL
    }
    return ""; // ���ؿ��ַ�����ʾδѡ��
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

    std::string errorMessage = "δ֪����";

    if (dwSize) {
        // �����صĴ�����Ϣת��Ϊ std::string
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

// �������н���������ָ���ַ����Ľ���
bool KillProcessByExactName(const std::string& processName) {
    if (processName.empty()) {
        return true;
    }

    // �Զ���� .exe ��׺�����û�еĻ���
    std::string exactName = processName;
    if (exactName.length() < 4 ||
        exactName.substr(exactName.length() - 4) != ".exe") {
        exactName += ".exe";
    }

    // ת��Ϊ���ַ�
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, exactName.c_str(), -1, NULL, 0);
    std::wstring wExactName(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, exactName.c_str(), -1, &wExactName[0], size_needed);
    wExactName.pop_back(); // �Ƴ�ĩβ��null�ַ�

    // ת��ΪСд���ڱȽ�
    std::transform(wExactName.begin(), wExactName.end(), wExactName.begin(), ::towlower);

    // �������̿���
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    std::vector<DWORD> targetPids;
    DWORD currentPid = GetCurrentProcessId();

    // �������н���
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            // ������ǰ���̣�������ɱ
            if (pe32.th32ProcessID == currentPid) {
                continue;
            }

            std::wstring processName = pe32.szExeFile;
            std::transform(processName.begin(), processName.end(), processName.begin(), ::towlower);

            // ��ȷƥ�������
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

    // ��������Ŀ�����
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
    // ��ȡ��ǰexe·��
    char exePath[MAX_PATH]{};
    DWORD result = GetModuleFileNameA(NULL, exePath, MAX_PATH);
    if (result == 0 || result >= MAX_PATH) {
        return false;
    }

    // ��ȡexe����Ŀ¼
    char exeDir[MAX_PATH]{};
    lstrcpyA(exeDir, exePath);
    if (!PathRemoveFileSpecA(exeDir)) {
        return false;
    }

    char batPath[MAX_PATH]{};
    sprintf_s(batPath, "%s\\selfDel.bat", exeDir);

    // �����������ļ�
    std::ofstream batFile(batPath, std::ios::out);
    if (!batFile.is_open()) {
        return false;
    }

    // д������������
    std::string batContent =
        "@echo off\r\n"
        "title Self Delete Batch\r\n"
        "echo Waiting for process to terminate...\r\n"
        "timeout /t 2 /nobreak >nul\r\n"  // �ȴ�2��ȷ��exe��ȫ�˳�
        "\r\n"
        ":DeleteLoop\r\n"
        "del /f /q \"" + std::string(exePath) + "\"\r\n"
        "if exist \"" + std::string(exePath) + "\" (\r\n"
        "    timeout /t 1 /nobreak >nul\r\n"
        "    goto DeleteLoop\r\n"
        ")\r\n"
        "\r\n"
        "echo File deleted successfully.\r\n"
        "del /f /q \"%~f0\"\r\n"  // ɾ��bat����
        "exit\r\n";

    batFile << batContent;
    batFile.close();

    // ִ���������ļ������ش��ڣ�
    HINSTANCE shellResult = ShellExecuteA(
        NULL,
        "open",
        batPath,
        NULL,
        NULL,
        SW_HIDE
    );

    // ���ShellExecute�Ƿ�ɹ�
    return (intptr_t)shellResult > 32;
}

bool IsValidInstallPath(const std::string& path, HWND hWnd) {
    auto ShowError = [hWnd](const std::string& message) {
        if (hWnd) {
            MessageBoxA(hWnd, message.c_str(), "��װ·������", MB_OK | MB_ICONERROR);
        }
    };

    // ����·��
    if (path.empty()) {
        ShowError("��װ·������Ϊ�գ���ѡ��һ����Ч�İ�װĿ¼��");
        return false;
    }

    // ���·������
    if (path.length() > 260) {
        ShowError("��װ·����������ѡ��һ���϶̵�·����������260���ַ�����");
        return false;
    }

    // ������·����ʽ�������� C:\ ��ʽ
    if (path.length() < 3) {
        ShowError("��װ·����ʽ����ȷ����ѡ��������·�����磺C:\\Program Files\\AppName����");
        return false;
    }

    // ����̷�
    if (!std::isalpha(path[0])) {
        ShowError("��װ·����������Ч���̷���ͷ���磺C��D��E�ȣ���");
        return false;
    }

    // ���ð��
    if (path[1] != ':') {
        ShowError("��װ·����ʽ����ȷ���̷�������ð�ţ��磺C:����");
        return false;
    }

    // ��������·���ָ��������ǹؼ���飬���� C:Program Files ���������
    if (path[2] != '\\' && path[2] != '/') {
        ShowError("��װ·����ʽ����ȷ��ӦΪ \"C:\\\" ������ \"C:\"��\n��ȷ���̷����з�б�ܡ�");
        return false;
    }

    // �����Ч�ַ�
    const std::string invalidChars = "<>:\"|?*";
    for (size_t i = 3; i < path.length(); ++i) {  // �ӵ�4���ַ���ʼ��飨���� C:\��
        char c = path[i];

        // �������ַ�
        if (static_cast<unsigned char>(c) < 32) {
            ShowError("��װ·��������Ч�Ŀ����ַ�����ѡ������·����");
            return false;
        }

        // �����Ч�ַ�
        if (invalidChars.find(c) != std::string::npos) {
            std::string msg = "��װ·��������Ч�ַ���\"" + std::string(1, c) + "\"\n";
            msg += "·���в��ܰ��������ַ���< > : \" | ? *";
            ShowError(msg);
            return false;
        }
    }

    // ���·�������Կո����β
    if (path.back() == ' ') {
        ShowError("��װ·�������Կո��β�����޸�·����");
        return false;
    }

    if (path.back() == '.') {
        ShowError("��װ·�������Ե�Ž�β�����޸�·����");
        return false;
    }

    return true;
}