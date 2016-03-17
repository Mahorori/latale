#include "stdafx.h"

#include "CPlainView.hpp"

#include <uxtheme.h>
#include <windowsx.h>

#include <boost/scoped_array.hpp>

#include "CResourceString.hpp"
#include "CInstanceManager.hpp"
#include "CPacketInfo.hpp"
#include "CSpamPacket.hpp"
#include "XPIUtilities.hpp"
#include "extvars.hpp"
#include "Hooks.h"
#include "resource.h"

CPlainView::CPlainView(__in HWND hDialog, __in PXPIGUI pXPIGUI)
{
	// initialize class data
	m_hDialog = hDialog;
	m_pXPIGUI = pXPIGUI;
	m_InjectDirection = PACKET_SEND;

	// set window data to hold class instance
	SetWindowLongPtrW(m_hDialog, GWLP_USERDATA, (LONG_PTR)this);
}

BOOL CPlainView::OnCreate()
{
	// set the dialog theme
	EnableThemeDialogTexture(m_hDialog, ETDT_ENABLETAB);

	HWND hCombo = GetDlgItem(m_hDialog, IDC_PLAINCOMBO);

	// sanity check
	if (hCombo == NULL)
		return FALSE;

	// combobox control text + font
	ComboBox_SetCueBannerText(hCombo, pStrings->Get(IDS_PLAIN_INJECT_CUE).c_str());
	SendMessageW(hCombo, WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	// no limit on text length
	ComboBox_LimitText(hCombo, 0);

	// combo box edit control mask
	HWND hComboEdit = FindWindowEx(hCombo, NULL, L"Edit", NULL);

	if (hComboEdit != NULL)
	{
		SetWindowLongW(hComboEdit, GWL_STYLE, GetWindowLongW(hComboEdit, GWL_STYLE) | ES_WANTRETURN);
		SetWindowLongPtrW(hComboEdit, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtrW(hComboEdit, GWLP_WNDPROC));
		SetWindowLongPtrW(hComboEdit, GWLP_WNDPROC, (LONG_PTR)CPlainView::MaskedEdit);
	}

	HWND hInject = GetDlgItem(m_hDialog, IDC_PLAININJECT);

	// sanity check
	if (hInject == NULL)
		return FALSE;

	// set button icon (inject send by default)
	SendMessageW(hInject, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectOut);

	HWND hListview = GetDlgItem(m_hDialog, IDC_PLAINLIST);

	// sanity check
	if (hListview == NULL)
		return FALSE;

	// initialize columns for list view
	LVCOLUMN  lvc;

	// FIRST COLUMN
	// - direction
	lvc.mask = LVCF_WIDTH;
	lvc.cx = 26;
	if (ListView_InsertColumn(hListview, LVC_DIRECTION, &lvc) == -1)
		return FALSE;

	std::wstring wstrTemp;

	// SECOND COLUMN
	// - size
	wstrTemp = pStrings->Get(IDS_SIZE);
	/***/
	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
	lvc.fmt = LVCFMT_CENTER;
	lvc.cx = 50;
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_SIZE, &lvc) == -1)
		return FALSE;

	// THIRD COLUMN
	// - data
	wstrTemp = pStrings->Get(IDS_DATA);
	/***/
	lvc.mask = LVCF_WIDTH | LVCF_TEXT;
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_DATA, &lvc) == -1)
		return FALSE;

	ListView_SetColumnWidth(hListview, LVC_DATA, LVSCW_AUTOSIZE_USEHEADER);

	// list view image list + font + styles
	ListView_SetImageList(hListview, m_pXPIGUI->hImageList, LVSIL_SMALL);
	SendMessageW(hListview, WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	ListView_SetExtendedListViewStyle(hListview, LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

	Button_Enable(hInject, FALSE);

	return TRUE;
}

VOID CPlainView::OnSize(__in WORD wHeight, __in WORD wWidth)
{
	// gather items to be resized
	HWND hCombo = GetDlgItem(m_hDialog, IDC_PLAINCOMBO);
	HWND hListview = GetDlgItem(m_hDialog, IDC_PLAINLIST);
	HWND hInject = GetDlgItem(m_hDialog, IDC_PLAININJECT);

	// sanity check
	if (hCombo == NULL || hListview == NULL || hInject == NULL)
		return;

	SetWindowPos(hListview, NULL, UI_LIST_X, UI_LIST_Y, wWidth - UI_LIST_PAD_RIGHT - UI_LIST_X, wHeight - UI_LIST_PAD_BOTTOM - UI_LIST_Y, 0);
	SetWindowPos(hCombo, NULL, UI_EDIT_X, wHeight - UI_EDIT_PAD_BOTTOM, wWidth - UI_EDIT_PAD_RIGHT - UI_EDIT_X, UI_EDIT_HEIGHT, 0);
	SetWindowPos(hInject, NULL, wWidth - UI_INJECT_PAD_RIGHT - UI_INJECT_WIDTH, wHeight - UI_INJECT_PAD_BOTTOM, UI_INJECT_WIDTH, UI_INJECT_HEIGHT, 0);
	ListView_SetColumnWidth(hListview, LVC_DATA, LVSCW_AUTOSIZE_USEHEADER);
}

VOID CPlainView::CopyPackets(__in BOOL bHeaders)
{
	std::vector<CMaplePacket*> vPackets;
	/***/
	LVITEM  lvi = { LVIF_PARAM, -1, 0 };
	HWND    hListview = GetDlgItem(m_hDialog, IDC_PLAINLIST);

	// sanity check
	if (hListview == NULL)
		return;

	// build up vector of selected packets
	while ((lvi.iItem = ListView_GetNextItem(hListview, lvi.iItem, LVNI_SELECTED)) != -1)
	{
		ListView_GetItem(hListview, &lvi);

		if (lvi.lParam == NULL)
			break;

		vPackets.push_back((CMaplePacket*)lvi.lParam);
	}

	// if there are no packets in the vector, return
	if (vPackets.empty())
		return;

	std::string strBuffer;
	/***/
	CHAR szTemp[(sizeof(UINT) * 2) + 1];

	// form string
	foreach(CMaplePacket* pPacket, vPackets)
	{
		if (bHeaders) // show packet headers (direction + size)
		{
			strBuffer += pPacket->GetDirection() == PACKET_SEND ? "->" : "<-";
			strBuffer += " || ";
			if (_itoa_s(pPacket->GetSize(), szTemp, _countof(szTemp), 10) == 0)
				strBuffer += szTemp;
			strBuffer += " || ";
		}

		// packet bytes
		foreach(BYTE b, *pPacket->GetData())
			if (SUCCEEDED(StringCchPrintfA(szTemp, _countof(szTemp), "%02X ", b)))
				strBuffer += szTemp;

		// trim last white-space
		strBuffer.erase(strBuffer.end() - 1, strBuffer.end());

		strBuffer += "\r\n";
	}

	// trim last line break
	strBuffer.erase(strBuffer.end() - 2, strBuffer.end());

	// begin copying to clipboard code
	HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, strBuffer.length() + 1);
	if (hData == NULL)
		return;

	LPVOID lpData = GlobalLock(hData);
	if (lpData == NULL)
	{
		GlobalFree(hData);
		return;
	}

	HRESULT hr = StringCchCopyA((LPSTR)lpData, strBuffer.length() + 1, strBuffer.c_str());

	GlobalUnlock(hData);

	do
	{
		if (SUCCEEDED(hr))
			if (OpenClipboard(m_hDialog))
				if (EmptyClipboard())
					if (SetClipboardData(CF_TEXT, hData) != NULL)
					{
						CloseClipboard();
						break;
					}
		GlobalFree(hData);
	} while (0);
}

