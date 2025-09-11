#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "icoextra.h"

typedef union _MEMICONITEM {
	UCHAR Buffer[FIELD_OFFSET(MEMICONDIRENTRY, wID) + sizeof(DWORD)];
	MEMICONDIRENTRY Entry;
} MEMICONITEM;

BOOL CALLBACK ExtractIconResource(DWORD resId, LPCTSTR pszFile)
{
	HMODULE hModule = ::GetModuleHandle(NULL);
	BOOL bResult = FALSE;

	HRSRC hResInfo = FindResource(hModule, MAKEINTRESOURCE(resId), RT_GROUP_ICON);
	if (hResInfo != nullptr)
	{
		DWORD dwResSize = SizeofResource(hModule, hResInfo);
		if (dwResSize > 0)
		{
			HANDLE hResData = LoadResource(hModule, hResInfo);
			if (hResData != nullptr)
			{
				LPVOID pvResData = LockResource(hResData);
				if (pvResData != nullptr)
				{
					MEMICONDIR *pHead = static_cast<MEMICONDIR *>(pvResData);
					MEMICONITEM *pItems = new MEMICONITEM[static_cast<size_t>(pHead->idCount)];
					DWORD cbItems = pHead->idCount * sizeof(MEMICONITEM);
					HANDLE hFile = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE | DELETE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						LARGE_INTEGER liOffset;
						liOffset.QuadPart = sizeof(MEMICONDIR) + static_cast<LONGLONG>(cbItems);
						bResult = SetFilePointerEx(hFile, liOffset, nullptr, FILE_BEGIN);
						if (bResult)
						{
							DWORD dwActual;
							for (size_t i = 0; i < pHead->idCount; i++)
							{
								hResInfo = FindResource(hModule, MAKEINTRESOURCE(pHead->idEntries[i].wID), RT_ICON);
								if (!hResInfo)
								{
									bResult = FALSE;
									goto CleanUp;
								}

								dwResSize = SizeofResource(hModule, hResInfo);
								if (!dwResSize)
								{
									bResult = FALSE;
									goto CleanUp;
								}

								hResData = LoadResource(hModule, hResInfo);
								if (!hResData)
								{
									bResult = FALSE;
									goto CleanUp;
								}

								pvResData = LockResource(hResData);
								if (!pvResData)
								{
									bResult = FALSE;
									goto CleanUp;
								}

								bResult = WriteFile(hFile, pvResData, dwResSize, &dwActual, nullptr);
								if (!bResult)
								{
									goto CleanUp;
								}

								memcpy(&pItems[i].Entry, &pHead->idEntries[i], FIELD_OFFSET(MEMICONDIRENTRY, wID));
								memcpy(&pItems[i].Entry.wID, &liOffset.LowPart, sizeof(DWORD));
								liOffset.QuadPart += dwResSize;
							}

							bResult = SetFilePointerEx(hFile, { 0LL }, nullptr, FILE_BEGIN);
							if (!bResult)
							{
								goto CleanUp;
							}

							bResult = WriteFile(hFile, pHead, sizeof(MEMICONDIR), &dwActual, nullptr);
							if (!bResult)
							{
								goto CleanUp;
							}

							bResult = WriteFile(hFile, pItems, cbItems, &dwActual, nullptr);
							if (!bResult)
							{
								goto CleanUp;
							}
						}

					CleanUp:
						//
						// 当上面任何一个函数调用失败时，这段代码将图标文件设置为关闭后自动删除。
						//
						// 如果无需考虑调用失败时删除文件的情况，删除文件创建权限中的 DELETE 权限。
						// 即 GENERIC_READ | GENERIC_WRITE | DELETE 改为 GENERIC_READ | GENERIC_WRITE
						//
						// 如果无需考虑调用失败时删除文件的情况，注释掉下面三行代码。
						//
						FILE_DISPOSITION_INFO fdi;
						fdi.DeleteFile = !bResult;
						SetFileInformationByHandle(hFile, FileDispositionInfo, &fdi, sizeof(fdi));

						//
						// 当开启上面三行代码且 bResult 为 FALSE 时，图标文件关闭后会自动删除。
						// 关闭图标文件。
						//
						CloseHandle(hFile);
					}

					delete[] pItems;
				}
			}
		}
	}

	return bResult;
}
