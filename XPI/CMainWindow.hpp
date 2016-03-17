#ifndef CMAIN_WINDOW_HPP_
#define CMAIN_WINDOW_HPP_

#include <commctrl.h>
#include <vector>

#include "XPIColors.h"

#define WM_SHELLNOTIFY (WM_USER + 100)

// image list indices (packet direction)
enum
{
	XPI_ILI_RECV = 0,
	XPI_ILI_RECVLB,
	XPI_ILI_SEND,
	XPI_ILI_SENDLB
};

// image list indices (settings)
enum
{
	XPI_SILI_NONE = 0,
	XPI_SILI_IGNORED,
	XPI_SILI_BLOCKED
};

// image list indices (formatted)
enum
{
	XPI_FILI_PLAIN = 0,
	XPI_FILI_FORMATTED
};

typedef struct _XPIGUI
{
	// app instance
	HINSTANCE   hInstance;
	// main icon
	HICON       hIcon;
	// small icon
	HICON       hIconSmall;
	// font memory
	HANDLE      hMemFont;
	// log/edit font
	HFONT       hFont;
	// log direction image-list
	HIMAGELIST  hImageList;
	// log inject out (send) icon
	HICON       hInjectOut;
	// log inject in (recv) icon
	HICON       hInjectIn;
	// pause logging icon
	HICON       hPauseIcon;
	// resume logging icon
	HICON       hResumeIcon;
	// clear packets icon
	HICON       hClearIcon;
	// settings image list
	HIMAGELIST  hSettingsImageList;
	// array of color bitmaps
	HBITMAP     hColorIcon[COUNT_XPI_COLOR];
	// formatting image list
	HIMAGELIST  hFormattedImageList;
	// stop icon
	HICON       hStop;
} XPIGUI, far *LPXPIGUI, near *PXPIGUI;

// used to load windows into the main tab control
typedef struct _TAB_INFO
{
	std::wstring wstrCaption;
	/**/
	LPWSTR  lpwszTemplate;
	DLGPROC DlgProc;
} TAB_INFO, far *LPTAB_INFO, near *PTAB_INFO;

class CMainWindow
{
private:
	// window handle
	HWND    m_hDialog;
	// XPI GUI
	PXPIGUI m_pXPIGUI;
	// hide window (shell notify icon)
	BOOL    m_bHidden;
	// tooltips window
	HWND    m_hIconTips;
	// tab children
	std::vector<HWND> m_vhTabs;
	// shell notify icon data
	NOTIFYICONDATAW   m_NotifyIconData;

	// UI element coordinates
	enum
	{
		// tab control
		UI_TAB_X = 4,
		UI_TAB_Y = 4,
		UI_TAB_PAD_RIGHT = 4,
		UI_TAB_PAD_BOTTOM = 4,
		// tab view
		UI_PANE_X = 5,
		UI_PANE_Y = 26,
		UI_PANE_PAD_RIGHT = 8,
		UI_PANE_PAD_BOTTOM = 6,
		// pause button
		UI_PAUSE_WIDTH = 16,
		UI_PAUSE_HEIGHT = 16,
		UI_PAUSE_PADDING = 5,
		// clear button
		UI_CLEAR_WIDTH = 16,
		UI_CLEAR_HEIGHT = 16,
		UI_CLEAR_PADDING = 5
	};

	// shell notify icon menu item IDs
	enum
	{
		IDM_SHOW = 0,
		IDM_HIDE,
		IDM_EXIT
	};

protected:
	static HBITMAP MenuBitmapFromIcon(__in HWND hDialog, __in HICON hIcon, __in USHORT cx, __in USHORT cy);

public:
	explicit CMainWindow(__in HWND hDialog);
	~CMainWindow();

	BOOL OnCreate(__in HINSTANCE hInstance);
	VOID RelayToChildren(__in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
	VOID OnSize(__in WORD wWidth, __in WORD wHeight);
	VOID OnTabNotify(__in LPNMHDR lpnmhdr);
	VOID OnShellNotify(__in LPARAM lParam);
	VOID OnPauseNotify(__in WPARAM wParam);
	VOID OnClearNotify(__in WPARAM wParam);
	VOID Show(__in BOOL bShow);

	static VOID NotifyErrorUI();

	static INT_PTR CALLBACK DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
};

#endif // CMAIN_WINDOW_HPP_