VOID CPlainView::ListviewCustomDraw(__inout LPNMLVCUSTOMDRAW lpnmlvcd)
{
	// remove focus rect
	lpnmlvcd->nmcd.uItemState &= ~CDIS_FOCUS;

	switch (lpnmlvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
		return;

	case CDDS_ITEMPREPAINT:
	{
		CMaplePacket* pPacket;

		// ensure the packet has a linked CMaplePacket instance
		pPacket = (CMaplePacket*)lpnmlvcd->nmcd.lItemlParam;
		if (pPacket == NULL)
			break;

		// is the packet injected?
		if ((pPacket->GetState() & PACKET_INJECTED) == 0)
			break;

		// set red tint for injected packets
		lpnmlvcd->clrTextBk = RGB(255, 225, 225);
		SetBkColor(lpnmlvcd->nmcd.hdc, RGB(255, 225, 225));
		ListView_SetBkColor(GetDlgItem(m_hDialog, IDC_PLAINLIST), RGB(255, 225, 225));

		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT);
		return;
	}

	case CDDS_ITEMPOSTPAINT:
		// reset background colour
		lpnmlvcd->clrTextBk = RGB(255, 255, 255);
		SetBkColor(lpnmlvcd->nmcd.hdc, RGB(255, 255, 255));
		ListView_SetBkColor(GetDlgItem(m_hDialog, IDC_PLAINLIST), RGB(255, 255, 255));
		break;
	}

	SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
}

