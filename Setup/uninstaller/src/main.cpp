#include <iostream>
#include <fstream>
#include <filesystem>
#include "UnInstallWindow.h"
#include "Helper.h"

// ����ж�س���
static int LaunchUninstaller()
{
    using namespace std::filesystem;

    char sourcePath[MAX_PATH]{};
    if (GetModuleFileNameA(NULL, sourcePath, MAX_PATH) == 0) {
        MessageBoxA(NULL, "��ȡ����·��ʧ�ܣ�", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    char tempPath[MAX_PATH]{};
    if (GetTempPathA(MAX_PATH, tempPath) == 0) {
        MessageBoxA(NULL, "��ȡ��ʱĿ¼ʧ�ܣ�", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    path targetPath = path(tempPath);
    targetPath.append(path(sourcePath).filename().string());

    // �����ļ�����ʱĿ¼
    if (!CopyFileA(sourcePath, targetPath.string().c_str(), FALSE)) {
        DWORD error = GetLastError();
        char errorMsg[256];
        sprintf_s(errorMsg, "�ļ�����ʧ�ܣ��������: %lu", error);
        MessageBoxA(NULL, errorMsg, "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // ����ж�ؽ���
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    std::string cmdLine = "\"" + targetPath.string() + "\" Uninstall";

    BOOL ok = CreateProcessA(
        NULL,
        const_cast<char*>(cmdLine.c_str()),
        NULL, NULL, FALSE, 0, NULL, NULL,
        &si, &pi);

    if (ok) {
        // �رվ����ֹ��Դй¶
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 0;
    }
    else {
        MessageBoxA(NULL, "ж�ؽ�������ʧ�ܣ�", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    bool hasUninstallParam = (lpCmdLine != nullptr && strstr(lpCmdLine, "Uninstall") != nullptr);
    if (!hasUninstallParam) {
        return LaunchUninstaller();
    }
    else {
        UnInstallWindow window;
        window.Show();
        DeleteApplicationSelfWithBat();
    }
	return 0;
}