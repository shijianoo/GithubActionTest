#include "InstallWindow.h"
#include "config.h"
#include "Helper.h"
#include "InstallHandler.h"

void InstallWindow::InitWindow() {
	Window::InitWindow();
	SetIcon(108);
	auto openOptionsBtn = dynamic_cast<CButtonUI*>(m_pm.FindControl("btnOptions"));
	openOptionsBtn->OnNotify += MakeDelegate(this, &InstallWindow::OpenOption);

	auto closeOptionsBtn = dynamic_cast<CButtonUI*>(m_pm.FindControl("closeOptionsBtn"));
	closeOptionsBtn->OnNotify += MakeDelegate(this, &InstallWindow::CloseOption);

	auto btnBrowser = dynamic_cast<CButtonUI*>(m_pm.FindControl("btnBrowser"));
	btnBrowser->OnNotify += MakeDelegate(this, &InstallWindow::InstallDirSelect);

	installDirEdit = dynamic_cast<CRichEditUI*>(m_pm.FindControl("installPath"));
	path defaultPath = L"C:\\Program Files";
	defaultPath.append(SOFT_NAME);
	installDirEdit->SetText(defaultPath.string().c_str());

	installBtn1 = dynamic_cast<CButtonUI*>(m_pm.FindControl("installBtn1"));
	installBtn1->OnNotify += MakeDelegate(this, &InstallWindow::OnExecuteInstall);
	installBtn2 = dynamic_cast<CButtonUI*>(m_pm.FindControl("installBtn2"));
	installBtn2->OnNotify += MakeDelegate(this, &InstallWindow::OnExecuteInstall);

	progressBar = dynamic_cast<CSliderUI*>(m_pm.FindControl("progressBar"));
	progressState = dynamic_cast<CLabelUI*>(m_pm.FindControl("progressState"));
	progressPercent = dynamic_cast<CLabelUI*>(m_pm.FindControl("progressPercent"));

	chkShortCut = dynamic_cast<CCheckBoxUI*>(m_pm.FindControl("chkShortCut"));
	chkMonitorAutoRun = dynamic_cast<CCheckBoxUI*>(m_pm.FindControl("chkMonitorAutoRun"));
	chkAgree = dynamic_cast<CCheckBoxUI*>(m_pm.FindControl("chkAgree"));
	chkAgree->OnNotify += MakeDelegate(this, &InstallWindow::ChkAgreeCheckBoxEvent);

	auto info = dynamic_cast<CLabelUI*>(m_pm.FindControl("instinfo"));
	info->SetText(format("{} 上位机软件", SOFT_NAME).c_str());

	auto welcomeInfo = dynamic_cast<CLabelUI*>(m_pm.FindControl("welcomeinfo"));
	welcomeInfo->SetText(format("{} 安装完成",SOFT_NAME).c_str());

	auto title = dynamic_cast<CLabelUI*>(m_pm.FindControl("title"));
	title->SetText(format("{}安装程序 - v{}", SOFT_NAME, SOFT_VERSION).c_str());

	auto runBtn = dynamic_cast<CButtonUI*>(m_pm.FindControl("runBtn"));
	runBtn->OnNotify += MakeDelegate(this, &InstallWindow::RunAndClose);
}

bool InstallWindow::ChkAgreeCheckBoxEvent(void* param)
{
	TNotifyUI* msg = reinterpret_cast<TNotifyUI*>(param);
	if (msg->sType == DUI_MSGTYPE_SELECTCHANGED)
	{
		if (!chkAgree->IsSelected())
		{
			installBtn1->SetEnabled(false);
			installBtn2->SetEnabled(false);
		}
		else
		{
			installBtn1->SetEnabled(true);
			installBtn2->SetEnabled(true);
		}
	}
	return true;
}

bool InstallWindow::IsSoftwareInstalled(string& uninstallString)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ShallowSea",
		0, KEY_READ, &hKey) == ERROR_SUCCESS) {

		char buffer[512];
		DWORD bufferSize = sizeof(buffer);
		if (RegQueryValueEx(hKey, "UninstallString", nullptr, nullptr, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)
		{
			uninstallString = buffer;
			RegCloseKey(hKey);
			return true;
		}

		RegCloseKey(hKey);
	}
	return false;
}

bool InstallWindow::Installed()
{
	string uninstallString;
	if (IsSoftwareInstalled(uninstallString))
	{
		TCHAR wszTitle[MAX_PATH] = { 0 };
		GetWindowText(m_hWnd, wszTitle, MAX_PATH);
		if (MessageBox(m_hWnd,
			format("{} 已安装。\n点击 [是] 启动卸载程序，点击 [否] 取消当前安装！", SOFT_NAME).c_str(),
			wszTitle, MB_YESNO | MB_ICONINFORMATION) != IDYES) return false;

		if (ShellExecute(NULL, "open", uninstallString.c_str(), NULL, NULL, SW_SHOWNORMAL) <= (HINSTANCE)32)
		{
			DWORD errorCode = GetLastError();
			return MessageBox(m_hWnd, format("启动卸载程序失败。\n错误信息: {}\n是否继续安装?", errorCode, uninstallString).c_str(), wszTitle, MB_YESNO | MB_ICONERROR) == IDYES;
		}
		return false;
	}
	return true;
}


bool InstallWindow::OnExecuteInstall(void* param)
{
	if (!IsClick(param)) return true;

	if (IsXP())
	{
		TCHAR wszTitle[MAX_PATH] = { 0 };
		GetWindowText(m_hWnd, wszTitle, MAX_PATH);
		MessageBox(m_hWnd, "不支持在 Windows 7 及更早版本的操作系统上运行", wszTitle, MB_OK | MB_ICONERROR);
		return true;
	}

	if(!Installed()) return true;
	installRoot = path(installDirEdit->GetText().GetData());
	if (!IsValidInstallPath(installRoot.string(), m_hWnd)) {
		return true;
	}

	string softName = SOFT_NAME;
	string softVersion = SOFT_VERSION;
	if (installRoot.filename() != softName) 
	{
		installRoot.append(softName);
	}
	installRoot.append(softName + softVersion);

	bool result = CreateMultipleDirectory(m_hWnd, installRoot.string());
	if (!result) return true;
	
	thread([&]() 
		{
			SetCloseBtnEnabled(false);
			InstallHandler handler(installRoot.string(), this);
			appPath = handler.ExecuteInstall();
			SetCloseBtnEnabled(true);
		}).detach();
	return true;
}
