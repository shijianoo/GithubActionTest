#include "RegistryHelper.h"

std::string RegistryHelper::FindSoftwareInstallPath(const std::string& softwareName)
{
    if (softwareName.empty())
    {
        return "";
    }

    // ������WOW6432Node�²��ң�64λϵͳ�ϵ�32λ����
    std::string registryPath = "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + softwareName;
    std::string installPath = GetInstallLocationFromRegistry(registryPath);
    
    if (!installPath.empty())
    {
        return installPath;
    }

    // ���û�ҵ������ڳ���·���²��ң�64λ����
    registryPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + softwareName;
    installPath = GetInstallLocationFromRegistry(registryPath);

    return installPath;
}

std::string RegistryHelper::GetInstallLocationFromRegistry(const std::string& registryPath)
{
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, registryPath.c_str(), 0, KEY_READ, &hKey);
    
    if (result != ERROR_SUCCESS)
    {
        return "";
    }

    // ��ȡInstallLocation�ֶ�
    char installLocation[512] = {0};
    DWORD installLocationSize = sizeof(installLocation);
    DWORD type;
    
    result = RegQueryValueExA(hKey, "InstallLocation", NULL, &type, 
        reinterpret_cast<LPBYTE>(installLocation), &installLocationSize);
    
    RegCloseKey(hKey);
    
    if (result == ERROR_SUCCESS && type == REG_SZ)
    {
        return std::string(installLocation);
    }

    return "";
}