#include "stdafx.h"

#include "CMainWindow.hpp"

#include <windowsx.h>

#include "CInstanceManager.hpp"
#include "CResourceString.hpp"
#include "CHookManager.hpp"
#include "CLog.hpp"
#include "XPIUtilities.hpp"
#include "extvars.hpp"
#include "resource.h"

// tabs
#include "CPlainView.hpp"
#include "CFormattedView.hpp"
#include "CMainSettings.hpp"

// GUI libraries and manifests
#pragma  comment(lib, "uxtheme")
#pragma  comment(lib, "comctl32")
#pragma  comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

CMainWindow::CMainWindow(__in HWND hDialog)
{
	// initialize instance variables
	m_hDialog = hDialog;
	m_bHidden = FALSE;
	m_hIconTips = NULL;
	ZeroMemory(&m_NotifyIconData, sizeof(m_NotifyIconData));

	SetWindowLongPtrW(hDialog, GWLP_USERDATA, (LONG_PTR)this);
}

BOOL CMainWindow::OnCreate(__in HINSTANCE hInstance)
{
	// window caption
	SetWindowTextW(m_hDialog, pStrings->Get(IDS_XPI).c_str());

	// XPIGUI
	m_pXPIGUI = new XPIGUI;

	ZeroMemory(m_pXPIGUI, sizeof(XPIGUI));

	// XPIGUI - instance
	m_pXPIGUI->hInstance = hInstance;

	// XPIGUI - application icon
	m_pXPIGUI->hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_XPI));
	if (m_pXPIGUI->hIcon != NULL)
		SendMessageW(m_hDialog, WM_SETICON, ICON_BIG, (LPARAM)m_pXPIGUI->hIcon);
#ifdef _DEBUG
	else
		pLog->Write(LOG_WF_ERROR | LOG_WF_ECHODEBUG, L"Couldn't load XPI icon, continuing anyway. [LE=0x%08X]", GetLastError());
#endif

	// XPIGUI - small application icon
	m_pXPIGUI->hIconSmall = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_XPI_SMALL));
	if (m_pXPIGUI->hIconSmall != NULL)
		SendMessageW(m_hDialog, WM_SETICON, ICON_SMALL, (LPARAM)m_pXPIGUI->hIconSmall);
#ifdef _DEBUG
	else
		pLog->Write(LOG_WF_ERROR | LOG_WF_ECHODEBUG, L"Couldn't load small XPI icon, continuing anyway. [LE=0x%08X]", GetLastError());
#endif

	// XPIGUI - load Consolas from resource
	m_pXPIGUI->hMemFont = AddFontFromResource(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDR_CONSOLAS));
	if (m_pXPIGUI->hMemFont == NULL)
	{
	}

	// XPIGUI - Consolas font handle
	m_pXPIGUI->hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, L"Consolas");
	if (m_pXPIGUI->hFont == NULL)
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't create log font, aborting. [LE=0x%08X]", GetLastError());
#endif
		NotifyErrorUI();
		return FALSE;
	}

	// XPIGUI - direction image list
	do
	{
		m_pXPIGUI->hImageList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 4, 0);
		ImageList_SetBkColor(m_pXPIGUI->hImageList, CLR_NONE);
		if (m_pXPIGUI->hImageList != NULL)
		{
			HBITMAP hBitmap = LoadBitmapW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDB_DIRECTIONS));
			if (hBitmap != NULL)
			{
				if (ImageList_AddMasked(m_pXPIGUI->hImageList, hBitmap, RGB(255, 0, 255)) != -1)
					break;
				DeleteObject((HGDIOBJ)hBitmap);
			}
		}

