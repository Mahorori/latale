#include "stdafx.h"

//#define BOOST_ALL_DYN_LINK

#include "XPI.hpp"

#include <Psapi.h>

#include "FindPattern.h"
#include "Hooks.h"
#include "CLog.hpp"
#include "CInstanceManager.hpp"
#include "CHookManager.hpp"
#include "CMainWindow.hpp"
#include "CResourceString.hpp"
#include "XPIUtilities.hpp"
#include "XPIConfig.h"
#include "extvars.hpp"
#include "resource.h"

#pragma  comment(lib, "psapi")

PIMAGE_NT_HEADERS _ImageNtHeader_sub(_In_ PVOID Base)
{
	PIMAGE_DOS_HEADER pImageDosHeader;
	PIMAGE_NT_HEADERS pImageNtHeaders;
	LONG e_lfanew;

	pImageDosHeader = PIMAGE_DOS_HEADER(Base);

	if (pImageDosHeader != NULL && pImageDosHeader != INVALID_HANDLE_VALUE)
	{
		if (pImageDosHeader->e_magic == IMAGE_DOS_SIGNATURE)
		{
			e_lfanew = pImageDosHeader->e_lfanew;

			if (e_lfanew >= 0 && e_lfanew < 0x10000000)
			{
				pImageNtHeaders = PIMAGE_NT_HEADERS(reinterpret_cast<PBYTE>(pImageDosHeader)+e_lfanew);
				if (pImageNtHeaders->Signature == IMAGE_NT_SIGNATURE)
					return pImageNtHeaders;
			}
		}
	}

	return NULL;
}

PIMAGE_NT_HEADERS WINAPI _ImageNtHeader(_In_ PVOID Base)
{
	PIMAGE_NT_HEADERS pImageNtHeaders = _ImageNtHeader_sub(Base);

	if (pImageNtHeaders == NULL)
		SetLastError(ERROR_BAD_EXE_FORMAT);

	return pImageNtHeaders;
}

_Success_(return) BOOL GetModuleSize(__in HMODULE hModule, __out LPVOID* lplpBase, __out LPDWORD lpdwSize)
{
	if (hModule == GetModuleHandle(NULL))
	{
		PIMAGE_NT_HEADERS pImageNtHeaders = _ImageNtHeader((PVOID)hModule);

		if (pImageNtHeaders == NULL)
			return FALSE;

		*lplpBase = (LPVOID)hModule;
		*lpdwSize = pImageNtHeaders->OptionalHeader.SizeOfImage;
	}
	else
	{
		MODULEINFO ModuleInfo;

		if (!GetModuleInformation(GetCurrentProcess(), hModule, &ModuleInfo, sizeof(MODULEINFO)))
			return FALSE;

		*lplpBase = ModuleInfo.lpBaseOfDll;
		*lpdwSize = ModuleInfo.SizeOfImage;
	}

	return TRUE;
}

BOOL XPIAPI LocateFunctionSignatures(__in PVOID pBase, __in DWORD dwSize)
{
#ifdef _DEBUG
	pLog->Write(L"Searching for function signatures...");
#endif

	foreach(const MAPLE_HOOK& i, MapleHooks)
	{
		PVOID pTarget = FindPatternW(pBase, dwSize, i.Function.lpcwszSignature);

		if (pTarget != NULL)
		{
			if (i.Function.OffsetType == OFF_ADD)
				pTarget = (PVOID)((ULONG_PTR)pTarget + i.Function.uOffset);
			else if (i.Function.OffsetType == OFF_SUB)
				pTarget = (PVOID)((ULONG_PTR)pTarget - i.Function.uOffset);
			else if (i.Function.OffsetType == OFF_CALL)
				pTarget = (PVOID)((ULONG_PTR)pTarget + i.Function.uOffset + *(ULONG_PTR*)((ULONG_PTR)pTarget + i.Function.uOffset + 1) + 5);
#ifdef _DEBUG
			pLog->Write(LOG_WF_DEBUG, L"%s = 0x%p.", i.Function.lpcwszName, pTarget);
#endif
			*(PVOID*)i.Function.pTarget = pTarget;
		}
		else
		{
#ifdef _DEBUG
			pLog->Write(LOG_WF_ERROR, L"Couldn't find %s, aborting!", i.Function.lpcwszName);
#endif
			return FALSE;
		}
	}

	return TRUE;
}

