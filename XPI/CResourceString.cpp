#include "stdafx.h"

#include "CResourceString.hpp"

#include <stdexcept>

CResourceString::CResourceString(__in HINSTANCE hInstance, __in WORD wLanguage)
{
	m_hInstance = hInstance;
	m_wLanguage = wLanguage;
}

VOID CResourceString::SetLanguage(__in WORD wLanguage)
{
	m_wLanguage = wLanguage;
}

const std::wstring CResourceString::Get(__in UINT uID)
{
	// convert string ID to bundle ID (16 strings to a bundle)
	HRSRC hResource = FindResourceEx(m_hInstance, RT_STRING, MAKEINTRESOURCE(uID / 16 + 1), m_wLanguage);

	if (hResource == NULL)
		return std::wstring();

	// load the bundle
	HGLOBAL hResData = LoadResource(m_hInstance, hResource);

	if (hResData == NULL)
		return std::wstring();

	// get a pointer to the string bundle
	LPCWSTR lpcwszTemp = reinterpret_cast<LPCWSTR>(LockResource(hResData));

	if (lpcwszTemp == NULL)
		return std::wstring();

	// for as many strings are in the bundle before the wanted string, skip over its entry
	// each string entry is represented as an ordered pair in the bundle (length, text), where the length is the first WCHAR
	// therefore, we skip over the length entry, and however many characters the length entry represents
	for (UINT i = 0; i < (uID & 15); i++)
		lpcwszTemp += 1 + (UINT)*lpcwszTemp;

	// store the string length and move the pointer to point at the first character in the string
	WORD wLength = (WORD)*lpcwszTemp++;

	// initializing the std::wstring with both string pointer and length is more effecient than only giving the string pointer
	// (no crawling the string for null-terminator)
	std::wstring wstrTemp(lpcwszTemp, wLength);

	FreeResource(hResData);

	return wstrTemp;
}

const std::wstring CResourceString::operator[](__in UINT uID)
{
	return Get(uID);
}
