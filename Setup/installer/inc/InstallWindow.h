#pragma once

#include <iostream>
#include <thread>
#include <filesystem>
#include "Window.h"
#include "helper.h"
#include "config.h"

using namespace std::filesystem;

#define HOMEPAGEINDEX 0										//主面的Index
#define INSTALLINGPAGEINDEX 1								//安装中页面的Index
#define CONFIGPAGEINDEX 2									//配置页面的Index
#define COMPLETEDPAGEINDEX 3								//安装完成页面的Index

class InstallWindow : public Window {
	void InitWindow() override;

public:
	inline void Show()
	{
		CPaintManagerUI::SetInstance(GetModuleHandle(NULL));
		Window::Show(format("{} 安装程序", SOFT_NAME));
	}

	inline void SetProgress(const int percent)
	{
		progressBar->SetValue(percent);
		string percentStr = to_string(percent) + "%";
		progressPercent->SetText(percentStr.c_str());
	}

	inline void SetProgressState(const string& text)
	{
		progressState->SetText(text.c_str());
	}

	inline bool IsCreateShortCut() 
	{
		return chkShortCut->GetCheck();
	}

	inline bool IsAutoRunMonitor()
	{
		return chkMonitorAutoRun->GetCheck();
	}
private:
	CRichEditUI* installDirEdit;
	CButtonUI* installBtn1;
	CButtonUI* installBtn2;
	CCheckBoxUI* chkAgree;

	CSliderUI* progressBar;
	CLabelUI* progressState;
	CLabelUI* progressPercent;

	CCheckBoxUI* chkShortCut;
	CCheckBoxUI* chkMonitorAutoRun;
	
	path installRoot;
	string appPath;
private:

	inline bool OpenOption(void* param)
	{
		if (!IsClick(param)) return false;
		ToPage(CONFIGPAGEINDEX);
		return true;
	}

	inline bool CloseOption(void* param)
	{
		if (!IsClick(param)) return false;
		ToPage(HOMEPAGEINDEX);
		return true;
	}

	inline bool RunAndClose(void* param)
	{
		if (!IsClick(param)) return false;
		ShellExecute(NULL, "open", appPath.c_str(), NULL, NULL, SW_SHOW);
		Close();
		return true;
	}

	inline bool InstallDirSelect(void* param)
	{
		if (!IsClick(param)) return false;
		auto dir = ShowFolderDialog(GetHWND());
		installDirEdit->SetText(dir.c_str());
	}

	bool OnExecuteInstall(void* param);
	bool ChkAgreeCheckBoxEvent(void* param);

	bool IsSoftwareInstalled(string& uninstallString);
	bool Installed();
};