VOID CPlainView::ListviewNotify(__in LPNMHDR lpnmhdr)
{
	switch (lpnmhdr->code)
	{
	case HDN_ENDTRACK:
		ListView_SetColumnWidth(GetDlgItem(m_hDialog, IDC_PLAINLIST), LVC_DATA, LVSCW_AUTOSIZE_USEHEADER);
		break;

	case NM_CUSTOMDRAW:
		ListviewCustomDraw((LPNMLVCUSTOMDRAW)lpnmhdr);
		break;

	case NM_DBLCLK:
		ListviewDoubleClick((LPNMITEMACTIVATE)lpnmhdr);
		break;

	case NM_RCLICK:
		ListviewRightClick((LPNMITEMACTIVATE)lpnmhdr);
		break;

	case LVN_KEYDOWN:
		if (((LPNMLVKEYDOWN)lpnmhdr)->wVKey == 0x43 && GetAsyncKeyState(VK_CONTROL)) // ctrl+c
			CopyPackets(FALSE);
		break;
	}
}

VOID CPlainView::ListviewDoubleClick(__in LPNMITEMACTIVATE lpnmia)
{
	LVITEM  lvi = { LVIF_PARAM, lpnmia->iItem };
	HWND    hListview = GetDlgItem(m_hDialog, IDC_PLAINLIST);

	if (!ListView_GetItem(hListview, &lvi))
		return;

	CMaplePacket* pPacket = (CMaplePacket*)lvi.lParam;
	if (pPacket == NULL)
		return;

	// toggle display mode
	pPacket->SetLParam(!pPacket->GetLParam());

	std::wstring wstrBuffer;

	if (pPacket->GetLParam()) // ASCII mode
	{
		foreach(BYTE b, *pPacket->GetData())
		{
			wstrBuffer += isgraph((CHAR)b) == 0 && b != ' ' ? L'.' : b;
			wstrBuffer += L" ";
		}
	}
	else // HEX mode
	{
		WCHAR wszTemp[16];

		foreach(BYTE b, *pPacket->GetData())
			if (SUCCEEDED(StringCchPrintfW(wszTemp, _countof(wszTemp), L"%02X ", b)))
				wstrBuffer += wszTemp;
	}

	// trim last whitespace
	wstrBuffer.erase(wstrBuffer.end() - 1, wstrBuffer.end());

	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = LVC_DATA;
	lvi.pszText = (LPWSTR)wstrBuffer.c_str();

	ListView_SetItem(hListview, &lvi);
}

VOID CPlainView::ListviewRightClick(__in LPNMITEMACTIVATE lpnmia)
{
	HMENU hMenu = CreatePopupMenu();
	if (hMenu == NULL)
		return;

	CMaplePacket* pPacket = NULL;
	UINT          uCount = 0;

	if (lpnmia->iItem >= 0) do // if a packet is selected, show packet menu items
	{
		LVITEM lvi = { LVIF_PARAM, lpnmia->iItem, 0 };

		if (!ListView_GetItem(GetDlgItem(m_hDialog, IDC_PLAINLIST), &lvi))
			break;

		pPacket = (CMaplePacket*)lvi.lParam;

		if (pPacket == NULL)
			break;

		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_STRING, IDM_COPYPACKET, pStrings->Get(IDS_COPY_PACKET).c_str());
		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_STRING, IDM_COPYPACKETDATA, pStrings->Get(IDS_COPY_PACKET_DATA).c_str());
		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_STRING, IDM_PACKETINFO, pStrings->Get(IDS_PACKET_INFO).c_str());
		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

	} while (0);

	std::wstring  wstrAutoscroll = pStrings->Get(IDS_AUTOSCROLL);
	/***/
	MENUITEMINFOW mii = { sizeof(MENUITEMINFOW) };

	mii.fMask = MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_STRING | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED | (bAutoscroll ? MFS_CHECKED : MFS_UNCHECKED);
	mii.wID = IDM_AUTOSCROLL;
	mii.cch = wstrAutoscroll.length();
	mii.dwTypeData = (LPWSTR)wstrAutoscroll.c_str();

	InsertMenuItemW(hMenu, uCount, TRUE, &mii);

	POINT pt;

	if (GetCursorPos(&pt))
	{
		switch (TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, m_hDialog, NULL))
		{
		case IDM_COPYPACKET:
			// copy packet(s) with headers
			CopyPackets(TRUE);
			break;

		case IDM_COPYPACKETDATA:
			// copy packet(s) without headers
			CopyPackets(FALSE);
			break;

		case IDM_PACKETINFO:
			// show packet info window
			if (pPacket != NULL)
			{
				PPACKETINFO pPacketInfo = new PACKETINFO;
				pPacketInfo->pPacket = pPacket;
				pPacketInfo->pXPIGUI = m_pXPIGUI;
				ShowWindow(CreateDialogParamW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDD_PACKETINFO), m_hDialog, CPacketInfo::DialogProc, (LPARAM)pPacketInfo), SW_SHOW);
			}
			break;

		case IDM_AUTOSCROLL:
			bAutoscroll = !bAutoscroll;
			break;
		}
	}

	DestroyMenu(hMenu);
}

