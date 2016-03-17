#include "stdafx.h"

#include "dinput8.h"
#include "latale.h"
#include "utils.h"
#include "xigncode.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

BOOL WINAPI DLLLoader()
{
	TCHAR szTemp[MAX_PATH];

	HANDLE hFile;
	WIN32_FIND_DATA fData;

	if (GetCurrentDirectory(MAX_PATH, szTemp) == 0)
		return FALSE;

	if (!SUCCEEDED(StringCchPrintf(szTemp, MAX_PATH, TEXT("%s%s"), szTemp, TEXT("\\Inject\\*.dll"))))
		return FALSE;

	hFile = FindFirstFile(szTemp, &fData);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	do
	{
		if ((fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_tcscmp(PathFindExtension(fData.cFileName), TEXT(".dll")) == 0)
			{
				if (SUCCEEDED(StringCchPrintf(szTemp, MAX_PATH, TEXT("%s%s"), TEXT("Inject\\"), fData.cFileName)))
					LoadLibrary(szTemp);
			}
		}
	} while (FindNextFile(hFile, &fData));

	return TRUE;
}

BOOL WINAPI OnAttachProcess(__in HINSTANCE hInstance)
{
	if (dinput8::Init())
	{
		// hwid
		Detour__CreateFileW();
		Detour__GetAdaptersInfo();
		Detour_o_o();

		xigncode_bypass();

		DLLLoader();
	}

	DisableThreadLibraryCalls(hInstance);

	return TRUE;
}

BOOL WINAPI DllMain(
	__in HINSTANCE hInstance,
	__in DWORD fdwReason,
	__reserved LPVOID lpvReserved
	)
{
	UNREFERENCED_PARAMETER(lpvReserved);

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			return OnAttachProcess(hInstance);
		}
		case DLL_PROCESS_DETACH:
		{
		}
	}

	return TRUE;
}