#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't create packet direction icons image-list, aborting.");
#endif
		NotifyErrorUI();
		return FALSE;
	} while (0);

	// XPIGUI - injection icons
	m_pXPIGUI->hInjectOut = (HICON)LoadImageW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDI_INJECT_OUT), IMAGE_ICON, 16, 16, 0);
	if (m_pXPIGUI->hInjectOut == NULL)
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't load inject OUT icon, aborting. [LE=0x%08X]", GetLastError());
#endif
		NotifyErrorUI();
		return FALSE;
	}
	m_pXPIGUI->hInjectIn = (HICON)LoadImageW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDI_INJECT_IN), IMAGE_ICON, 16, 16, 0);
	if (m_pXPIGUI->hInjectIn == NULL)
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't load inject IN icon, aborting. [LE=0x%08X]", GetLastError());
#endif
		NotifyErrorUI();
		return FALSE;
	}

	// XPIGUI - pause/resume icons
	m_pXPIGUI->hPauseIcon = (HICON)LoadImageW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDI_PAUSE), IMAGE_ICON, 16, 16, 0);
	if (m_pXPIGUI->hPauseIcon == NULL)
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't load pause icon, aborting. [LE=0x%08X]", GetLastError());
#endif
		NotifyErrorUI();
		return FALSE;
	}
	Static_SetIcon(GetDlgItem(m_hDialog, IDC_PAUSELOGGING), m_pXPIGUI->hPauseIcon);
	m_pXPIGUI->hResumeIcon = (HICON)LoadImageW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDI_RESUME), IMAGE_ICON, 16, 16, 0);
	if (m_pXPIGUI->hResumeIcon == NULL)
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't load resume icon, aborting. [LE=0x%08X]", GetLastError());
#endif
		NotifyErrorUI();
		return FALSE;
	}

	RECT rcClient;
	GetClientRect(m_hDialog, &rcClient);
	SetWindowPos(GetDlgItem(m_hDialog, IDC_PAUSELOGGING), NULL, rcClient.right - rcClient.left - UI_PAUSE_WIDTH - UI_PAUSE_PADDING, UI_PAUSE_PADDING, UI_PAUSE_WIDTH, UI_PAUSE_HEIGHT, 0);

	// XPIGUI - clear packets icon
	m_pXPIGUI->hClearIcon = (HICON)LoadImageW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDI_CLEAR), IMAGE_ICON, 16, 16, 0);
	if (m_pXPIGUI->hClearIcon == NULL)
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't load clear icon, aborting. [LE=0x%08X]", GetLastError());
#endif
		NotifyErrorUI();
		return FALSE;
	}
	Static_SetIcon(GetDlgItem(m_hDialog, IDC_CLEAR), m_pXPIGUI->hClearIcon);
	SetWindowPos(GetDlgItem(m_hDialog, IDC_CLEAR), NULL, rcClient.right - rcClient.left - UI_CLEAR_WIDTH - (UI_CLEAR_PADDING * 2) - UI_PAUSE_WIDTH, UI_CLEAR_PADDING, UI_CLEAR_WIDTH, UI_CLEAR_HEIGHT, 0);

	// XPIGUI - settings image list
	do
	{
		m_pXPIGUI->hSettingsImageList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 4, 0);
		ImageList_SetBkColor(m_pXPIGUI->hSettingsImageList, CLR_NONE);
		if (m_pXPIGUI->hSettingsImageList != NULL)
		{
			HBITMAP hBitmap = LoadBitmapW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDB_PACKET_STATES));
			if (hBitmap != NULL)
			{
				if (ImageList_AddMasked(m_pXPIGUI->hSettingsImageList, hBitmap, RGB(255, 0, 255)) != -1)
					break;
				DeleteObject((HGDIOBJ)hBitmap);
			}
		}

