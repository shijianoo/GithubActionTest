#include <iostream>
#include <format>
#include <Shlwapi.h>
#include "UnInstallWindow.h"
#include "Helper.h"
#include "RegistryHelper.h"
#include "FileCollector.h"
#include "FileDeleter.h"

void UnInstallWindow::InitWindow()
{
	Window::InitWindow();
	SetIcon(108);

	unInstallBtn = dynamic_cast<CButtonUI*>(m_pm.FindControl("btnUninstall"));
	unInstallBtn->OnNotify += MakeDelegate(this, &UnInstallWindow::OnExecuteUnInstall);
	cancelBtn = dynamic_cast<CButtonUI*>(m_pm.FindControl("btnCancel"));
	cancelBtn->OnNotify += MakeDelegate(this, &Window::CloseWindow);

	progressBar = dynamic_cast<CSliderUI*>(m_pm.FindControl("progressBar"));
	progressState = dynamic_cast<CLabelUI*>(m_pm.FindControl("progressState"));
	progressPercent = dynamic_cast<CLabelUI*>(m_pm.FindControl("progressPercent"));

	auto uninstinfo = dynamic_cast<CLabelUI*>(m_pm.FindControl("uninstinfo"));
	uninstinfo->SetText(format("确定要卸载 {} 吗?", SOFT_NAME).c_str());

	auto title = dynamic_cast<CLabelUI*>(m_pm.FindControl("title"));
	title->SetText(format("{} 卸载程序", SOFT_NAME).c_str());

	auto uninstinginfo = dynamic_cast<CLabelUI*>(m_pm.FindControl("uninstinginfo"));
	uninstinginfo->SetText(format("正在卸载 {}", SOFT_NAME).c_str());

	auto thanksinfo = dynamic_cast<CLabelUI*>(m_pm.FindControl("thanksinfo"));
	thanksinfo->SetText(format("感谢使用 {}", SOFT_NAME).c_str());
}

bool UnInstallWindow::OnExecuteUnInstall(void* param)
{
	if (!IsClick(param)) return true;

	// 查找软件安装路径
	RegistryHelper registryHelper;
	std::string installPath = registryHelper.FindSoftwareInstallPath(SOFT_NAME);
	if (installPath.empty() || !m_fileCollector.PathExists(installPath))
	{
		MessageBox(m_hWnd, "找不到卸载路径！", format("{} 卸载程序", SOFT_NAME).c_str(), MB_OK | MB_ICONERROR);
		return true;
	}

	std::vector<FileItem> filesToDelete = m_fileCollector.CollectFilesToDelete(SOFT_NAME, installPath);
	if (filesToDelete.empty())
	{
		MessageBox(m_hWnd, "找不到路径！", format("{} 卸载程序", SOFT_NAME).c_str(), MB_OK | MB_ICONERROR);
		return true;
	}
	ToPage(1);
	SetCloseBtnEnabled(false);

	m_fileDeleter.DeleteFilesAsync(filesToDelete,
		// 进度回调
		[this](int current, int total, const std::string& currentFile, bool success) {
			OnUninstallProgress(current, total, currentFile, success);
		},
		// 完成回调
		[this]() {
			OnUninstallCompleted();
		});

	return true;
}

void UnInstallWindow::OnUninstallProgress(int current, int total, const std::string& currentFile, bool success)
{
	// 更新进度百分比
	int percent = (current * 100) / total;
	progressPercent->SetText(format("{}%", percent).c_str());
	progressBar->SetValue(percent);

	// 更新当前删除的文件显示
	std::string fileName = currentFile;
	size_t lastSlash = fileName.find_last_of("\\/");
	if (lastSlash != std::string::npos) {
		fileName = fileName.substr(lastSlash + 1);
	}

	std::string statusText = format("({}/{}) {}", current, total, fileName);
	progressState->SetText(statusText.c_str());

}

void UnInstallWindow::OnUninstallCompleted()
{
	for (int i = 0; i < m_fileCollector.changeNotifies.size(); i++)
	{
		const string& item = m_fileCollector.changeNotifies[i];
		SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, item.c_str(), NULL);
	}

	progressState->SetText("删除注册表项...");

	//删除卸载注册表
	string path = "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
	path += SOFT_NAME;
	SHDeleteKey(HKEY_LOCAL_MACHINE, path.c_str());

	//删除文件关联
	SHDeleteKey(HKEY_CLASSES_ROOT, ".sstd");
	SHDeleteKey(HKEY_CLASSES_ROOT, "sstdfile");

	//删除仪器监听进程的开机自动启动
	HKEY hKey;
	RegOpenKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
	RegDeleteValue(hKey, TEXT("ShallowSeaMonitor"));
	RegDeleteValue(hKey, TEXT("ShallowSea"));
	RegCloseKey(hKey);

	::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	::PostMessage(HWND_BROADCAST, WM_COMMAND, 41504, NULL);

	ToPage(COMPLETEDPAGEINDEX);
	SetCloseBtnEnabled(true);
}