VOID CPlainView::OnInjectReady(__in BOOL bReady)
{
	HWND hInject = GetDlgItem(m_hDialog, IDC_PLAININJECT);

	// sanity check
	if (hInject == NULL)
		return;

	Button_Enable(hInject, bReady);
}

INT_PTR CALLBACK CPlainView::MaskedEdit(__in HWND hEdit, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_CHAR:
		// allow ctrl+<x> messages to pass through
		if (GetAsyncKeyState(VK_CONTROL))
			break;

		// if it's not a hexadecimal integer, block
		if (s_cbPlainMask[wParam & 0xFF] == FALSE)
			return FALSE;

		// set it to upper-case
		wParam = toupper(wParam);

		break;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
		{
			CPlainView* pPlainView = GetClassInstance<CPlainView>(GetParent(GetParent(hEdit)));

			if (pPlainView != NULL)
				pPlainView->OnInjectClick();
		}
		else if (wParam == VK_UP)
		{
			CPlainView* pPlainView = GetClassInstance<CPlainView>(GetParent(GetParent(hEdit)));

			if (pPlainView != NULL)
				pPlainView->ScrollCombo(1);
		}
		else if (wParam == VK_DOWN)
		{
			CPlainView* pPlainView = GetClassInstance<CPlainView>(GetParent(GetParent(hEdit)));

			if (pPlainView != NULL)
				pPlainView->ScrollCombo(-1);
		}
		else break;
		return 0;
	}

	WNDPROC WndProc = (WNDPROC)GetWindowLongPtrW(hEdit, GWLP_USERDATA);

	return WndProc != NULL ? CallWindowProcW(WndProc, hEdit, uMessage, wParam, lParam) : 0;
}

VOID CPlainView::AddToScrollBack(__in LPCWSTR lpcwszPacket, __in PACKET_DIRECTION Direction)
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_PLAINCOMBO);

	// sanity check
	if (hCombo == NULL)
		return;

	INT iIndex = ComboBox_FindStringExact(hCombo, -1, lpcwszPacket);

	if (iIndex != CB_ERR)
		ComboBox_DeleteString(hCombo, iIndex);

	iIndex = ComboBox_InsertString(hCombo, 0, lpcwszPacket);

	if (iIndex == CB_ERR || iIndex == CB_ERRSPACE)
		return;

	ComboBox_SetItemData(hCombo, iIndex, (LPARAM)Direction);
}

