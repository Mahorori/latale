#include "stdafx.h"

#include "CLog.hpp"

#include <psapi.h>
#include <stdexcept>
#pragma  comment(lib, "psapi")

#define _SafeStringFormatW(buff, fmt, ...) SUCCEEDED(StringCchPrintfW(buff, _countof(buff), fmt, __VA_ARGS__))
#define _SafeStringVFormatW(buff, fmt, args) SUCCEEDED(StringCchVPrintfW(buff, _countof(buff), fmt, args))
#define _SafeStringCopyW(buff, src) SUCCEEDED(StringCchCopyW(buff, _countof(buff), src))
#define _SafeStringLengthW(str, size) SUCCEEDED(StringCchLengthW(str, _countof(str), size))

// keep huge buffers off the stack
__declspec(thread) WCHAR wszLogBuffer[LOG_BUFFER_SIZE];
__declspec(thread) WCHAR wszLogMessage[LOG_BUFFER_SIZE];

VOID CLog::DebugPrint(__in __format_string LPCWSTR lpcwszFormat, ...)
{
	va_list pArguments;

	va_start(pArguments, lpcwszFormat);
	if (_SafeStringVFormatW(wszLogBuffer, lpcwszFormat, pArguments))
		OutputDebugStringW(wszLogBuffer);
	va_end(pArguments);
}

VOID CLog::Write(__in __format_string LPCWSTR lpcwszFormat, ...)
{
	_LOG_WRITE  LogWrite;

	GetLocalTime(&LogWrite.Time);
	LogWrite.lpcwszFormat = lpcwszFormat;
	LogWrite.lpcwszPrefix = L"LOG";
	LogWrite.bPrintDebug = FALSE;
	va_start(LogWrite.pArguments, lpcwszFormat);
	Write(&LogWrite);
	va_end(LogWrite.pArguments);
}

VOID CLog::Write(__in DWORD fdwOptions, __in __format_string LPCWSTR lpcwszFormat, ...)
{
	_LOG_WRITE  LogWrite;

	GetLocalTime(&LogWrite.Time);

	if (fdwOptions & LOG_WF_NOPREFIX)
		LogWrite.lpcwszPrefix = NULL;
	else if (fdwOptions & LOG_WF_ERROR)
		LogWrite.lpcwszPrefix = L"ERROR";
	else if (fdwOptions & LOG_WF_SDEBUG)
		LogWrite.lpcwszPrefix = L"DEBUG";
	else
		LogWrite.lpcwszPrefix = L"LOG";

	LogWrite.bPrintDebug = (fdwOptions & LOG_WF_ECHODEBUG) != 0;
	LogWrite.lpcwszFormat = lpcwszFormat;
	va_start(LogWrite.pArguments, lpcwszFormat);
	Write(&LogWrite);
	va_end(LogWrite.pArguments);
}

VOID CLog::Write(__in _LOG_WRITE* lpLogWrite)
{
	LPWSTR  lpwszToken, lpwszContext;
	size_t  nSize;
	WCHAR   wszTime[10], wszPrefix[6];

	if (lpLogWrite->lpcwszPrefix != NULL)
	{
		if (FAILED(StringCchLengthW(lpLogWrite->lpcwszPrefix, 7, &nSize)))
			return;
		if (nSize > 5)
			return;
		if (!_SafeStringCopyW(wszPrefix, lpLogWrite->lpcwszPrefix))
			return;
		if (nSize < 5)
		{
			for (; nSize < 5; nSize++)
				wszPrefix[nSize] = L' ';
			wszPrefix[nSize] = L'\0';
		}
	}

	if (GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &lpLogWrite->Time, L"HH':'mm':'ss", wszTime, _countof(wszTime)) == 0)
		return;

	if (!_SafeStringVFormatW(wszLogMessage, lpLogWrite->lpcwszFormat, lpLogWrite->pArguments))
		return;

	lpwszToken = wcstok_s(wszLogMessage, L"\r\n", &lpwszContext);

	if (lpLogWrite->lpcwszPrefix != NULL)
	{
		if (lpwszToken == NULL)
		{
			ToFile(L"[%s] %.5s >> %s\r\n", wszTime, wszPrefix, wszLogMessage);
			if (lpLogWrite->bPrintDebug)
				DebugPrint(L"[%s] %s\r\n", m_wszName, wszLogMessage);
		}
		else
		{
			ToFile(L"[%s] %.5s >> %s\r\n", wszTime, wszPrefix, lpwszToken);
			if (lpLogWrite->bPrintDebug)
				DebugPrint(L"[%s] %s\r\n", m_wszName, lpwszToken);
			while ((lpwszToken = wcstok_s(NULL, L"\r\n", &lpwszContext)) != NULL)
			{
				ToFile(L"[%s]       >> %s\r\n", wszTime, lpwszToken);
				if (lpLogWrite->bPrintDebug)
					DebugPrint(L"[%s] %s\r\n", m_wszName, lpwszToken);
			}
		}
	}
	else
	{
		if (lpwszToken == NULL)
		{
			ToFile(L"[%s] %s\r\n", wszTime, wszLogMessage);
			if (lpLogWrite->bPrintDebug)
				DebugPrint(L"[%s] %s\r\n", m_wszName, wszLogMessage);
		}
		else
		{
			do
			{
				ToFile(L"[%s] %s\r\n", wszTime, lpwszToken);
				if (lpLogWrite->bPrintDebug)
					DebugPrint(L"[%s] %s\r\n", m_wszName, lpwszToken);
			} while ((lpwszToken = wcstok_s(NULL, L"\r\n", &lpwszContext)) != NULL);
		}
	}
}

