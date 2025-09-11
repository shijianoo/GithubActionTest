#include "window.h"
#include "helper.h"
#include "FileCollector.h"
#include "FileDeleter.h"
#include <iostream>
#include <format>

#define HOMEPAGEINDEX 0										//主面的Index
#define UNINSTALLINGPAGEINDEX 1								//卸载中页面的Index
#define COMPLETEDPAGEINDEX 2								//卸载完成页面的Index

class UnInstallWindow : public Window {
	void InitWindow() override;

public:
	inline void Show()
	{
		CPaintManagerUI::SetInstance(GetModuleHandle(NULL));
		Window::Show(format("{} 卸载程序", SOFT_NAME));
	}

private:
	CButtonUI* unInstallBtn;
	CButtonUI* cancelBtn;

	CSliderUI* progressBar;
	CLabelUI* progressState;
	CLabelUI* progressPercent;

	// 文件删除器
	FileCollector m_fileCollector;
	FileDeleter m_fileDeleter;

private:
	bool OnExecuteUnInstall(void* param);
	void OnUninstallProgress(int current, int total, const std::string& currentFile, bool success);
	void OnUninstallCompleted();
};