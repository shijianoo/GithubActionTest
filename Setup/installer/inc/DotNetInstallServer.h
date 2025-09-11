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

    //��鱾���Ƿ�װ��ָ�������ϰ汾��.net ����true���Ѱ�װ������false��δ��װ
    static bool IsNetFx4Present(DWORD dwMinimumRelease);
    

private:
    inline void OnInstallProgress(int progress)
    {
        window->SetProgress(progress);
    }

};