VOID CLog::ToFile(__in __format_string LPCWSTR lpcwszFormat, ...)
{
	va_list pArguments;
	size_t  nSize;
	DWORD   dwnWritten;

	va_start(pArguments, lpcwszFormat);

	if (!_SafeStringVFormatW(wszLogBuffer, lpcwszFormat, pArguments))
	{
		va_end(pArguments);
		return;
	}

	if (!_SafeStringLengthW(wszLogBuffer, &nSize))
		return;

	WriteFile(m_hFile, wszLogBuffer, nSize * sizeof(WCHAR), &dwnWritten, NULL);
}

VOID CLog::WriteHeader()
{
	static const WCHAR s_cwszSessionStart[] =
	{
		L"===========================================\r\n"
		L"SESSION START\r\n"
		L"-------------------------------------------"
	};
	static LPCWSTR s_lpcwszMonths[] =
	{
		NULL, // optimization so no - 1 is required for month
		L"January", L"Febuary", L"March",
		L"April", L"May", L"June",
		L"July", L"August", L"September",
		L"October", L"November", L"December"
	};
	static LPCWSTR s_lpcwszDays[] =
	{
		L"Sunday", L"Monday", L"Tuesday", L"Wednesday",
		L"Thursday", L"Friday", L"Saturday"
	};
	/**********/
	SYSTEMTIME  Time;
	LPWSTR      lpwszSuffix;
	WCHAR       wszBaseName[MAX_PATH];

	Write(LOG_WF_NOPREFIX, s_cwszSessionStart);

	if (GetModuleBaseNameW(GetCurrentProcess(), NULL, wszBaseName, _countof(wszBaseName)) != 0)
		Write(LOG_WF_NOPREFIX, L"\"%s\" initialized on \"%s\". [PID=%d]", m_wszName, wszBaseName, GetCurrentProcessId());

	GetLocalTime(&Time);
	switch (Time.wDay)
	{
	case 1:
	case 11:
	case 21:
	case 31:
		lpwszSuffix = L"st";
		break;

	case 2:
	case 22:
		lpwszSuffix = L"nd";
		break;

	case 3:
	case 23:
		lpwszSuffix = L"rd";
		break;

	default:
		lpwszSuffix = L"th";
		break;
	}
	Write(LOG_WF_NOPREFIX, L"%s the %d%s of %s, %d.", s_lpcwszDays[Time.wDayOfWeek], Time.wDay, lpwszSuffix, s_lpcwszMonths[Time.wMonth], Time.wYear);
	Write(LOG_WF_NOPREFIX, L"===========================================");
}

CLog::CLog(__in LPCWSTR lpcwszName, __in_opt LPCWSTR lpcwszFile)
{

	if (!_SafeStringCopyW(m_wszName, lpcwszName))
		throw std::length_error("couldn't fit specified symbolic log name in buffer.");

	if (lpcwszFile != NULL)
	{
		if (!_SafeStringCopyW(m_wszFile, lpcwszFile))
			throw std::length_error("couldn't fit log filename in buffer.");
	}
	else
	{
		if (!_SafeStringFormatW(m_wszFile, L"%s.log", lpcwszName))
			throw std::length_error("couldn't fit log filename in buffer.");
	}

	m_hFile = CreateFileW(m_wszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		throw std::runtime_error("couldn't open/create log file.");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
		SetLastError(ERROR_SUCCESS);

	if (GetFileSize(m_hFile, NULL) > 0)
		SetFilePointer(m_hFile, 0, NULL, FILE_END);

	WriteHeader();
}

CLog::~CLog()
{
	static const WCHAR cwszSessionEnd[] =
	{
		L"===========================================\r\n"
		L"SESSION END\r\n"
		L"==========================================="
	};

	Write(LOG_WF_NOPREFIX, cwszSessionEnd);

	CloseHandle(m_hFile);
}
