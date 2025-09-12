#pragma once
#include <string>
#include "InstallWindow.h"

class InstallHandler {
private:
	string installDir;
    string appPath;
    string uninstallPath;
    InstallWindow* window;

    inline void ToPage(int index) 
    {
        if (window != NULL) {
            window->ToPage(index);
        }
    }

    inline void SetState(const string& text)
    {
        if (window != NULL) {
            window->SetProgressState(text);
        }
    }

    inline void SetProgress(const int percent) {
        if (window != NULL) {
            window->SetProgress(percent);
        }
    }

    inline bool IsCreateShortCut()
    {
        if (window != NULL) {
            return window->IsCreateShortCut();
        }
        return false;
    }

    inline bool IsAutoRunMonitor()
    {
        if (window != NULL) {
            return window->IsAutoRunMonitor();
        }
        return false;
    }

public:
    InstallHandler(const std::string& dir, InstallWindow* window)
    {
        this->installDir = dir;
        appPath = format("{}\\{}.exe", installDir, SOFT_NAME);
        uninstallPath = path(installDir).append("Uninstaller.exe").string().c_str();
        this->window = window;
    }

    string ExecuteInstall();

    void InstallRunTime();
    void InstallApplication();
    void InstallAutoRunMonitor();
    void RegisterApplication();
    void CreateShortcutEx();
    void RegisterDataFile();
};