#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't create settings image-list, aborting.");
#endif
		NotifyErrorUI();
		return FALSE;
	} while (0);

	// XPIGUI - color icons
	for (INT i = 0; i < _countof(m_pXPIGUI->hColorIcon); i++)
	{
		HICON hTempIcon = LoadIcon(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(g_XPIColors[i].wIcon));
		m_pXPIGUI->hColorIcon[i] = MenuBitmapFromIcon(m_hDialog, hTempIcon, 16, 16);
		if (m_pXPIGUI->hColorIcon[i] == NULL)
		{
#ifdef _DEBUG
			pLog->Write(LOG_WF_ERROR, L"Couldn't load color icon #%d, aborting. [LE=0x%08X]", i, GetLastError());
#endif
			NotifyErrorUI();
			return FALSE;
		}
		DestroyIcon(hTempIcon);
	}

	// XPIGUI - formatted image list
	do
	{
		m_pXPIGUI->hFormattedImageList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 4, 0);
		if (m_pXPIGUI->hFormattedImageList != NULL)
		{
			ImageList_SetBkColor(m_pXPIGUI->hFormattedImageList, CLR_NONE);

			HBITMAP hBitmap = LoadBitmapW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDB_FORMATTED_LIST));
			if (hBitmap != NULL)
			{
				if (ImageList_AddMasked(m_pXPIGUI->hFormattedImageList, hBitmap, RGB(255, 0, 255)) != -1)
					break;
				DeleteObject((HGDIOBJ)hBitmap);
			}
		}

#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't create formatted image-list, aborting.");
#endif
		NotifyErrorUI();
		return FALSE;
	} while (0);

	// XPIGUI - stop icon
	m_pXPIGUI->hStop = (HICON)LoadImageW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDI_STOP), IMAGE_ICON, 16, 16, 0);
	if (m_pXPIGUI->hStop == NULL)
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't load stop icon, aborting. [LE=0x%08X]", GetLastError());
#endif
		NotifyErrorUI();
		return FALSE;
	}

	// shell notify icon
	ZeroMemory(&m_NotifyIconData, sizeof(m_NotifyIconData));
	m_NotifyIconData.cbSize = sizeof(m_NotifyIconData);
	m_NotifyIconData.hWnd = m_hDialog;
	m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_NotifyIconData.hIcon = m_pXPIGUI->hIconSmall;
	m_NotifyIconData.uCallbackMessage = WM_SHELLNOTIFY;
	if (FAILED(StringCchCopyW(m_NotifyIconData.szTip, _countof(m_NotifyIconData.szTip), pStrings->Get(IDS_XPI).c_str())))
		m_NotifyIconData.uFlags &= ~NIF_TIP;
	Shell_NotifyIconW(NIM_ADD, &m_NotifyIconData);

	// tooltips window (for icons)
	m_hIconTips = CreateWindowEx(0, TOOLTIPS_CLASS, L"", TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hDialog, 0, m_pXPIGUI->hInstance, NULL);
	SetWindowPos(m_hIconTips, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti;
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = m_hDialog;

	std::wstring wstrTemp;

	wstrTemp = pStrings->Get(IDS_CLEAR_ALL_PACKETS);
	ti.uId = (UINT_PTR)GetDlgItem(m_hDialog, IDC_CLEAR);
	ti.lpszText = (LPWSTR)wstrTemp.c_str();
	SendMessage(m_hIconTips, TTM_ADDTOOL, 0, (LPARAM)&ti);

	wstrTemp = pStrings->Get(IDS_PAUSE_LOGGING);
	ti.uId = (UINT_PTR)GetDlgItem(m_hDialog, IDC_PAUSELOGGING);
	ti.lpszText = (LPWSTR)wstrTemp.c_str();
	SendMessage(m_hIconTips, TTM_ADDTOOL, 0, (LPARAM)&ti);

	// tabs creation
	TAB_INFO  Tabs[] =
	{
		{ pStrings->Get(IDS_TAB_PLAIN), MAKEINTRESOURCEW(IDD_PLAINVIEW), CPlainView::DialogProc },
		{ pStrings->Get(IDS_TAB_FORMATTED), MAKEINTRESOURCEW(IDD_FORMATTEDVIEW), CFormattedView::DialogProc },
		{ pStrings->Get(IDS_TAB_SETTINGS), MAKEINTRESOURCEW(IDD_SETTINGS), CMainSettings::DialogProc }
	};

	INT iCount = 0;

	foreach(const TAB_INFO& i, Tabs)
	{
		HWND hWnd = CreateDialogParamW(m_pXPIGUI->hInstance, i.lpwszTemplate, m_hDialog, i.DlgProc, (LPARAM)m_pXPIGUI);
		if (hWnd != NULL)
		{
			TCITEMW tci;
			tci.mask = TCIF_TEXT | TCIF_PARAM;
			tci.pszText = (LPWSTR)i.wstrCaption.c_str();
			tci.lParam = (LPARAM)m_pXPIGUI;

			if (TabCtrl_InsertItem(GetDlgItem(m_hDialog, IDC_TAB), iCount++, &tci) != -1)
			{
				ShowWindow(hWnd, m_vhTabs.empty() ? SW_SHOW : SW_HIDE);
				m_vhTabs.push_back(hWnd);
				continue;
			}
		}

#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't create tab ID %s, aborting.", i.wstrCaption);
#endif
		NotifyErrorUI();
		return FALSE;
	}

	return TRUE;
}

