#include "stdafx.h"

#include "CHeader.hpp"

#include <uxtheme.h>
#include <vssym32.h>

#pragma comment(lib, "gdiplus")

BOOL CHeader::Initialize(__in HWND hWnd)
{
	HTHEME hTheme = OpenThemeData(hWnd, L"TaskDialog");

	if (hTheme == NULL)
	{
		// task dialog theme does not exist in Windows XP, assure font support
		m_hTitleFont = CreateFontW(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, L"MS Shell Dlg");
		if (m_hTitleFont == NULL)
			return FALSE;

		m_hSubTextFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, L"MS Shell Dlg");
		if (m_hSubTextFont == NULL)
			return FALSE;
	}
	else
	{
		LOGFONTW lfw;

		do
		{
			if (SUCCEEDED(GetThemeFont(hTheme, NULL, TDLG_MAININSTRUCTIONPANE, 0, TMT_FONT, &lfw)))
			{
				lfw.lfWeight = FW_SEMIBOLD;
				m_hTitleFont = CreateFontIndirectW(&lfw);
				if (m_hTitleFont != NULL)
				{
					if (SUCCEEDED(GetThemeFont(hTheme, NULL, TDLG_CONTENTPANE, 0, TMT_FONT, &lfw)))
					{
						m_hSubTextFont = CreateFontIndirectW(&lfw);
						if (m_hSubTextFont != NULL)
							break;
						// DeleteObject(m_hSubTextFont);
					}
				}
				DeleteObject(m_hTitleFont);
			}
			CloseThemeData(hTheme);
			return FALSE;
		} while (0);
	}

	if (Gdiplus::GdiplusStartup(&m_ulToken, &m_StartupInput, NULL) == Gdiplus::Ok)
	{
		m_hWnd = hWnd;
		m_pImage = NULL;
		m_crText = RGB(255, 255, 255);
		m_uSpan = 0;
		m_Type = BACKGROUND_GRADIENT;
		m_Color1 = Gdiplus::Color(49, 49, 49);
		m_Color2 = Gdiplus::Color(0, 0, 0);
		m_GradientMode = Gdiplus::LinearGradientModeVertical;
		/***/
		m_hResBuffer = NULL;
		/***/
	}

	CloseThemeData(hTheme);
	return TRUE;
}

CHeader::CHeader(__in HWND hWnd)
{
	if (!Initialize(hWnd))
		throw std::runtime_error("couldn't initialize header class!");
}

CHeader::~CHeader()
{
	if (m_pImage != NULL)
		delete m_pImage;

	if (m_hResBuffer != NULL)
	{
		GlobalUnlock(m_hResBuffer);
		GlobalFree(m_hResBuffer);
	}

	DeleteObject(m_hTitleFont);
	DeleteObject(m_hSubTextFont);

	Gdiplus::GdiplusShutdown(m_ulToken);
}

VOID CHeader::Paint(__in HDC hDC)
{
	RECT ClientRect;
	UINT uPadding = m_uSpan + (HEADER_PADDING * 2);

	if (!GetClientRect(m_hWnd, &ClientRect))
		return;

	Gdiplus::Graphics  Graphics(hDC);
	Gdiplus::Rect      HeaderRect(0, 0, ClientRect.right, uPadding);

	if (m_Type == BACKGROUND_GRADIENT)
	{
		Gdiplus::LinearGradientBrush Brush(HeaderRect, m_Color1, m_Color2, m_GradientMode);

		if (Graphics.FillRectangle(&Brush, HeaderRect) != Gdiplus::Ok)
			return;
	}
	else // m_Type == BACKGROUND_SOLID
	{
		Gdiplus::SolidBrush Brush(m_Color1);

		if (Graphics.FillRectangle(&Brush, HeaderRect) != Gdiplus::Ok)
			return;
	}

	// draw an image if there is one
	if (m_pImage != NULL)
		Graphics.DrawImage(m_pImage, HEADER_PADDING, HEADER_PADDING);

	// set text color
	COLORREF crTemp = SetTextColor(hDC, m_crText);
	if (crTemp == CLR_INVALID)
		return;

	// set background colour
	int iBkMode = SetBkMode(hDC, TRANSPARENT);
	if (iBkMode == 0)
	{
		SetTextColor(hDC, crTemp);
		return;
	}

	// set logo font
	HGDIOBJ hObject = SelectObject(hDC, m_hTitleFont);
	if (hObject == NULL || hObject == (HGDIOBJ)GDI_ERROR)
	{
		SetTextColor(hDC, crTemp);
		SetBkMode(hDC, iBkMode);
		return;
	}

	// calculate title text extent
	SIZE TitleSize;
	if (GetTextExtentPoint32W(hDC, m_wstrTitle.c_str(), m_wstrTitle.length(), &TitleSize))
	{
		if (m_wstrSubText.empty()) // one-line header
		{
			ExtTextOutW(hDC, uPadding, (uPadding / 2) - (TitleSize.cy / 2), 0, NULL, m_wstrTitle.c_str(), m_wstrTitle.length(), NULL);
		}
		else // two-line header
		{
			SIZE SubTextSize;

			if (GetTextExtentPoint32W(hDC, m_wstrSubText.c_str(), m_wstrSubText.length(), &SubTextSize))
			{
				UINT uOffset = (uPadding / 2) - (TitleSize.cy / 2) - (SubTextSize.cy / 2) - SUBTEXT_SPACING;

				if (ExtTextOutW(hDC, uPadding, uOffset, 0, NULL, m_wstrTitle.c_str(), m_wstrTitle.length(), NULL))
				{
					HGDIOBJ hTemp;

					uOffset += TitleSize.cy + SUBTEXT_SPACING;
					hTemp = SelectObject(hDC, m_hSubTextFont);

					if (hTemp != NULL && hTemp != (HGDIOBJ)GDI_ERROR)
						ExtTextOutW(hDC, uPadding, uOffset, 0, NULL, m_wstrSubText.c_str(), m_wstrSubText.length(), NULL);
				}
			}
		}

		SelectObject(hDC, hObject);
		SetTextColor(hDC, crTemp);
		SetBkMode(hDC, iBkMode);
	}
}

