#include "RegistryHelper.h"

std::string RegistryHelper::FindSoftwareInstallPath(const std::string& softwareName)
{
    if (softwareName.empty())
    {
        return "";
    }

    // 首先在WOW6432Node下查找（64位系统上的32位程序）
    std::string registryPath = "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + softwareName;
    std::string installPath = GetInstallLocationFromRegistry(registryPath);
    
    if (!installPath.empty())
    {
        return installPath;
    }

    // 如果没找到，再在常规路径下查找（64位程序）
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

    // 读取InstallLocation字段
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