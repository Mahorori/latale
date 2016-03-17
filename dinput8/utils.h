#pragma once

VOID WriteBypassCode(LPVOID* lppvFunction, LPVOID lpvRedirection);

BOOL WriteMemory(__in LPVOID lpvTarget, __in CONST BYTE lpbMemory[], __in INT iSize);

BOOL DetourFunction(BOOL fStatus, LPVOID* lppvFunction, LPVOID lpvRedirection);

VOID WINAPI Error(__in LPCWSTR lpcwszText);

VOID WINAPI Win32Error();

PIMAGE_NT_HEADERS WINAPI _ImageNtHeader(_In_ PVOID Base);