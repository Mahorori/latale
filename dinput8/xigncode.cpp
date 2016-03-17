#include "stdafx.h"

#include "xigncode.h"
#include "utils.h"

#include <Psapi.h>
#include <string>
#include <winternl.h>

#define XC3_LOG

BOOL fDenyAllProcess = FALSE;

VOID Detour__GetWindowThreadProcessId()
{
	static decltype(&GetWindowThreadProcessId) _GetWindowThreadProcessId = GetWindowThreadProcessId;

	decltype(&GetWindowThreadProcessId) GetWindowThreadProcessId__Hook = [](
		_In_ HWND hWnd,
		_Out_opt_ LPDWORD lpdwProcessId
		) -> DWORD
	{
		HMODULE hModule;

		if (lpdwProcessId)
		{
			if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<TCHAR*>(_ReturnAddress()), &hModule))
			{
				// 同プロセスのウィンドウは調べていない
				*lpdwProcessId = GetCurrentProcessId();

				return NULL;
			}
		}

		return _GetWindowThreadProcessId(hWnd, lpdwProcessId);
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_GetWindowThreadProcessId), GetWindowThreadProcessId__Hook);
}

VOID Detour__NtOpenProcess()
{
	typedef struct _CLIENT_ID
	{
		DWORD UniqueProcess;
		DWORD UniqueThread;
	} CLIENT_ID, *PCLIENT_ID;

	typedef NTSTATUS(NTAPI * pfnNtOpenProcess)(
		_Out_    PHANDLE            ProcessHandle,
		_In_     ACCESS_MASK        DesiredAccess,
		_In_     POBJECT_ATTRIBUTES ObjectAttributes,
		_In_opt_ PCLIENT_ID			ClientId);

	static pfnNtOpenProcess _NtOpenProcess = reinterpret_cast<pfnNtOpenProcess>(GetProcAddress(
		GetModuleHandle(TEXT("ntdll")), "NtOpenProcess"));

	pfnNtOpenProcess NtOpenProcess__Hook = [](
		_Out_    PHANDLE            ProcessHandle,
		_In_     ACCESS_MASK        DesiredAccess,
		_In_     POBJECT_ATTRIBUTES ObjectAttributes,
		_In_opt_ PCLIENT_ID			ClientId) -> NTSTATUS
	{
		if (fDenyAllProcess || ClientId->UniqueProcess != GetCurrentProcessId())
			return STATUS_ACCESS_DENIED;

		return _NtOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_NtOpenProcess), NtOpenProcess__Hook);
}

VOID Detour__NtQuerySystemInformation()
{
	typedef NTSTATUS(NTAPI * pfnNtQuerySystemInformation)(
		_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
		_Inout_   PVOID                    SystemInformation,
		_In_      ULONG                    SystemInformationLength,
		_Out_opt_ PULONG                   ReturnLength);

	static pfnNtQuerySystemInformation _NtQuerySystemInformation = reinterpret_cast<pfnNtQuerySystemInformation>(GetProcAddress(
		GetModuleHandle(TEXT("ntdll")), "NtQuerySystemInformation"));

	pfnNtQuerySystemInformation NtQuerySystemInformation__Hook = [](
		_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
		_Inout_   PVOID                    SystemInformation,
		_In_      ULONG                    SystemInformationLength,
		_Out_opt_ PULONG                   ReturnLength) -> NTSTATUS
	{
		HMODULE hModule;
		NTSTATUS ret;

		// SystemCurrentTimeZoneInformation
		if (SystemInformationClass == (SYSTEM_INFORMATION_CLASS)0x2C)
			return STATUS_ACCESS_DENIED;

		ret = _NtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

		if (SystemInformationClass == SystemProcessInformation && ret == STATUS_SUCCESS)
		{
			if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<TCHAR*>(_ReturnAddress()), &hModule))
			{
				if (SystemInformation)
					((PSYSTEM_PROCESS_INFORMATION)SystemInformation)->NextEntryOffset = 0;
			}
		}

		return ret;
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_NtQuerySystemInformation), NtQuerySystemInformation__Hook);
}

VOID Detour__NtOpenThread()
{
	typedef struct _CLIENT_ID
	{
		DWORD UniqueProcess;
		DWORD UniqueThread;
	} CLIENT_ID, *PCLIENT_ID;

	typedef NTSTATUS(NTAPI * pfnNtOpenThread)(
		_Out_ PHANDLE            ThreadHandle,
		_In_  ACCESS_MASK        DesiredAccess,
		_In_  POBJECT_ATTRIBUTES ObjectAttributes,
		_In_  PCLIENT_ID         ClientId);

	static pfnNtOpenThread _NtOpenThread = reinterpret_cast<pfnNtOpenThread>(GetProcAddress(
		GetModuleHandle(TEXT("ntdll")), "NtOpenThread"));

	pfnNtOpenThread NtOpenThread__Hook = [](
		_Out_ PHANDLE            ThreadHandle,
		_In_  ACCESS_MASK        DesiredAccess,
		_In_  POBJECT_ATTRIBUTES ObjectAttributes,
		_In_  PCLIENT_ID         ClientId) -> NTSTATUS
	{
		if (DesiredAccess & THREAD_ALL_ACCESS)
			DesiredAccess &= ~(THREAD_SET_THREAD_TOKEN);

		DesiredAccess &= ~(THREAD_QUERY_INFORMATION);

		return _NtOpenThread(ThreadHandle, DesiredAccess, ObjectAttributes, ClientId);
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_NtOpenThread), NtOpenThread__Hook);
}

