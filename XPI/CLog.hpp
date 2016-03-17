#ifndef CLOG_HPP_
#define CLOG_HPP_

#define LOG_BUFFER_SIZE 2000

#define LOG_WF_NOPREFIX  ((DWORD)0x00000002)
#define LOG_WF_ERROR     ((DWORD)0x00000004)
#define LOG_WF_ECHODEBUG ((DWORD)0x00000008)
#define LOG_WF_SDEBUG    ((DWORD)0x00000010)
#define LOG_WF_DEBUG     (LOG_WF_SDEBUG | LOG_WF_ECHODEBUG)

struct _LOG_WRITE
{
	SYSTEMTIME  Time;
	LPCWSTR     lpcwszPrefix;
	LPCWSTR     lpcwszFormat;
	va_list     pArguments;
	BOOL        bPrintDebug;
};

class CLog
{
private:
	HANDLE  m_hFile;
	WCHAR   m_wszName[100];
	WCHAR   m_wszFile[MAX_PATH];
protected:
	VOID Write(__in _LOG_WRITE* lpLogWrite);
	VOID ToFile(__in __format_string LPCWSTR lpcwszFormat, ...);
	VOID DebugPrint(__in __format_string LPCWSTR lpcwszFormat, ...);
	VOID WriteHeader();
public:
	VOID Write(__in __format_string LPCWSTR lpcwszFormat, ...);
	VOID Write(__in DWORD fdwOptions, __in __format_string LPCWSTR lpcwszFormat, ...);
	// ctor/dtor
	explicit CLog(__in LPCWSTR lpcwszName, __in_opt LPCWSTR lpcwszFile = NULL);
	~CLog();
};

#endif // CLOG_HPP_