CMainWindow::~CMainWindow()
{
	// shut down the user interface

	if (m_pXPIGUI == NULL)
		return;

	if (m_pXPIGUI->hInstance == NULL)
		return;

	Shell_NotifyIconW(NIM_DELETE, &m_NotifyIconData);

#ifdef _DEBUG
	pLog->Write(LOG_WF_DEBUG, L"Freeing all graphical resources.");
#endif

	if (m_pXPIGUI->hIcon != NULL)
		DestroyIcon(m_pXPIGUI->hIcon);

	if (m_pXPIGUI->hIconSmall != NULL)
		DestroyIcon(m_pXPIGUI->hIconSmall);

	if (m_pXPIGUI->hMemFont != NULL)
		RemoveFontMemResourceEx(m_pXPIGUI->hMemFont);

	if (m_pXPIGUI->hFont != NULL)
		DeleteObject((HGDIOBJ)m_pXPIGUI->hFont);

	if (m_pXPIGUI->hImageList != NULL)
		ImageList_Destroy(m_pXPIGUI->hImageList);

	if (m_pXPIGUI->hInjectOut != NULL)
		DestroyIcon(m_pXPIGUI->hInjectOut);

	if (m_pXPIGUI->hInjectIn != NULL)
		DestroyIcon(m_pXPIGUI->hInjectIn);

	if (m_pXPIGUI->hPauseIcon != NULL)
		DestroyIcon(m_pXPIGUI->hPauseIcon);

	if (m_pXPIGUI->hResumeIcon != NULL)
		DestroyIcon(m_pXPIGUI->hResumeIcon);

	if (m_pXPIGUI->hClearIcon != NULL)
		DestroyIcon(m_pXPIGUI->hClearIcon);

	if (m_pXPIGUI->hSettingsImageList != NULL)
		ImageList_Destroy(m_pXPIGUI->hSettingsImageList);

	for (INT i = 0; i < _countof(m_pXPIGUI->hColorIcon); i++)
		if (m_pXPIGUI->hColorIcon[i] != NULL)
			DeleteObject((HGDIOBJ)m_pXPIGUI->hColorIcon[i]);

	if (m_pXPIGUI->hFormattedImageList != NULL)
		ImageList_Destroy(m_pXPIGUI->hFormattedImageList);

	if (m_pXPIGUI->hStop != NULL)
		DestroyIcon(m_pXPIGUI->hStop);

	delete m_pXPIGUI;
}

VOID CMainWindow::NotifyErrorUI()
{
	MessageBoxW(NULL, pStrings->Get(IDS_UI_ERROR_MSG).c_str(), pStrings->Get(IDS_ERROR_TITLE).c_str(), MB_OK | MB_ICONSTOP);
}

VOID CMainWindow::RelayToChildren(__in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	foreach(HWND hWnd, m_vhTabs)
		PostMessage(hWnd, uMessage, wParam, lParam);
}

