// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

#include <qapplication.h>

#include "autologin.h"
#include "CMainWindow.hpp"

#ifdef _DEBUG
#error "pls build rls ver :'(";
#endif

VOID WINAPI Initialize(__in HINSTANCE hInstance)
{
	DisableThreadLibraryCalls(hInstance);

	int argc = 0;
	QApplication *pApp = new QApplication(argc, NULL);
	CMainWindow *pMainWindow = new CMainWindow;
	pMainWindow->show();
	pApp->exec();

	delete pMainWindow;
	delete pApp;

	FreeLibraryAndExitThread(hInstance, 0);
}

VOID WINAPI Finallize()
{
}

BOOL APIENTRY DllMain(__in HINSTANCE hInstance, __in DWORD fdwReason, __in __reserved LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Initialize, (LPVOID)hInstance, 0, NULL);
			if (hThread != NULL)
				break;
			else
				return FALSE;
		}
		case DLL_PROCESS_DETACH:
		{
			Finallize();
			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}