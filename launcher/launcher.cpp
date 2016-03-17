// launcher.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <Windows.h>
#include <shellapi.h>

int _tmain(int argc, _TCHAR* argv[])
{
	SHELLEXECUTEINFO ShExecInfo;
	DWORD dwLastError;

	TCHAR szTemp[MAX_PATH];

	if (GetCurrentDirectory(MAX_PATH, szTemp) == 0)
	{
		printf("GetCurrentDirectory GLE = 0x%08X\n", GetLastError());
		getchar();
		return -1;
	}

	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI;
	ShExecInfo.hwnd = NULL; // LaTale Launcher - (#32770)(Invisible)
	ShExecInfo.lpVerb = TEXT("Open");
	ShExecInfo.lpFile = TEXT("LaTaleClient.exe");;
	ShExecInfo.lpParameters = TEXT("/Full /LoginServer:157.7.172.197:10000 /Run:VHIGH /LOCALE:JAPAN /PUBLISHER:GAMEPOT");
	ShExecInfo.lpDirectory = szTemp;
	ShExecInfo.nShow = SW_NORMAL;
	ShExecInfo.hInstApp = NULL;

	if (!ShellExecuteEx(&ShExecInfo))
	{
		dwLastError = GetLastError();

		if (dwLastError == ERROR_FILE_NOT_FOUND)
		{
			printf("ERROR_FILE_NOT_FOUND\n");
		}
		else if (dwLastError == ERROR_PATH_NOT_FOUND)
		{
			printf("ERROR_PATH_NOT_FOUND\n");
		}
		else if (dwLastError == ERROR_ACCESS_DENIED)
		{
			printf("ERROR_ACCESS_DENIED\n");
		}
		else
		{
			printf("ShellExecuteEx GLE = 0x%08X\n", dwLastError);
		}
		getchar();
	}
	else
	{
		printf("SUCCESS\n");
	}

	return 0;
}

