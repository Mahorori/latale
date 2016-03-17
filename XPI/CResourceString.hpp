#ifndef CRESOURCE_STRING_HPP_
#define CRESOURCE_STRING_HPP_

#include <string>

class CResourceString
{
private:
	HINSTANCE m_hInstance;
	WORD      m_wLanguage;
public:
	CResourceString(__in HINSTANCE hInstance, __in WORD wLanguage = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
	VOID SetLanguage(__in WORD wLanguage);
	const std::wstring Get(__in UINT uID);
	const std::wstring operator[](__in UINT uID);
};

#endif // CRESOURCE_STRING_HPP_
