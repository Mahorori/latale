// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

#include <string>

VOID WINAPI Initialize(__in HINSTANCE hInstance)
{
	DisableThreadLibraryCalls(hInstance);

	struct std__string__baseclass
	{
		LPVOID lpv;
		std::string string;
	};

	typedef std__string__baseclass*(__thiscall * pfnSPListItemBase__GetOptionText)(__in LPVOID *lpECX, __in WORD wOption);
	pfnSPListItemBase__GetOptionText SPListItemBase__GetOptionText = reinterpret_cast<pfnSPListItemBase__GetOptionText>(0x00667FE0);

	LPVOID **SPListItemBase__msInstance = reinterpret_cast<LPVOID**>(0x0087F80C);

	HANDLE hFile = CreateFile(TEXT("option.txt"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD dwNumberOfBytesWritten;
	CHAR szTemp[32];

	if (hFile == INVALID_HANDLE_VALUE)
		FreeLibraryAndExitThread(hInstance, 0);

	for (unsigned short i = 0; i < 32767; i++)
	{
		std__string__baseclass *p = SPListItemBase__GetOptionText(*SPListItemBase__msInstance, i);
		if (p)
		{
			// printf(p->string.c_str(), 0);
			sprintf_s(szTemp, "%d ", i);
			
			WriteFile(hFile, szTemp, strlen(szTemp), &dwNumberOfBytesWritten, NULL);
			WriteFile(hFile, p->string.c_str(), strlen(p->string.c_str()), &dwNumberOfBytesWritten, NULL);
			WriteFile(hFile, "\r\n", 2, &dwNumberOfBytesWritten, NULL);
		}
	}

	CloseHandle(hFile);

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