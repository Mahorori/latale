#include "stdafx.h"

#include "utils.h"

#include <detours.h>
#pragma comment( lib, "detours.lib" )

BOOL DetourFunction(BOOL fStatus, LPVOID* lppvFunction, LPVOID lpvRedirection)
{
	if (DetourTransactionBegin() != NO_ERROR)
		return FALSE;

	if (DetourUpdateThread(GetCurrentThread()) == NO_ERROR)
		if ((fStatus ? DetourAttach : DetourDetach)(lppvFunction, lpvRedirection) == NO_ERROR)
			if (DetourTransactionCommit() == NO_ERROR)
				return TRUE;

	DetourTransactionAbort();
	return FALSE;
}

BOOL WriteJump(__in LPVOID lpvFunction, __in LPVOID lpvRedirection)
{
	if (lpvFunction == NULL || lpvRedirection == NULL)
		return FALSE;

	MEMORY_BASIC_INFORMATION mbi;

	if (VirtualQuery(lpvFunction, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) != sizeof(MEMORY_BASIC_INFORMATION))
		return FALSE;

	if (!mbi.Protect || (mbi.Protect & PAGE_GUARD))
		return FALSE;

	PBYTE pbCode = (PBYTE)lpvFunction;
	PBYTE pbJmpVal = (PBYTE)lpvRedirection;
	PBYTE pbJmpSrc = pbCode + 5;

	PBYTE pbTarget = (PBYTE)lpvFunction;
	PBYTE pbSrc = pbTarget;
	LONG lExtra = 0;

	if (mbi.Protect & PAGE_EXECUTE_READWRITE)
	{
		while (pbSrc < pbTarget + 5)
			pbSrc = (PBYTE)DetourCopyInstruction(NULL, NULL, pbSrc, NULL, &lExtra);

		*pbCode++ = 0xE9; // jmp rel32
		*((INT32*&)pbCode) = (INT32)(pbJmpVal - pbJmpSrc);

		memset(pbJmpSrc, 0xCC, pbSrc - pbJmpSrc);

		return TRUE;
	}
	else
	{
		DWORD flOldProtect;

		if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &flOldProtect))
			return FALSE;

		while (pbSrc < pbTarget + 5)
			pbSrc = (PBYTE)DetourCopyInstruction(NULL, NULL, pbSrc, NULL, &lExtra);

		*pbCode++ = 0xE9; // jmp rel32
		*((INT32*&)pbCode) = (INT32)(pbJmpVal - pbJmpSrc);

		memset(pbJmpSrc, 0xCC, pbSrc - pbJmpSrc);

		return VirtualProtect(mbi.BaseAddress, mbi.RegionSize, flOldProtect, &flOldProtect);
	}
}

VOID WriteBypassCode(LPVOID* lppvFunction, LPVOID lpvRedirection)
{
	PBYTE pbTarget = (PBYTE)*lppvFunction;
	PBYTE pbSrc = pbTarget;
	PBYTE pbDest = (unsigned char*)malloc(30); // where's delete..
	PBYTE pbDestPool = pbDest + sizeof(pbDest);

	LONG lExtra = 0;

	//
	*lppvFunction = pbDest;

	// 1st opcode
	pbSrc = (PBYTE)DetourCopyInstruction(NULL, NULL, pbSrc, NULL, &lExtra);

	while (pbSrc < pbTarget + 5)
	{
		PBYTE pbOp = pbSrc;
		pbSrc = (PBYTE)DetourCopyInstruction(pbDest, (PVOID*)&pbDestPool, pbSrc, NULL, &lExtra);
		pbDest += (pbSrc - pbOp) + lExtra;
	}

	WriteJump(pbDest, pbSrc);

	WriteJump(pbTarget, lpvRedirection);
}

BOOL WriteMemory(__in LPVOID lpvTarget, __in CONST BYTE lpbMemory[], __in INT iSize)
{
	MEMORY_BASIC_INFORMATION mbi;

	if (VirtualQuery(lpvTarget, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) != sizeof(MEMORY_BASIC_INFORMATION))
		return FALSE;

	if (!mbi.Protect || (mbi.Protect & PAGE_GUARD))
		return FALSE;

	if (mbi.Protect & PAGE_EXECUTE_READWRITE)
	{
		memcpy(lpvTarget, lpbMemory, iSize);

		return TRUE;
	}
	else
	{
		DWORD flOldProtect;

		if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &flOldProtect))
			return FALSE;

		memcpy(lpvTarget, lpbMemory, iSize);

		return VirtualProtect(mbi.BaseAddress, mbi.RegionSize, flOldProtect, &flOldProtect);
	}
}

VOID WINAPI Win32Error()
{
	DWORD dwMessageId = ::GetLastError();

	if (dwMessageId != NO_ERROR) {
		LPVOID lpvBuffer = NULL;

		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpvBuffer, 0, NULL) == 0) {
			MessageBox(NULL, L"An error has occured as a result of system function call.", L"JMS VIP Bypass",
				MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND);
		}
		else {
			MessageBox(NULL, (LPWSTR)lpvBuffer, L"JMS VIP Bypass",
				MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND);
		}

		if (lpvBuffer != NULL)
			::LocalFree((::HLOCAL)lpvBuffer);
	}
}

VOID WINAPI Error(__in LPCWSTR lpcwszText)
{
	MessageBox(NULL, lpcwszText, L"JMS Bypass", MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND);
}

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