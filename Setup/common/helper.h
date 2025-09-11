#pragma once

#include <windows.h>
#include <string>

using namespace std;

wstring ConvertToWString(const string& str);

bool IsXP();

string ShowFolderDialog(HWND hwndOwner);

bool CreateMultipleDirectory(HWND hWnd, const string& directory);

bool LoadResourceData(DWORD resId, const string& resType, LPVOID& pRes, DWORD& dwResSize);

bool FreeResFile(DWORD redId, const string& resType, const string& fileName);

DWORD GetDirectorySize(const string& directory);

void CreateShortcut(const string& shortcutPath, const string& targetPath, const string& desc);

int KillProcess(string processName);

void RegisterFileRelation(string strExt, string strAppName, string strAppKey, string strDefaultIcon, string strDescribe);

void NotifyShell(string path);

string GetErrorMessage(DWORD errorCode);

bool IsFileExist(const string& csFile);

bool IsDirExist(const string& csDir);

wstring StringToWstring(const string& str);

bool KillProcessByExactName(const std::string& processName);

bool DeleteApplicationSelfWithBat();

bool IsValidInstallPath(const std::string& path, HWND hWnd = NULL);