VOID CPlainView::OnInjectClick()
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_PLAINCOMBO);

	// sanity check
	if (hCombo == NULL)
		return;

	INT nLength = ComboBox_GetTextLength(hCombo);

	if (nLength <= 0)
	{
		HWND hComboEdit = FindWindowEx(hCombo, NULL, L"Edit", NULL);

		if (hComboEdit == NULL)
			return;

		std::wstring wstrTitle, wstrText;
		wstrTitle = wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
		wstrText = pStrings->Get(IDS_INJECT_ERROR_EMPTY);

		EDITBALLOONTIP  TipError;

		TipError.cbStruct = sizeof(EDITBALLOONTIP);
		TipError.ttiIcon = TTI_ERROR;
		TipError.pszTitle = wstrTitle.c_str();
		TipError.pszText = wstrText.c_str();
		Edit_ShowBalloonTip(hComboEdit, &TipError);

		return;
	}

	boost::scoped_array<WCHAR> wszBuffer(new WCHAR[nLength + 1]);

	// NOTE: no need to check this since we already do so with ComboBox_GetTextLength
	ComboBox_GetText(hCombo, wszBuffer.get(), nLength + 1);

	// convert string to byte array
	std::vector<BYTE> vbData;

	if (!StringToBuffer(wszBuffer.get(), &vbData))
	{
		HWND hComboEdit = FindWindowEx(hCombo, NULL, L"Edit", NULL);

		if (hComboEdit == NULL)
			return;

		std::wstring wstrTitle, wstrText;
		wstrTitle = wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
		wstrText = pStrings->Get(IDS_INJECT_ERROR_PARSE);

		EDITBALLOONTIP  TipError;

		TipError.cbStruct = sizeof(EDITBALLOONTIP);
		TipError.ttiIcon = TTI_ERROR;
		TipError.pszTitle = wstrTitle.c_str();
		TipError.pszText = wstrText.c_str();
		Edit_ShowBalloonTip(hComboEdit, &TipError);

		return;
	}

	// ensure there is at least an opcode
	if (vbData.size() < sizeof(WORD))
	{
		HWND hComboEdit = FindWindowEx(hCombo, NULL, L"Edit", NULL);

		if (hComboEdit == NULL)
			return;

		std::wstring wstrTitle, wstrText;
		wstrTitle = wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
		wstrText = pStrings->Get(IDS_INJECT_ERROR_SHORT);

		EDITBALLOONTIP  TipError;

		TipError.cbStruct = sizeof(EDITBALLOONTIP);
		TipError.ttiIcon = TTI_ERROR;
		TipError.pszTitle = wstrTitle.c_str();
		TipError.pszText = wstrText.c_str();
		Edit_ShowBalloonTip(hComboEdit, &TipError);

		return;
	}

	CMAPLEPACKETSTRUCT  cmps;
	CPacket				packet;

	cmps.pInstance = &packet;
	cmps.Direction = m_InjectDirection;
	cmps.ulState = PACKET_INJECTED;
	cmps.lpv = _ReturnAddress();

	pInstances->Add(&packet, pPacketPool->construct(&cmps));

	// encode bytes
	if (vbData.size() > sizeof(WORD))
		EncodeBuffer(&packet, 0, &vbData[0], vbData.size());

	if (m_InjectDirection == PACKET_SEND)
		SendPacket(pClientSocket, 0, &packet); // inject packet
	else
	{
		// inject packet
		ProcessPacket(&packet);
	}

	pInstances->Remove(&packet);

	// add text to scroll-back log
	AddToScrollBack(wszBuffer.get(), m_InjectDirection);

	// clear control
	ComboBox_SetText(hCombo, NULL);
}

VOID CPlainView::OnAddPacket(__inout CMaplePacket* pPacket)
{
	if (pPacket == NULL)
		return;

	if (pPacket->GetState() & PACKET_BLOCKED)
		return;

	if (pPacket->GetMemberCount() < 1)
		return;

	if ((*pOpcodeInfo)[pPacket->GetOpcode()].bIgnore && !(pPacket->GetState() & PACKET_INJECTED))
		return;

	pPacket->SetLParam(FALSE); // set initial state to HEX mode
	pPacket->SetWParam(0); // set initial state

	LVITEM  lvi;
	HWND    hListview = GetDlgItem(m_hDialog, IDC_PLAINLIST);

	// FIRST COLUMN
	// - direction
	lvi.mask = LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = ListView_GetItemCount(hListview);
	lvi.iSubItem = LVC_DIRECTION;
	lvi.lParam = (LPARAM)pPacket;
	if (pPacket->GetDirection() == PACKET_SEND)
		lvi.iImage = pPacket->GetState() & PACKET_LOOPBACK ? XPI_ILI_SENDLB : XPI_ILI_SEND;
	else // direction == PACKET_RECV
		lvi.iImage = pPacket->GetState() & PACKET_LOOPBACK ? XPI_ILI_RECVLB : XPI_ILI_RECV;

	if (ListView_InsertItem(hListview, &lvi) == -1)
		return;

	// SECOND COLUMN
	// - size
	WCHAR wszTemp[9];
	if (_itow_s(pPacket->GetSize(), wszTemp, _countof(wszTemp), 10) != 0)
		return;

	lvi.pszText = wszTemp;
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = LVC_SIZE;

	if (ListView_SetItem(hListview, &lvi) == -1)
		return;

	// THIRD COLUMN
	// - data
	std::wstring wstrBuffer;

	foreach(BYTE b, *pPacket->GetData())
		if (SUCCEEDED(StringCchPrintfW(wszTemp, _countof(wszTemp), L"%02X ", b)))
			wstrBuffer += wszTemp;

	// trim last whitespace
	wstrBuffer.erase(wstrBuffer.end() - 1, wstrBuffer.end());

	lvi.iSubItem = LVC_DATA;
	lvi.pszText = (LPWSTR)wstrBuffer.c_str();

	if (ListView_SetItem(hListview, &lvi) == -1)
		return;

	// do we auto-scroll?
	if (!bAutoscroll)
		return;

	RECT rc;

	if (!ListView_GetItemRect(hListview, 0, &rc, LVIR_BOUNDS))
		return;

	INT iHeight = rc.bottom - rc.top, iCount = ListView_GetItemCount(hListview);

	// NOTE: post the message instead of sending to ensure speed
	PostMessageW(hListview, LVM_SCROLL, 0, iHeight * (iCount + 1));
}

