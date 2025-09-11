#include "DotNetInstallServer.h"
#include <iostream>

bool DotNetInstallServer::Launch(const string& runTIme)
{
    TCHAR szCommandLine[] = " /q /norestart /pipe runTimeSection";
    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = { 0 };
    BOOL bLaunchedSetup = ::CreateProcess(
        runTIme.c_str(),
        szCommandLine,
        NULL, NULL, FALSE, 0, NULL, NULL,
        &si,
        &pi);

    //如果进程启动成功
    if (bLaunchedSetup)
    {
        IProgressObserver& observer = dynamic_cast<IProgressObserver&>(*this);
        Run(pi.hProcess, observer);//开始监控、直到安装完成

        DWORD exitCode = 0;
        ::GetExitCodeProcess(pi.hProcess, &exitCode);//获取进程结束码 https://learn.microsoft.com/zh-cn/dotnet/framework/deployment/how-to-get-progress-from-the-dotnet-installer
        HRESULT hrInternalResult = GetInternalResult();//获取内部结果 https://learn.microsoft.com/zh-cn/dotnet/framework/deployment/how-to-get-progress-from-the-dotnet-installer

        ::CloseHandle(pi.hThread);
        ::CloseHandle(pi.hProcess);

        if (exitCode == 0 || hrInternalResult != 0) //如果进程退出码等于0 或者内部结果不为0，简单的认为安装失败
        {
            return FALSE;
        }
        return TRUE;
    }
    else//如果创建进程启动失败
    {
        return FALSE;
    }
}

bool DotNetInstallServer::IsNetFx4Present(DWORD dwMinimumRelease)
{
     DWORD dwError = ERROR_SUCCESS;
     HKEY hKey = NULL;
     DWORD dwData = 0;
     DWORD dwType = 0;
     DWORD dwSize = sizeof(dwData);

     dwError = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full", 0, KEY_READ, &hKey);
     if (ERROR_SUCCESS == dwError)
     {
         dwError = ::RegQueryValueEx(hKey, "Release", 0, &dwType, (LPBYTE)&dwData, &dwSize);

         if ((ERROR_SUCCESS == dwError) && (REG_DWORD != dwType))
         {
             dwError = ERROR_INVALID_DATA;
         }
         else if (ERROR_FILE_NOT_FOUND == dwError)
         {
             // Release value was not found, let's check for 4.0.
             dwError = ::RegQueryValueEx(hKey, "Install", 0, &dwType, (LPBYTE)&dwData, &dwSize);

             // Install = (REG_DWORD)1;
             if ((ERROR_SUCCESS == dwError) && (REG_DWORD == dwType) && (dwData == 1))
             {
                 // treat 4.0 as Release = 0
                 dwData = 0;
             }
             else
             {
                 dwError = ERROR_INVALID_DATA;
             }
         }
     }

     if (hKey != NULL)
     {
         ::RegCloseKey(hKey);
     }

     return ((ERROR_SUCCESS == dwError) && (dwData >= dwMinimumRelease));
}
