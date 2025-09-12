#include "InstallHandler.h"
#include "Helper.h"
#include "config.h"
#include "DotNetInstallServer.h"

string InstallHandler::ExecuteInstall()
{
	ToPage(INSTALLINGPAGEINDEX);
	//退出所有与软件名称相关的进程
	KillProcessByExactName(SOFT_NAME);
	InstallRunTime();
	InstallApplication();
	NotifyShell(installDir);
	RegisterApplication();
	CreateShortcutEx();
	RegisterDataFile();
	InstallAutoRunMonitor();
	PostMessage(HWND_BROADCAST, WM_COMMAND, 41504, NULL);
	ToPage(COMPLETEDPAGEINDEX);
	return appPath;
}

void InstallHandler::InstallRunTime()
{
	SetState("正在检查运行时环境");
	path runTimePath(installDir);
	runTimePath.append("NET48RUNTIME.exe");
	bool result = FreeResFile(112, "EXE", runTimePath.string().c_str());

	if (!DotNetInstallServer::IsNetFx4Present(NETVERSION))
	{
		SetState("正在安装运行时环境...");
		SetProgress(0);
		DotNetInstallServer install(this->window);
		install.Launch(runTimePath.string());
	}
}

void InstallHandler::InstallApplication()
{
	LPVOID pRes = NULL;
	DWORD ziplen = 0;
	if (!LoadResourceData(110, "zip", pRes, ziplen)) return;
	HZIP hz = OpenZip(pRes, ziplen, 0);
	SetUnzipBaseDir(hz, installDir.c_str());
	if (hz == NULL) return;
	ZIPENTRY ze;
	GetZipItem(hz, -1, &ze);
	int numitems = ze.index;
	float rate = 100.0f / numitems;
	for (int i = 0; i < numitems; i++)
	{
		GetZipItem(hz, i, &ze);
		DWORD dwRet = UnzipItem(hz, i, ze.name);
		SetProgress(static_cast<int>(rate * (i + 1)));
		SetState(ze.name);
		if (dwRet != 0)
		{
			CloseZipU(hz);
			return;
		}
	}
	CloseZipU(hz);
}

void InstallHandler::InstallAutoRunMonitor()
{
	if (IsAutoRunMonitor())
	{
		HKEY hKey;
		RegOpenKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
		string regValue = format("\"{}\" Monitor --path \"{}\" --sp", appPath, appPath);
		RegSetValueEx(hKey, "ShallowSeaMonitor", 0, REG_SZ, (LPCBYTE)regValue.c_str(), regValue.size() * sizeof(CHAR));
		RegCloseKey(hKey);
	}
}

void InstallHandler::RegisterApplication()
{
	SetState("创建注册表项...");
	DWORD dwError;
	HKEY hKey = NULL;
	DWORD dwDisposition;
	path uninstallRegKey("SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
	uninstallRegKey.append(SOFT_NAME);
	string keyPath = uninstallRegKey.string();
	dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	if (dwError == ERROR_SUCCESS)
	{
		//显示名称
		RegSetValueEx(hKey, "DisplayName", 0, REG_SZ, (LPCBYTE)SOFT_NAME, (lstrlen(SOFT_NAME) + 1) * sizeof(CHAR));
		
		//设置exe图标
		RegSetValueEx(hKey, "DisplayIcon", 0, REG_SZ, (LPCBYTE)appPath.c_str(), appPath.size() * sizeof(CHAR));
		
		//卸载程序路径
		RegSetValueEx(hKey, "UninstallString", 0, REG_SZ, (LPCBYTE)uninstallPath.c_str(), uninstallPath.size() * sizeof(CHAR));
		
		//发布者
		string publisher = "杭州浅海科技有限责任公司";
		RegSetValueEx(hKey, "Publisher", 0, REG_SZ, (LPCBYTE)publisher.c_str(), publisher.size() * sizeof(CHAR));

		//软件大小
		DWORD estimatedSize = GetDirectorySize(installDir) / 1024;
		RegSetValueEx(hKey, "EstimatedSize", 0, REG_DWORD, (LPCBYTE)&estimatedSize, sizeof(DWORD));

		//版本
		RegSetValueEx(hKey, "DisplayVersion", 0, REG_SZ, (LPCBYTE)SOFT_VERSION, (lstrlen(SOFT_VERSION) + 1) * sizeof(CHAR));

		//支持链接
		string urlInfo = "http://www.shallow-sea.com/";
		RegSetValueEx(hKey, "URLInfoAbout", 0, REG_SZ, (LPCBYTE)urlInfo.c_str(), urlInfo.size() * sizeof(CHAR));

		//安装位置
		RegSetValueEx(hKey, "InstallLocation", 0, REG_SZ, (LPCBYTE)installDir.c_str(), installDir.size() * sizeof(CHAR));
	}
	RegCloseKey(hKey);
	return VOID();
}

void InstallHandler::CreateShortcutEx()
{
	TCHAR dir[MAX_PATH];
	SHGetSpecialFolderPath(HWND_DESKTOP, dir, CSIDL_COMMON_PROGRAMS, 0);
	string programs(dir);

	SHGetSpecialFolderPath(HWND_DESKTOP, dir, CSIDL_COMMON_DESKTOPDIRECTORY, 0);
	string desktop(dir);
	
	string aprogramsDir = path(programs).append(SOFT_NAME).string();
	bool result = CreateMultipleDirectory(NULL, aprogramsDir);
	if (!result) return;

	string applnk = path(aprogramsDir).append(SOFT_NAME + string(".lnk")).string();
	CreateShortcut(applnk, appPath, SOFT_NAME);

	string uninstallName = format("卸载{}", SOFT_NAME);
	string uninstallLnk = path(aprogramsDir).append(uninstallName + ".lnk").string();
	CreateShortcut(uninstallLnk, uninstallPath, uninstallName);
	NotifyShell(applnk);
	NotifyShell(uninstallLnk);
	if (IsCreateShortCut())
	{
		string desktopApplnk = path(desktop).append(SOFT_NAME + string(".lnk")).string();
		CreateShortcut(desktopApplnk, appPath, SOFT_NAME);
		NotifyShell(desktopApplnk);
	}
}

void InstallHandler::RegisterDataFile()
{
	string iconPath = path(installDir).append("sstdfile.ico").string();
	string openPath = appPath + " \"sstd\" \"--f\" \"%1\"";
	RegisterFileRelation(".sstd", openPath, "sstdfile", iconPath, "ShallowSea Tech Data File");
}