VOID CPlainView::ShowInjectMenu()
{
	HMENU hMenu = CreatePopupMenu();
	if (hMenu == NULL)
		return;

	MENUITEMINFOW mii;
	std::wstring  wstrTemp; // string to hold temporary menu strings

	// MENU ITEM #1
	// - "Send"
	wstrTemp = pStrings->Get(IDS_SEND);
	/***/
	mii.cbSize = sizeof(MENUITEMINFOW);
	mii.fMask = MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_STRING | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.wID = IDM_SEND;
	mii.fState = m_InjectDirection == PACKET_SEND ? MFS_DISABLED | MFS_CHECKED : MFS_ENABLED | MFS_UNCHECKED;
	mii.hbmpChecked = NULL;
	mii.hbmpUnchecked = NULL;
	mii.dwTypeData = (LPWSTR)wstrTemp.c_str();
	mii.cch = wstrTemp.length();

	INT iCount = 0;

	if (!InsertMenuItemW(hMenu, iCount++, TRUE, &mii))
	{
		DestroyMenu(hMenu);
		return;
	}

	// MENU ITEM #2
	// - "Receive"
	wstrTemp = pStrings->Get(IDS_RECEIVE);
	/***/
	mii.fState = m_InjectDirection == PACKET_RECV ? MFS_DISABLED | MFS_CHECKED : MFS_ENABLED | MFS_UNCHECKED;
	mii.wID = IDM_RECEIVE;
	mii.dwTypeData = (LPWSTR)wstrTemp.c_str();
	mii.cch = wstrTemp.length();

	if (!InsertMenuItemW(hMenu, iCount++, TRUE, &mii))
	{
		DestroyMenu(hMenu);
		return;
	}

	// MENU ITEM #3
	// - Separator
	mii.fMask = MIIM_FTYPE;
	mii.fType = MFT_SEPARATOR;

	if (!InsertMenuItemW(hMenu, iCount++, TRUE, &mii))
	{
		DestroyMenu(hMenu);
		return;
	}

	// MENU ITEM #4
	// - "Spam Packet"
	wstrTemp = pStrings->Get(IDS_SPAM_PACKET);
	/***/
	mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.wID = IDM_SPAMPACKET;
	mii.dwTypeData = (LPWSTR)wstrTemp.c_str();
	mii.cch = wstrTemp.length();

	if (!InsertMenuItemW(hMenu, iCount++, TRUE, &mii))
	{
		DestroyMenu(hMenu);
		return;
	}

	POINT pt;

	if (GetCursorPos(&pt))
	{
		switch (TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, m_hDialog, NULL))
		{
		case IDM_SEND:
			m_InjectDirection = PACKET_SEND;
			/***/
			SendMessageW(GetDlgItem(m_hDialog, IDC_PLAININJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectOut);
			break;

		case IDM_RECEIVE:
			m_InjectDirection = PACKET_RECV;
			/***/
			SendMessageW(GetDlgItem(m_hDialog, IDC_PLAININJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectIn);
			break;

		case IDM_SPAMPACKET:
			CreateSpamDialog();
			break;
		}
	}

	DestroyMenu(hMenu);
}

VOID CPlainView::CreateSpamDialog()
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_PLAINCOMBO);

	// sanity check
	if (hCombo == NULL)
		return;

	PSPAMPACKET pSpamPacket = new SPAMPACKET;
	INT         nLength = ComboBox_GetTextLength(hCombo);

	if (nLength > 0)
	{
		LPWSTR lpwszBuffer = new WCHAR[nLength + 1];

		// NOTE: no need to check this since we already do so with ComboBox_GetTextLength
		ComboBox_GetText(hCombo, lpwszBuffer, nLength + 1);

		// NOTE: make sure to release this data in the CSpamPacket
		pSpamPacket->lpcwszPacket = lpwszBuffer;
	}
	else
		pSpamPacket->lpcwszPacket = NULL;

	pSpamPacket->bFormatted = FALSE;
	pSpamPacket->Direction = m_InjectDirection;
	pSpamPacket->pXPIGUI = m_pXPIGUI;

	ComboBox_SetText(hCombo, NULL);

	ShowWindow(CreateDialogParamW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDD_SPAMPACKET), m_hDialog, CSpamPacket::DialogProc, (LPARAM)pSpamPacket), SW_SHOW);
}

