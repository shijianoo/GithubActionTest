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

    //������������ɹ�
    if (bLaunchedSetup)
    {
        IProgressObserver& observer = dynamic_cast<IProgressObserver&>(*this);
        Run(pi.hProcess, observer);//��ʼ��ء�ֱ����װ���

        DWORD exitCode = 0;
        ::GetExitCodeProcess(pi.hProcess, &exitCode);//��ȡ���̽����� https://learn.microsoft.com/zh-cn/dotnet/framework/deployment/how-to-get-progress-from-the-dotnet-installer
        HRESULT hrInternalResult = GetInternalResult();//��ȡ�ڲ���� https://learn.microsoft.com/zh-cn/dotnet/framework/deployment/how-to-get-progress-from-the-dotnet-installer

        ::CloseHandle(pi.hThread);
        ::CloseHandle(pi.hProcess);

        if (exitCode == 0 || hrInternalResult != 0) //��������˳������0 �����ڲ������Ϊ0���򵥵���Ϊ��װʧ��
        {
            return FALSE;
        }
        return TRUE;
    }
    else//���������������ʧ��
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
