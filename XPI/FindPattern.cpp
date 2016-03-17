#include "stdafx.h"

#include "FindPattern.h"

#include <boost/scoped_array.hpp>

static const BYTE s_cbNibble[0x100] =
{
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x08
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x10
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x18
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x20
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x28
	NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, // 0x30
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, // 0x38
	0x8, 0x9, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_ERR, NB_WC, // 0x40
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

BOOL GenerateXPattern(__out LPSRCH_CTX lpCTX, __in LPCSTR lpcszPattern)
{
	USHORT  usCurrent;
	BYTE    bN1, bN2;

	lpCTX->pusXPattern = new USHORT[strlen(lpcszPattern) * 2 + 2];
	lpCTX->nLength = 0;

	for (INT i = 0; lpcszPattern[i] != '\0'; i++)
	{
		// skip white-spaces
		if (isspace(lpcszPattern[i]) != 0)
			continue;

		// grab first nibble [X?]
		bN1 = s_cbNibble[lpcszPattern[i++]];

		// skip white-spaces
		while (isspace(lpcszPattern[i]) != 0)
			i++;

		// grab second nibble [?X]
		bN2 = s_cbNibble[lpcszPattern[i]];

		if (bN1 == NB_ERR || bN2 == NB_ERR)
		{
			delete lpCTX->pusXPattern;
			return FALSE;
		}

		// generate & store x-pattern
		usCurrent = bN1 == NB_WC ? NB_WC_1 : bN1 << 4;
		usCurrent |= bN2 == NB_WC ? NB_WC_2 : bN2;

		lpCTX->pusXPattern[lpCTX->nLength++] = usCurrent;
	}

	return TRUE;
}

__inline BOOL XEqual(__in WORD w1, __in WORD w2)
{
	switch ((w1 >> 8) | (w2 >> 8))
	{
	case 0: return w1 == w2;
	case 1: return (w1 & 0x0F) == (w2 & 0x0F);
	case 2: return (w1 & 0xF0) == (w2 & 0xF0);
	case 3: return TRUE;
	default: return FALSE;
	}
}

VOID CalculateBC(__inout LPSRCH_CTX lpCTX)
{
	INT cLength;

	// パターンに無い文字のずらし幅
	for (INT i = 0; i < _countof(lpCTX->iBC); i++)
		lpCTX->iBC[i] = lpCTX->nLength + 1;

	// パターンに含まれる文字のずらし幅
	for (INT i = 0; i < lpCTX->nLength; i++)
	{
		cLength = lpCTX->nLength - i;

		switch (lpCTX->pusXPattern[i] >> 8)
		{
		case 1:	// NB_WC_1 (X?)
			for (INT n = 0; n < 0x10; n++)
				lpCTX->iBC[(n << 4) | (lpCTX->pusXPattern[i] & 0x0F)] = cLength;
			break;

		case 2:	// NB_WC_2
			for (INT n = 0; n < 0x10; n++)
				lpCTX->iBC[n | (lpCTX->pusXPattern[i] & 0xF0)] = cLength;
			break;

		case 3:	// NB_WC_1 | NB_WC_2
			for (INT n = 0; n < _countof(lpCTX->iBC); n++)
				lpCTX->iBC[n] = cLength;
			break;

		default:
			lpCTX->iBC[lpCTX->pusXPattern[i]] = cLength;
		}
	}
}

LPVOID PerformSearch(__in LPSRCH_CTX lpCTX, __in_bcount(nSize) const PBYTE pcbData, __in INT nSize)
{
	INT i, j;

	if (nSize < lpCTX->nLength)
		return NULL;

	i = 0;

	while (i + lpCTX->nLength - 1 < nSize)
	{
		// 先頭から探索する。
		j = 0;

		while (j < lpCTX->nLength && XEqual(lpCTX->pusXPattern[j], pcbData[i + j]))
		{
			// 一致した場合1byte右にずらす。
			j++;
		}

		if (j == lpCTX->nLength)
			return &pcbData[i];
		else
		{
			// テーブル分右にずらす
			if (i + lpCTX->nLength < nSize)
				i += lpCTX->iBC[pcbData[i + lpCTX->nLength]];
			else
				return NULL;
		}
	}

	return NULL;
}

BOOL InitializeSearch(__out LPSRCH_CTX lpCTX, __in LPCSTR lpcszPattern)
{
	ZeroMemory(lpCTX, sizeof(SRCH_CTX));

	if (GenerateXPattern(lpCTX, lpcszPattern))
	{
		CalculateBC(lpCTX);
		return TRUE;
	}

	return FALSE;
}

VOID EndSearch(__out LPSRCH_CTX lpCTX)
{
	if (lpCTX->pusXPattern != NULL)
		delete[] lpCTX->pusXPattern;
}

LPVOID FindPatternA(__in_bcount(nSize) LPCVOID lpcv, __in INT nSize, __in LPCSTR lpcszPattern)
{
	SRCH_CTX  ctx;
	LPVOID    lpRET = NULL;

	if (InitializeSearch(&ctx, lpcszPattern))
	{
		lpRET = PerformSearch(&ctx, (const PBYTE)lpcv, nSize);
		EndSearch(&ctx);
	}

	return lpRET;
}

LPVOID FindPatternW(__in_bcount(nSize) LPCVOID lpcv, __in INT nSize, __in LPCWSTR lpcwszPattern)
{
	SRCH_CTX  ctx;
	INT       nLength;

	nLength = WideCharToMultiByte(CP_ACP, 0, lpcwszPattern, -1, NULL, 0, NULL, NULL);
	if (nLength < 0)
		return NULL;

	boost::scoped_array<CHAR> szBuffer(new CHAR[nLength]);

	if (WideCharToMultiByte(CP_ACP, 0, lpcwszPattern, nLength, szBuffer.get(), nLength, NULL, NULL) == 0)
		return NULL;

	LPVOID lpRET = NULL;

	if (InitializeSearch(&ctx, szBuffer.get()))
	{
		lpRET = PerformSearch(&ctx, (const PBYTE)lpcv, nSize);
		EndSearch(&ctx);
	}

	return lpRET;
}