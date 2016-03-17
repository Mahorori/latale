#ifndef CHEADER_HPP_
#define CHEADER_HPP_

#include <gdiplus.h>

#include <string>

#define SUBTEXT_SPACING 1
#define HEADER_PADDING  10

enum HBG_TYPE { BACKGROUND_SOLID, BACKGROUND_GRADIENT };

class CHeader
{
private:
	HWND          m_hWnd;
	/**/
	std::wstring  m_wstrTitle;
	std::wstring  m_wstrSubText;
	/**/
	HFONT         m_hTitleFont;
	HFONT         m_hSubTextFont;
	/**/
	HBG_TYPE      m_Type; // background brush type
	COLORREF      m_crText; // text colour
	UINT          m_uSpan; // image span
	/**/
	Gdiplus::Color   m_Color1, m_Color2; // background brush color(s)
	Gdiplus::Image*  m_pImage; // header image
	/**/
	Gdiplus::LinearGradientMode   m_GradientMode; // background gradient direction
	/**/
	ULONG_PTR m_ulToken;
	Gdiplus::GdiplusStartupInput  m_StartupInput;
	/**/
	HGLOBAL       m_hResBuffer;
public:
	explicit CHeader(__in HWND hWnd);
	~CHeader();
	BOOL Initialize(__in HWND hWnd);
	/**/
	VOID Paint(__in HDC hDC);
	/**/
	VOID SetTitle(__in const std::wstring cwstrTitle);
	VOID SetSubText(__in_opt const std::wstring cwstrSubText = std::wstring());
	/**/
	VOID SetBackground(__in Gdiplus::Color Color1, __in Gdiplus::Color Color2, __in_opt Gdiplus::LinearGradientMode GradientMode = Gdiplus::LinearGradientModeVertical);
	VOID SetBackground(__in Gdiplus::Color Color);
	/**/
	VOID SetImage(__in Gdiplus::Image* pImage);
	BOOL SetImageFromPNGResource(__in HINSTANCE hInstance, __in LPCWSTR lpcwszName);
	/**/
	const std::wstring GetTitle() const;
	const std::wstring GetSubText() const;
};

#endif // CHEADER_HPP_