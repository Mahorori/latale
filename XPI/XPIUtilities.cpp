#include "stdafx.h"

#include "XPIUtilities.hpp"

#include <boost/scoped_array.hpp>

#include "extvars.hpp"

// keeping it in this file due to the static declaration ensuring
// it's only within the scope of this file
static const BYTE s_cbHexCharacterLUT[0x100] =
{
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x08
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x10
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x18
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x20
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x28
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x30
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, // 0x38
	0x8, 0x9, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x40
	NB_ERR, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, NB_ERR, // 0x48
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x50
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x58
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x60
	NB_ERR, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, NB_ERR, // 0x68
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x70
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x78
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x80
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x88
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x90
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x98
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xA0
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xA8
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xB0
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xB8
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xC0
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xC8
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xD0
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xD8
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xE0
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xE8
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xF0
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0xF8
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR  // 0xFF
};

BOOL StringToBuffer(__in std::wstring wstr, __inout std::vector<BYTE>* pvbBuffer)
{
	BYTE bN1, bN2;

	// clear the buffer beforehand
	pvbBuffer->clear();

	for (UINT i = 0; i < wstr.length(); i++)
	{
		if (isspace(wstr[i] & 0xFF))
			continue;

		// get first nibble
		bN1 = s_cbHexCharacterLUT[wstr[i++] & 0xFF];

		// skip any white-spaces
		while (isspace(wstr[i] & 0xFF))
			i++;

		// grab second nibble
		bN2 = s_cbHexCharacterLUT[wstr[i] & 0xFF];

		if (bN1 == NB_ERR || bN2 == NB_ERR)
			return FALSE;

		pvbBuffer->push_back(bN1 << 4 | bN2);
	}

	return TRUE;
}

BOOL StringToBuffer(__in std::string str, __inout std::vector<BYTE>* pvbBuffer)
{
	INT iLength;

	iLength = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), NULL, 0);
	if (iLength == 0)
		return FALSE;

	boost::scoped_array<WCHAR> wszBuffer(new WCHAR[iLength]);

	if (MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), wszBuffer.get(), iLength) == 0)
		return FALSE;

	return StringToBuffer(wszBuffer.get(), pvbBuffer);
}

BOOL IsOpcodeBlocked(__in DWORD dwOpcode)
{
	if (pOpcodeInfo == NULL)
		return FALSE;

	return (*pOpcodeInfo)[dwOpcode].bBlock;
}

HANDLE AddFontFromResource(__in HMODULE hModule, __in LPCWSTR lpcwszName)
{
	HRSRC   hResource;
	DWORD   dwFontCount = 0;

	hResource = FindResourceW(hModule, lpcwszName, RT_FONT);
	if (hResource == NULL)
		return NULL;

	HANDLE hRET = NULL;

	HGLOBAL hMemory = LoadResource(hModule, hResource);
	if (hMemory != NULL)
	{
		LPVOID lpData = LockResource(hMemory);
		if (lpData != NULL)
			hRET = AddFontMemResourceEx(lpData, SizeofResource(hModule, hResource), NULL, &dwFontCount);
		FreeResource(hMemory);
	}

	return hRET;
}

INT MessageBoxIcon(__in HWND hwndOwner, __in LPCWSTR lpcwszText, __in LPCWSTR lpcwszCaption, __in UINT uType, __in HINSTANCE hInstance, __in LPCWSTR lpcwszIcon)
{
	MSGBOXPARAMS mbp = { 0 };

	mbp.cbSize = sizeof(mbp);
	mbp.hwndOwner = hwndOwner;
	mbp.hInstance = hInstance;
	mbp.lpszText = lpcwszText;
	mbp.lpszCaption = lpcwszCaption;
	mbp.dwStyle = uType | MB_USERICON;
	mbp.lpszIcon = lpcwszIcon;

	return MessageBoxIndirect(&mbp);
}

BOOL StringToWString(__in std::string str, __inout std::wstring &wstr)
{
	INT nLength;

	nLength = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (wchar_t*)NULL, 0);
	if (nLength == 0)
		return FALSE;

	boost::scoped_array<WCHAR> wszBuffer(new WCHAR[nLength]);

	if (MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wszBuffer.get(), nLength) == 0)
		return FALSE;

	wstr = std::wstring(wszBuffer.get(), wszBuffer.get() + nLength - 1);

	return TRUE;
}