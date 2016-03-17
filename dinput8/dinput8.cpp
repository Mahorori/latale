#include "stdafx.h"

#include "dinput8.h"
#include "utils.h"
#include <ShlObj.h>

namespace dinput8
{
	pfnDirectInput8Create _DirectInput8Create;
	pfnDllCanUnloadNow _DllCanUnloadNow;
	pfnDllGetClassObject _DllGetClassObject;
	pfnDllRegisterServer _DllRegisterServer;
	pfnDllUnregisterServer _DllUnregisterServer;

	HRESULT WINAPI DirectInput8Create(HINSTANCE hinst,
		DWORD dwVersion,
		REFIID riidltf,
		LPVOID *ppvOut,
		LPUNKNOWN punkOuter)
	{
		return _DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
	}

	HRESULT WINAPI DllCanUnloadNow()
	{
		return _DllCanUnloadNow();
	}

	HRESULT WINAPI DllGetClassObject(
		const CLSID & rclsid,
		const IID & riid,
		void ** ppv)
	{
		return _DllGetClassObject(rclsid, riid, ppv);
	}

	HRESULT WINAPI DllRegisterServer()
	{
		return _DllRegisterServer();
	}

	HRESULT WINAPI DllUnregisterServer()
	{
		return _DllUnregisterServer();
	}

	BOOL Detour__FindFileA()
	{
		static decltype(&FindFirstFileA) _FindFirstFileA = FindFirstFileA;
		static decltype(&FindNextFileA) _FindNextFileA = FindNextFileA;

		decltype(&FindFirstFileA) FindFirstFileA_Hook = [](
			_In_ LPCSTR lpFileName,
			_Out_ LPWIN32_FIND_DATAA lpFindFileData) -> HANDLE
		{
			HANDLE hFind = _FindFirstFileA(lpFileName, lpFindFileData);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				if (strstr(lpFindFileData->cFileName, "dinput8.dll") != NULL)
				{
					FindNextFileA(hFind, lpFindFileData);
					return hFind;
				}

				return hFind;
			}

			return INVALID_HANDLE_VALUE;
		};

		decltype(&FindNextFileA) FindNextFileA_Hook = [](
			_In_ HANDLE hFindFile,
			_Out_ LPWIN32_FIND_DATAA lpFindFileData) -> BOOL
		{
			if (_FindNextFileA(hFindFile, lpFindFileData))
			{
				if (strstr(lpFindFileData->cFileName, "dinput8.dll") != NULL)
					return _FindNextFileA(hFindFile, lpFindFileData);

				return TRUE;
			}

			return FALSE;
		};

		if (!DetourFunction(TRUE, reinterpret_cast<void**>(&_FindNextFileA), FindNextFileA_Hook))
			return FALSE;

		if (!DetourFunction(TRUE, reinterpret_cast<void**>(&_FindFirstFileA), FindFirstFileA_Hook))
		{
			DetourFunction(FALSE, reinterpret_cast<void**>(&_FindNextFileA), FindNextFileA_Hook);
			return FALSE;
		}

		return TRUE;
	}

	BOOL Init()
	{
		TCHAR szPath[MAX_PATH];

		if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, 0, szPath)))
			return FALSE;

		if (!SUCCEEDED(StringCchPrintf(szPath, MAX_PATH, TEXT("%s%s"), szPath, TEXT("\\dinput8.dll"))))
			return FALSE;

		HMODULE hModule = LoadLibrary(szPath);

		if (!hModule)
			return FALSE;

		_DirectInput8Create = reinterpret_cast<pfnDirectInput8Create>(GetProcAddress(hModule, "DirectInput8Create"));
		_DllCanUnloadNow = reinterpret_cast<pfnDllCanUnloadNow>(GetProcAddress(hModule, "DllCanUnloadNow"));
		_DllGetClassObject = reinterpret_cast<pfnDllGetClassObject>(GetProcAddress(hModule, "DllGetClassObject"));
		_DllRegisterServer = reinterpret_cast<pfnDllRegisterServer>(GetProcAddress(hModule, "DllRegisterServer"));
		_DllUnregisterServer = reinterpret_cast<pfnDllUnregisterServer>(GetProcAddress(hModule, "DllUnregisterServer"));

		return Detour__FindFileA();
	}
}