DWORD XPIAPI InitializeXPI(__in HINSTANCE hInstance)
{
#ifdef _DEBUG
	pLog = new CLog(L"XPI");
	pLog->Write(L"Initializing...");
#endif

	if (!GetModuleSize(GetModuleHandle(NULL), &pMapleBase, &dwMapleSize))
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't get target module base & size, aborting!");
#endif
		FreeLibraryAndExitThread(hInstance, EXIT_FAILURE);
	}

#ifdef _DEBUG
	pLog->Write(LOG_WF_DEBUG, L"Target module base = 0x%p.", pMapleBase);
	pLog->Write(LOG_WF_DEBUG, L"Target module size = 0x%08X.", dwMapleSize);
#endif

	if (!LocateFunctionSignatures(pMapleBase, dwMapleSize))
		FreeLibraryAndExitThread(hInstance, EXIT_FAILURE);

	pInstances = new CInstanceManager;
	pHookManager = new CHookManager;

	foreach(const MAPLE_HOOK& i, MapleHooks)
		pHookManager->Add((PVOID*)i.Function.pTarget, i.pHook);

#ifdef _DEBUG
	pLog->Write(LOG_WF_DEBUG, L"Installing hooks.");
#endif

	if (pHookManager->Install())
	{
#ifdef _DEBUG
		pLog->Write(L"Hooks installed & initialized correctly!");
#endif

		pStrings = new CResourceString(hInstance);
		pPacketPool = new boost::object_pool<CMaplePacket>;
		pOpcodeInfo = new OPCODE_MAP();

		if (!LoadXPIConfig(XPI_CONFIG_FILE))
		{
#ifdef _DEBUG
			pLog->Write(L"XPI configuration file not found.");
#endif
		}

		DisableThreadLibraryCalls(hInstance);

		DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_XPI), NULL, CMainWindow::DialogProc, (LPARAM)hInstance);
		if (GetLastError() == ERROR_SUCCESS)
		{
#ifdef _DEBUG
			pLog->Write(L"Saving XPI configuration...");
#endif

			if (!SaveXPIConfig(XPI_CONFIG_FILE))
			{
#ifdef _DEBUG
				pLog->Write(L"Error writing configuration file!");
#endif
			}

			FreeLibraryAndExitThread(hInstance, EXIT_SUCCESS);
		}
		else
			FreeLibraryAndExitThread(hInstance, EXIT_FAILURE);
	}
	else
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR | LOG_WF_ECHODEBUG, L"Couldn't install hooks, aborting!");
#endif

		FreeLibraryAndExitThread(hInstance, EXIT_FAILURE);
	}
}

VOID XPIAPI DestroyXPI()
{
#ifdef _DEBUG
	if (pLog != NULL)
	{
		pLog->Write(L"Destroying XPI...");
		delete pLog;
	}
#endif

	if (pHookManager != NULL)
		delete pHookManager;

	if (pInstances != NULL)
		delete pInstances;

	if (pStrings != NULL)
		delete pStrings;

	if (pPacketPool != NULL)
		delete pPacketPool;

	if (pOpcodeInfo != NULL)
		delete pOpcodeInfo;
}

BOOL APIENTRY DllMain(__in HINSTANCE hInstance, __in DWORD fdwReason, __reserved LPVOID lpReserved)
{
	HANDLE hThread;

	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitializeXPI, (LPVOID)hInstance, 0, NULL);
			break;
		}

		case DLL_PROCESS_DETACH:
		{
			DestroyXPI();
			break;
		}
	}

	return TRUE;
}
