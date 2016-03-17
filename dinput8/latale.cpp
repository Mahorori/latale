#include "stdafx.h"

#include "latale.h"
#include "utils.h"

#include <IPHlpApi.h>
#pragma comment(lib, "IPHlpApi")

// HarddiskSerialNo
VOID Detour__CreateFileW()
{
	static decltype(&CreateFileW) _CreateFileW = CreateFileW;

	decltype(&CreateFileW) CreateFileW__Hook = [](
		_In_ LPCWSTR lpFileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
		) -> HANDLE
	{
		if (wcsstr(lpFileName, L"\\\\.\\PhysicalDrive") != NULL)
		{
			return INVALID_HANDLE_VALUE;
		}

		return _CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_CreateFileW), CreateFileW__Hook);
}

// MacAddress
VOID Detour__GetAdaptersInfo()
{
	static decltype(&GetAdaptersInfo) _GetAdaptersInfo = GetAdaptersInfo;

	decltype(&GetAdaptersInfo) GetAdaptersInfo__Hook = [](
		_Out_writes_bytes_opt_(*SizePointer) PIP_ADAPTER_INFO AdapterInfo,
		_Inout_                         PULONG           SizePointer
		) -> ULONG
	{
		if (AdapterInfo == NULL)
			return 0; // not ERROR_BUFFER_OVERFLOW

		return _GetAdaptersInfo(AdapterInfo, SizePointer);
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_GetAdaptersInfo), GetAdaptersInfo__Hook);
}

// 00705210 login packet

VOID Detour_o_o()
{
	static decltype(&VirtualAlloc) _VirtualAlloc = VirtualAlloc;

	decltype(&VirtualAlloc) VirtualAlloc__Hook = [](
		_In_opt_ LPVOID lpAddress,
		_In_ SIZE_T dwSize,
		_In_ DWORD flAllocationType,
		_In_ DWORD flProtect) -> LPVOID
	{
		if (dwSize == 0x84D00)
		{
			//MessageBoxA(NULL, "", "", MB_OK);
		}

		return _VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_VirtualAlloc), VirtualAlloc__Hook);
}