VOID CMainWindow::OnSize(__in WORD wWidth, __in WORD wHeight)
{
	SetWindowPos(GetDlgItem(m_hDialog, IDC_TAB), NULL, UI_TAB_X, UI_TAB_Y, wWidth - UI_TAB_PAD_RIGHT - UI_TAB_X, wHeight - UI_TAB_PAD_BOTTOM - UI_TAB_X, 0);
	SetWindowPos(GetDlgItem(m_hDialog, IDC_PAUSELOGGING), NULL, wWidth - UI_PAUSE_WIDTH - UI_PAUSE_PADDING, UI_PAUSE_PADDING, UI_PAUSE_WIDTH, UI_PAUSE_HEIGHT, 0);
	SetWindowPos(GetDlgItem(m_hDialog, IDC_CLEAR), NULL, wWidth - UI_PAUSE_WIDTH - UI_PAUSE_PADDING - UI_CLEAR_WIDTH - UI_CLEAR_PADDING, UI_CLEAR_PADDING, UI_CLEAR_WIDTH, UI_CLEAR_HEIGHT, 0);

	// resize every child window
	foreach(HWND hWnd, m_vhTabs)
		SetWindowPos(hWnd, NULL, UI_PANE_X, UI_PANE_Y, wWidth - UI_PANE_PAD_RIGHT - UI_PANE_X, wHeight - UI_PANE_PAD_BOTTOM - UI_PANE_Y, 0);
}

VOID CMainWindow::OnTabNotify(__in LPNMHDR lpnmhdr)
{
	if (lpnmhdr->code != TCN_SELCHANGE)
		return;

	INT iSelectedTab = TabCtrl_GetCurSel(GetDlgItem(m_hDialog, IDC_TAB));

	if (iSelectedTab > -1)
	{
		// hide every tab window
		foreach(HWND hWnd, m_vhTabs)
			ShowWindow(hWnd, SW_HIDE);
		// show the selected tab window
		ShowWindow(m_vhTabs.at(iSelectedTab), SW_SHOW);
	}
}

VOID CMainWindow::OnShellNotify(__in LPARAM lParam)
{
	if (lParam == WM_LBUTTONDOWN)
	{
		m_bHidden = !m_bHidden;
		ShowWindow(m_hDialog, m_bHidden ? SW_HIDE : SW_SHOW);
		return;
	}

	if (lParam != WM_CONTEXTMENU && lParam != WM_RBUTTONDOWN)
		return;

	HMENU hMenu = CreatePopupMenu();
	if (hMenu == NULL)
		return;

	INT iMenuCount = 0;

	// MEU ITEM #1 - "XPI"
	InsertMenu(hMenu, iMenuCount++, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, pStrings->Get(IDS_XPI).c_str());

	// MENU ITEM #2 - separator
	InsertMenu(hMenu, iMenuCount++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

	// MENU ITEM #3 - "show"/"hide"
	if (m_bHidden)
		InsertMenu(hMenu, iMenuCount++, MF_BYPOSITION | MF_STRING, IDM_SHOW, pStrings->Get(IDS_SHOW).c_str());
	else
		InsertMenu(hMenu, iMenuCount++, MF_BYPOSITION | MF_STRING, IDM_HIDE, pStrings->Get(IDS_HIDE).c_str());

	// MENU ITEM #4 - "exit"
	InsertMenu(hMenu, iMenuCount++, MF_BYPOSITION | MF_STRING, IDM_EXIT, pStrings->Get(IDS_EXIT).c_str());

	POINT Location;

	if (!GetCursorPos(&Location))
	{
		DestroyMenu(hMenu);
		return;
	}

	switch (TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_BOTTOMALIGN, Location.x, Location.y, 0, m_hDialog, NULL))
	{
	case IDM_SHOW:
		Show(TRUE);
		break;

	case IDM_HIDE:
		Show(FALSE);
		break;

	case IDM_EXIT:
		SendMessage(m_hDialog, WM_CLOSE, 0, 0);
		break;
	}

	DestroyMenu(hMenu);
}

VOID CMainWindow::OnPauseNotify(__in WPARAM wParam)
{
	if (HIWORD(wParam) != STN_CLICKED)
		return;

	bLogging = !bLogging;

	Static_SetIcon(GetDlgItem(m_hDialog, IDC_PAUSELOGGING), bLogging ? m_pXPIGUI->hPauseIcon : m_pXPIGUI->hResumeIcon);

	// if any packet is in the process of being parsed by hooks, disregard them
	if (!bLogging)
		pInstances->Clear();
}

