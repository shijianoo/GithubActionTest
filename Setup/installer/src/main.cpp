#include <iostream>
#include <fstream>
#include "config.h"
#include "cmdline.h"
#include "InstallWindow.h"
#include "InstallHandler.h"
#include "Helper.h"

using namespace cmdline;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	int argc;
	LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
	char** argv = new char* [argc];
	for (int i = 0; i < argc; ++i)
	{
		int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
		if (size > 0)
		{
			argv[i] = new char[size];
			int result = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argv[i], size, nullptr, nullptr);
		}
	}
	parser cmdParser;
	cmdParser.add<string>("del", 'd', "Delete Self", false, "false", cmdline::oneof<string>("true", "false"));
	cmdParser.add<string>("path", 'p', "Release path", false);
	cmdParser.parse_check(argc, argv);
	string isDelSelf = cmdParser.get<string>("del");
	string path = cmdParser.get<string>("path");

	if (!path.empty())
	{
		if (!IsDirExist(path)) return 1;
		InstallHandler install(path, NULL);
		install.InstallApplication();
	}
	else
	{
		HANDLE Hmutex = CreateMutex(0, true, (string(SOFT_NAME) + "install").c_str());
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			MessageBox(NULL, "已有安装程序正在运行，请稍后再试！", (string(SOFT_NAME) + "安装程序").c_str(), MB_OK | MB_ICONINFORMATION);
			return 0;
		}
		InstallWindow window;
		window.Show();
	}

	if (isDelSelf == "true")
	{
		DeleteApplicationSelfWithBat();
	}
	return 0;
}