VOID CPlainView::ClearAllPackets()
{
	ListView_DeleteAllItems(GetDlgItem(m_hDialog, IDC_PLAINLIST));
}

VOID CPlainView::OnComboChange()
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_PLAINCOMBO);

	// sanity check
	if (hCombo == NULL)
		return;

	INT iIndex = ComboBox_GetCurSel(hCombo);

	if (iIndex == CB_ERR)
		return;

	m_InjectDirection = (PACKET_DIRECTION)ComboBox_GetItemData(hCombo, iIndex);
	/***/
	SendMessageW(GetDlgItem(m_hDialog, IDC_PLAININJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)(m_InjectDirection == PACKET_SEND ? m_pXPIGUI->hInjectOut : m_pXPIGUI->hInjectIn));
}

VOID CPlainView::ScrollCombo(__in INT iOffset)
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_PLAINCOMBO);

	// sanity check
	if (hCombo == NULL)
		return;

	INT iIndex = ComboBox_GetCurSel(hCombo) + iOffset;

	if (iIndex < -1)
		return;

	ComboBox_SetCurSel(hCombo, iIndex);

	if (iIndex >= 0)
	{
		m_InjectDirection = (PACKET_DIRECTION)ComboBox_GetItemData(hCombo, iIndex);
		/***/
		SendMessageW(GetDlgItem(m_hDialog, IDC_PLAININJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)(m_InjectDirection == PACKET_SEND ? m_pXPIGUI->hInjectOut : m_pXPIGUI->hInjectIn));
	}
}

INT_PTR CALLBACK CPlainView::DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		if (!(new CPlainView(hDialog, (PXPIGUI)lParam))->OnCreate())
		{
			CMainWindow::NotifyErrorUI();
			EndDialog(GetParent(hDialog), EXIT_FAILURE);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_PLAINLIST)
			GetClassInstance<CPlainView>(hDialog)->ListviewNotify((LPNMHDR)lParam);
		else if (((LPNMHDR)lParam)->idFrom == IDC_PLAININJECT && ((LPNMHDR)lParam)->code == BCN_DROPDOWN)
			GetClassInstance<CPlainView>(hDialog)->ShowInjectMenu();
		break;

	case WM_SIZE:
		GetClassInstance<CPlainView>(hDialog)->OnSize(HIWORD(lParam), LOWORD(lParam));
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_PLAININJECT)
			GetClassInstance<CPlainView>(hDialog)->OnInjectClick();
		else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_PLAINCOMBO)
			GetClassInstance<CPlainView>(hDialog)->OnComboChange();
		break;

	case WM_ADDPACKET:
		GetClassInstance<CPlainView>(hDialog)->OnAddPacket((CMaplePacket*)lParam);
		break;

	case WM_CLEARPACKETS:
		GetClassInstance<CPlainView>(hDialog)->ClearAllPackets();
		break;

	case WM_INJECTREADY:
		GetClassInstance<CPlainView>(hDialog)->OnInjectReady((BOOL)wParam);
		break;

	case WM_DESTROY:
		delete GetClassInstance<CPlainView>(hDialog);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}