VOID Detour__WideCharToMultiByte()
{
	static decltype(&WideCharToMultiByte) _WideCharToMultiByte = WideCharToMultiByte;

	decltype(&WideCharToMultiByte) WideCharToMultiByte__Hook = [](
		_In_ UINT CodePage,
		_In_ DWORD dwFlags,
		_In_NLS_string_(cchWideChar) LPCWCH lpWideCharStr,
		_In_ int cchWideChar,
		_Out_writes_bytes_to_opt_(cbMultiByte, return) LPSTR lpMultiByteStr,
		_In_ int cbMultiByte,
		_In_opt_ LPCCH lpDefaultChar,
		_Out_opt_ LPBOOL lpUsedDefaultChar) -> int
	{
		DWORD dwNumberOfBytesWritten;
		HANDLE hFile;

		int ret = _WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr,
			cbMultiByte, lpDefaultChar, lpUsedDefaultChar);

		if (CodePage == CP_UTF8 && cbMultiByte == 239)
		{
			if (wcsstr(lpWideCharStr, L"DRIVER") == NULL &&
				wcsstr(lpWideCharStr, L"PROCESS") == NULL &&
				wcsstr(lpWideCharStr, L"MODULE") == NULL)
			{
#ifdef XC3_LOG
				hFile = CreateFile(TEXT("xigncode.txt"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
					OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile != INVALID_HANDLE_VALUE)
				{
					SetFilePointer(hFile, 0, NULL, FILE_END);
					WriteFile(hFile, lpMultiByteStr, ret, &dwNumberOfBytesWritten, NULL);
					WriteFile(hFile, "\r\n", strlen("\r\n"), &dwNumberOfBytesWritten, NULL);
					CloseHandle(hFile);
				}
				printf("%ws\n", lpWideCharStr);
#endif
			}
		}
		if (!fDenyAllProcess)
		{
			if (wcsstr(lpWideCharStr, L"xkaga init c"))
				fDenyAllProcess = TRUE;
		}

		return ret;
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_WideCharToMultiByte), WideCharToMultiByte__Hook);
}

VOID Detour__WriteFile()
{
	static decltype(&WriteFile) _WriteFile = WriteFile;

	decltype(&WriteFile) WriteFile__Hook = [](
		_In_ HANDLE hFile,
		_In_reads_bytes_opt_(nNumberOfBytesToWrite) LPCVOID lpBuffer,
		_In_ DWORD nNumberOfBytesToWrite,
		_Out_opt_ LPDWORD lpNumberOfBytesWritten,
		_Inout_opt_ LPOVERLAPPED lpOverlapped) -> BOOL
	{
		buffer::request *request = (buffer::request*)lpBuffer;

		if (nNumberOfBytesToWrite == XIGNCODE_BUFSIZE &&
			request->size == XIGNCODE_BUFSIZE &&
			request->signature == XIGNCODE_SIGNATURE)
		{
			buffer::response *response = (buffer::response*)request->response;

			switch (request->operation)
			{
				case 0x0000030E: // start/stop 'watching'
				{
					// write response headers
					response->size = XIGNCODE_BUFSIZE;
					response->signature = 0x12121212;
					response->auth = ~request->key;
					response->status = STATUS_SUCCESS;

					if (lpNumberOfBytesWritten)
						*lpNumberOfBytesWritten = XIGNCODE_BUFSIZE;

					return TRUE;
				}
				case 0x00000311: // OpenProcess
				case 0x00000313: // rpm?
				case 0x00000314: // memcpy?
				case 0x00000315: // get driver name
				case 0x00000317: // ZwQueryInformationProcess?
				case 0x00000318: // ZwWow64QueryInformationProcess64 (to get PEB)
				case 0x00000319: // some value from process
				case 0x0000031A: // QueryFullProcessImageNameW
				{
					return FALSE;
				}
				case 0x00000312: // some sort of value check used for 'MM.XMOD HkCorrupted'
				{
					static BOOL static_f = 1;
					// write response headers
					response->size = XIGNCODE_BUFSIZE;
					response->signature = 0x12121212;
					response->auth = ~request->key;
					response->status = STATUS_SUCCESS;
					response->params[0] = static_f;

					static_f = !static_f;

					if (lpNumberOfBytesWritten)
						*lpNumberOfBytesWritten = XIGNCODE_BUFSIZE;

					return TRUE;
				}
			}
		}

		return _WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_WriteFile), WriteFile__Hook);
}

VOID Detour__SetEnvironmentVariableW()
{
	static decltype(&SetEnvironmentVariableW) _SetEnvironmentVariableW = SetEnvironmentVariableW;

	decltype(&SetEnvironmentVariableW) SetEnvironmentVariableW__Hook = [](
		_In_ LPCWSTR lpName,
		_In_opt_ LPCWSTR lpValue) -> BOOL
	{
		if (wcsstr(lpName, L"TrayMsg") != NULL)
		{
			// XIGNCODE3 53942
			// CHECK VERSION HERE
			if (lpValue)
			{
				MessageBoxW(NULL, lpValue, L"TrayMsg", MB_OK);
			}
		}

		return _SetEnvironmentVariableW(lpName, lpValue);
	};

	DetourFunction(TRUE, reinterpret_cast<LPVOID*>(&_SetEnvironmentVariableW), SetEnvironmentVariableW__Hook);
}

VOID xigncode_bypass()
{
#ifdef XC3_LOG
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* pFile = nullptr;
	freopen_s(&pFile, "CON", "r", stdin);
	freopen_s(&pFile, "CON", "w", stdout);
	freopen_s(&pFile, "CON", "w", stderr);
#endif
	Detour__WriteFile();
	Detour__WideCharToMultiByte();
	Detour__NtOpenThread();
	Detour__NtOpenProcess();
	Detour__NtQuerySystemInformation();
	Detour__GetWindowThreadProcessId();
}