#pragma once

#include "MmIoChainer.h"
#include "InstallWindow.h"

class DotNetInstallServer : public MmioChainer, public IProgressObserver
{
public:
    InstallWindow* window;
    DotNetInstallServer(InstallWindow* window) : MmioChainer(L"runTimeSection", L"runTimeEvent")
    {
        this->window = window;
    }

    bool Launch(const string& runTIme);

    //检查本机是否安装了指定或以上版本的.net 返回true：已安装，返回false：未安装
    static bool IsNetFx4Present(DWORD dwMinimumRelease);
    

private:
    inline void OnInstallProgress(int progress)
    {
        window->SetProgress(progress);
    }

};