VOID CHeader::SetImage(__in Gdiplus::Image* pImage)
{
	UINT uHeight;

	uHeight = pImage->GetHeight();

	// only accept even images
	if (uHeight != pImage->GetWidth())
		return;

	if (m_pImage != NULL)
		delete m_pImage;

	if (m_hResBuffer != NULL)
	{
		GlobalUnlock(m_hResBuffer);
		GlobalFree(m_hResBuffer);
		m_hResBuffer = NULL;
	}

	m_uSpan = uHeight;
	m_pImage = pImage;
}

BOOL CHeader::SetImageFromPNGResource(__in HINSTANCE hInstance, __in LPCWSTR lpcwszName)
{
	if (m_pImage != NULL)
		delete m_pImage;

	HRSRC hResource = FindResourceW(hInstance, lpcwszName, L"PNG");
	if (!hResource)
		return FALSE;

	DWORD dwResSize = SizeofResource(hInstance, hResource);
	if (!dwResSize)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInstance, hResource);
	if (!hGlobal)
		return FALSE;

	PVOID pResourceData = LockResource(hGlobal);
	if (!pResourceData)
		return FALSE;

	m_hResBuffer = GlobalAlloc(GMEM_MOVEABLE, dwResSize);
	if (m_hResBuffer)
	{
		void* pBuffer = GlobalLock(m_hResBuffer);
		if (pBuffer)
		{
			CopyMemory(pBuffer, pResourceData, dwResSize);

			IStream* pStream = NULL;
			if (CreateStreamOnHGlobal(m_hResBuffer, FALSE, &pStream) == S_OK)
			{
				m_pImage = Gdiplus::Bitmap::FromStream(pStream);
				pStream->Release();
				if (m_pImage)
				{
					if (m_pImage->GetLastStatus() == Gdiplus::Ok)
					{
						UINT uHeight;

						uHeight = m_pImage->GetHeight();

						// only accept even images
						if (uHeight == m_pImage->GetWidth())
						{
							m_uSpan = uHeight;
							return TRUE;
						}
					}

					delete m_pImage;
					m_pImage = NULL;
				}
			}
			GlobalUnlock(m_hResBuffer);
		}
		GlobalFree(m_hResBuffer);
		m_hResBuffer = NULL;
	}

	return FALSE;
}

VOID CHeader::SetBackground(__in Gdiplus::Color Color1, __in Gdiplus::Color Color2, __in_opt Gdiplus::LinearGradientMode GradientMode)
{
	m_Type = BACKGROUND_GRADIENT;

	m_Color1 = Color1;
	m_Color2 = Color2;

	m_GradientMode = GradientMode;
}

VOID CHeader::SetBackground(__in Gdiplus::Color Color)
{
	m_Type = BACKGROUND_SOLID;

	m_Color1 = Color;
}

VOID CHeader::SetTitle(__in const std::wstring cwstrTitle)
{
	m_wstrTitle = cwstrTitle;
}

VOID CHeader::SetSubText(__in_opt const std::wstring cwstrSubText)
{
	m_wstrSubText = cwstrSubText;
}

const std::wstring CHeader::GetTitle() const
{
	return m_wstrTitle;
}

const std::wstring CHeader::GetSubText() const
{
	return m_wstrSubText;
}
