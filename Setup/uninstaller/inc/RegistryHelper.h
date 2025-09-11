#pragma once

#include <windows.h>
#include <string>

class RegistryHelper
{
public:
    /**
     * 根据软件名称查找其安装路径
     * @param softwareName 软件名称
     * @return 如果找到返回安装路径，否则返回空字符串
     */
    std::string FindSoftwareInstallPath(const std::string& softwareName);

private:
    /**
     * 检查指定注册表路径下的软件安装信息
     * @param registryPath 完整的注册表路径
     * @return 如果找到InstallLocation返回路径，否则返回空字符串
     */
    std::string GetInstallLocationFromRegistry(const std::string& registryPath);
};