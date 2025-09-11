#include "window.h"
#include "helper.h"
#include "FileCollector.h"
#include "FileDeleter.h"
#include <iostream>
#include <format>

#define HOMEPAGEINDEX 0										//�����Index
#define UNINSTALLINGPAGEINDEX 1								//ж����ҳ���Index
#define COMPLETEDPAGEINDEX 2								//ж�����ҳ���Index

class UnInstallWindow : public Window {
	void InitWindow() override;

public:
	inline void Show()
	{
		CPaintManagerUI::SetInstance(GetModuleHandle(NULL));
		Window::Show(format("{} ж�س���", SOFT_NAME));
	}

private:
	CButtonUI* unInstallBtn;
	CButtonUI* cancelBtn;

	CSliderUI* progressBar;
	CLabelUI* progressState;
	CLabelUI* progressPercent;

	// �ļ�ɾ����
	FileCollector m_fileCollector;
	FileDeleter m_fileDeleter;

private:
	bool OnExecuteUnInstall(void* param);
	void OnUninstallProgress(int current, int total, const std::string& currentFile, bool success);
	void OnUninstallCompleted();
};