VOID CMainWindow::OnClearNotify(__in WPARAM wParam)
{
	if (HIWORD(wParam) != STN_CLICKED)
		return;

	if (MessageBoxIcon(m_hDialog, pStrings->Get(IDS_CLEAR_INSTRUCTION).c_str(), pStrings->Get(IDS_CLEAR_TITLE).c_str(), MB_OKCANCEL, m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDI_CLEAR_LARGE)) == IDOK)
	{
		RelayToChildren(WM_CLEARPACKETS, 0, 0);

		// disable hooks
		BOOL bSetHooks = pHookManager->Disable();

		// clear list of running instances
		pInstances->Clear();

		// create a temporary pool, so there's never a moment where the packets are being added to a NULL pool
		boost::object_pool<CMaplePacket>* pTempPool = new boost::object_pool<CMaplePacket>, *pOldPool = pPacketPool;
		pPacketPool = pTempPool;
		delete pOldPool;

		lPacketCount = 0;

		// reset hooks
		if (bSetHooks)
			pHookManager->Install();
	}
}

VOID CMainWindow::Show(__in BOOL bShow)
{
	m_bHidden = !bShow;
	ShowWindow(m_hDialog, bShow ? SW_SHOW : SW_HIDE);
}

HBITMAP CMainWindow::MenuBitmapFromIcon(__in HWND hDialog, __in HICON hIcon, __in USHORT cx, __in USHORT cy)
{
	// converts an icon into a transparent popup-menu bitmap

	HDC hDC, hMemoryDC;

	// sanity check
	if (hIcon == NULL || hDialog == NULL)
		return NULL;

	hDC = GetDC(hDialog);
	if (hDC == NULL)
		return NULL;

	hMemoryDC = CreateCompatibleDC(hDC);
	if (hMemoryDC == NULL)
	{
		DeleteDC(hDC);
		return NULL;
	}

	HBITMAP hBitmap = CreateCompatibleBitmap(hDC, 16, 16);

	if (hBitmap != NULL)
	{
		if (SelectObject(hMemoryDC, hBitmap) != NULL)
		{
			RECT rc = { 0, 0, cx, cy };

			if (FillRect(hMemoryDC, &rc, GetSysColorBrush(COLOR_MENU)) != 0)
				DrawIconEx(hMemoryDC, 0, 0, hIcon, cx, cy, 0, 0, DI_NORMAL);
		}
	}

	DeleteDC(hMemoryDC);
	ReleaseDC(hDialog, hDC);

	return hBitmap;
}

INT_PTR CALLBACK CMainWindow::DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		if (!(new CMainWindow(hDialog))->OnCreate((HINSTANCE)lParam))
			EndDialog(hDialog, EXIT_FAILURE);
		break;

	case WM_UPDATEOPCODE:
	case WM_FORMATALIAS:
	case WM_INJECTREADY:
	case WM_ADDPACKET:
		GetClassInstance<CMainWindow>(hDialog)->RelayToChildren(uMessage, wParam, lParam);
		break;

	case WM_SIZE:
		GetClassInstance<CMainWindow>(hDialog)->OnSize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PAUSELOGGING:
			GetClassInstance<CMainWindow>(hDialog)->OnPauseNotify(wParam);
			break;

		case IDC_CLEAR:
			GetClassInstance<CMainWindow>(hDialog)->OnClearNotify(wParam);
			break;
		}
		break;

	case WM_SHELLNOTIFY:
		GetClassInstance<CMainWindow>(hDialog)->OnShellNotify(lParam);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_TAB)
			GetClassInstance<CMainWindow>(hDialog)->OnTabNotify((LPNMHDR)lParam);
		break;

	case WM_CLOSE:
		EndDialog(hDialog, EXIT_SUCCESS);
		break;

	case WM_DESTROY:
		delete GetClassInstance<CMainWindow>(hDialog);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}