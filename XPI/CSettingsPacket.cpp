#include "stdafx.h"

#include "CSettingsPacket.hpp"

#include <windowsx.h>
#include <uxtheme.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <vector>

#include "CResourceString.hpp"
#include "resource.h"
#include "XPIUtilities.hpp"
#include "extvars.hpp"
#include "XPIColors.h"

CSettingsPacket::CSettingsPacket(__in HWND hDialog, __in PXPIGUI pXPIGUI)
{
	// initialize class data
	m_hDialog = hDialog;
	m_pXPIGUI = pXPIGUI;
	m_pHeader = NULL;
	m_Sort = LVS_OPCODE_DESC;
	m_hEdit = NULL;
	m_DoubleClick = LVC_ALIAS;

	// set window data to hold class instance
	SetWindowLongPtrW(m_hDialog, GWLP_USERDATA, (LONG_PTR)this);
}

CSettingsPacket::~CSettingsPacket()
{
	// graphical clean-up
	if (m_pHeader != NULL)
		delete m_pHeader;
}

BOOL CSettingsPacket::OnCreate()
{
	// apply tab child style
	EnableThemeDialogTexture(m_hDialog, ETDT_ENABLETAB);

	HWND hListview = GetDlgItem(m_hDialog, IDC_OPCODELIST);

	// sanity check
	if (hListview == NULL)
		return FALSE;

	// initialize columns for list view
	LVCOLUMN  lvc;

	// FIRST COLUMN
	// - state
	lvc.mask = LVCF_WIDTH;
	lvc.cx = 26;
	if (ListView_InsertColumn(hListview, LVC_STATE, &lvc) == -1)
		return FALSE;

	std::wstring wstrTemp;

	// SECOND COLUMN
	// - opcode
	wstrTemp = pStrings->Get(IDS_OPCODE);
	/***/
	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
	lvc.fmt = LVCFMT_CENTER;
	lvc.cx = 60;
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_OPCODE, &lvc) == -1)
		return FALSE;

	// THIRD COLUMN
	// - ALIAS
	wstrTemp = pStrings->Get(IDS_ALIAS);
	/***/
	lvc.cx = 80;
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_ALIAS, &lvc) == -1)
		return FALSE;

	// FOURTH COLUMN
	// - comment
	wstrTemp = pStrings->Get(IDS_COMMENT);
	/***/
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_COMMENT, &lvc) == -1)
		return FALSE;

	ListView_SetColumnWidth(hListview, LVC_COMMENT, LVSCW_AUTOSIZE_USEHEADER);

	// list view image list + font + styles
	ListView_SetImageList(hListview, m_pXPIGUI->hSettingsImageList, LVSIL_SMALL);
	SendMessageW(hListview, WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	ListView_SetExtendedListViewStyle(hListview, LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

	// graphical header
	m_pHeader = new CHeader(m_hDialog);

	if (m_pHeader == NULL)
		return FALSE;

	// header text
	m_pHeader->SetTitle(pStrings->Get(IDS_SETTINGS_PACKET).c_str());
	m_pHeader->SetSubText(pStrings->Get(IDS_SETTINGS_PACKET_SUB).c_str());

	// header icon
	if (!m_pHeader->SetImageFromPNGResource(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDB_SETTINGS_PACKET)))
		return FALSE;

	// if any opcodes have been loaded from the configuration file, add them to the list
	foreach(OPCODE_MAP::iterator::value_type& i, *pOpcodeInfo)
		OnUpdateOpcode(i.first);

	return TRUE;
}

VOID CSettingsPacket::OnSize(__in WORD wHeight, __in WORD wWidth)
{
	SetWindowPos(GetDlgItem(m_hDialog, IDC_OPCODELIST), NULL, 0, 0, wWidth, wHeight - UI_LIST_SUBHEIGHT, SWP_NOMOVE);
}

VOID CSettingsPacket::OnPaint()
{
	PAINTSTRUCT ps;
	HDC         hDC;

	// start painting and grab device context
	hDC = BeginPaint(m_hDialog, &ps);

	// paint header
	if (hDC != NULL && m_pHeader != NULL)
		m_pHeader->Paint(hDC);

	// stop painting
	EndPaint(m_hDialog, &ps);
}

VOID CSettingsPacket::OnUpdateOpcode(__in DWORD dwOpcode)
{
	HWND hListview = GetDlgItem(m_hDialog, IDC_OPCODELIST);

	// sanity check
	if (hListview == NULL)
		return;

	LVFINDINFOW lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = dwOpcode;

	INT iIndex = ListView_FindItem(hListview, -1, &lvfi);

	if (iIndex == -1)
	{
		// add an opcode
		LVITEM lvi;

		// FIRST COLUMN
		// - state
		lvi.mask = LVIF_IMAGE | LVIF_PARAM;
		lvi.iItem = ListView_GetItemCount(hListview);
		lvi.iSubItem = LVC_STATE;
		lvi.lParam = dwOpcode;

		OPCODE_INFO* pTempOpcode = &(*pOpcodeInfo)[dwOpcode];
		if (pTempOpcode == NULL)
			return;

		if (pTempOpcode->bBlock)
			lvi.iImage = XPI_SILI_BLOCKED;
		else if (pTempOpcode->bIgnore)
			lvi.iImage = XPI_SILI_IGNORED;
		else
			lvi.iImage = XPI_SILI_NONE;

		if (ListView_InsertItem(hListview, &lvi) == -1)
			return;

		WCHAR wszOpcode[OP_ALIAS_MAXCC];

		if (FAILED(StringCchPrintfW(wszOpcode, _countof(wszOpcode), L"%08X", dwOpcode)))
			return;

		// SECOND COLUMN
		// - opcode
		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = LVC_OPCODE;
		lvi.pszText = wszOpcode;

		if (!ListView_SetItem(hListview, &lvi))
			return;

		// THIRD COLUMN
		// - alias
		lvi.iSubItem = LVC_ALIAS;
		lvi.pszText = pTempOpcode->wszAlias;

		if (!ListView_SetItem(hListview, &lvi))
			return;

		// FOURTH COLUMN
		// - comment
		lvi.iSubItem = LVC_COMMENT;
		lvi.pszText = pTempOpcode->wszComment;

		if (!ListView_SetItem(hListview, &lvi))
			return;

		ListView_SetColumnWidth(hListview, LVC_COMMENT, LVSCW_AUTOSIZE_USEHEADER);
	}
	else
	{
		// update an opcode
		OPCODE_INFO* pTempOpcode = &(*pOpcodeInfo)[dwOpcode];
		if (pTempOpcode == NULL)
			return;

		LVITEM lvi;
		lvi.mask = LVIF_IMAGE | LVIF_PARAM;
		lvi.iItem = iIndex;
		lvi.iSubItem = LVC_STATE;
		lvi.lParam = dwOpcode;

		if (pTempOpcode->bBlock)
			lvi.iImage = XPI_SILI_BLOCKED;
		else if (pTempOpcode->bIgnore)
			lvi.iImage = XPI_SILI_IGNORED;
		else
			lvi.iImage = XPI_SILI_NONE;

		ListView_SetItem(hListview, &lvi);

		ListView_SetItemText(hListview, iIndex, LVC_ALIAS, pTempOpcode->wszAlias);
	}
}

VOID CSettingsPacket::ListviewCustomDraw(__inout LPNMLVCUSTOMDRAW lpnmlvcd)
{
	// remove focus rect
	lpnmlvcd->nmcd.uItemState &= ~CDIS_FOCUS;

	switch (lpnmlvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
		return;

	case CDDS_ITEMPREPAINT:
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW);
		return;

	case (CDDS_SUBITEM | CDDS_ITEMPREPAINT) :
		// set opcode text color
		if (lpnmlvcd->iSubItem == LVC_OPCODE)
			lpnmlvcd->clrText = (*pOpcodeInfo)[(DWORD)lpnmlvcd->nmcd.lItemlParam].crColor;
		else
			lpnmlvcd->clrText = RGB(0, 0, 0);

		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NEWFONT);
		break;
	}

	SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
}

INT CALLBACK CSettingsPacket::ListviewSort(__in LPARAM lParam1, __in LPARAM lParam2, __in LPARAM lParamSort)
{
	UNREFERENCED_PARAMETER(lParamSort);

	// get the window's current listview sort
	CSettingsPacket::LV_ORDER Sort = GetClassInstance<CSettingsPacket>((HWND)lParamSort)->GetSort();

	// sort by opcode (numerical)
	if (Sort == LVS_OPCODE_ASC)
		return lParam1 - lParam2;
	else if (Sort == LVS_OPCODE_DESC)
		return lParam2 - lParam1;

	LVFINDINFOW lvfi;
	HWND        hOpcodeList = GetDlgItem((HWND)lParamSort, IDC_OPCODELIST);

	if (hOpcodeList == NULL)
		return 0;

	// get ready to sort
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = lParam1;
	INT iIndex1 = ListView_FindItem(hOpcodeList, -1, &lvfi);
	lvfi.lParam = lParam2;
	INT iIndex2 = ListView_FindItem(hOpcodeList, -1, &lvfi);

	// sort by state (index-numerical)
	if (Sort == LVS_STATE_ASC || Sort == LVS_STATE_DESC)
	{
		LVITEMW lvi1 = { LVIF_IMAGE, iIndex1, LVC_STATE };
		ListView_GetItem(hOpcodeList, &lvi1);
		LVITEMW lvi2 = { LVIF_IMAGE, iIndex2, LVC_STATE };
		ListView_GetItem(hOpcodeList, &lvi2);

		if (Sort == LVS_STATE_ASC)
			return lvi2.iImage - lvi1.iImage;
		else // Sort == LVS_STATE_DESC
			return lvi1.iImage - lvi2.iImage;
	}
	// sort by alias (alphabetical)
	else if (Sort == LVS_ALIAS_ASC || Sort == LVS_ALIAS_DESC)
	{
		if (pOpcodeInfo == NULL)
			return 0;

		if (Sort == LVS_ALIAS_ASC)
			return lstrcmpiW((*pOpcodeInfo)[(DWORD)lParam1].wszAlias, (*pOpcodeInfo)[(DWORD)lParam2].wszAlias);
		else // Sort == LVS_ALIAS_DESC
			return lstrcmpiW((*pOpcodeInfo)[(DWORD)lParam2].wszAlias, (*pOpcodeInfo)[(DWORD)lParam1].wszAlias);
	}
	// sort by comment (alphabetical)
	else if (Sort == LVS_COMMENT_ASC || Sort == LVS_COMMENT_DESC)
	{
		if (pOpcodeInfo == NULL)
			return 0;

		if (Sort == LVS_COMMENT_ASC)
			return lstrcmpiW((*pOpcodeInfo)[(DWORD)lParam1].wszComment, (*pOpcodeInfo)[(DWORD)lParam2].wszComment);
		else // Sort == LVS_COMMENT_DESC
			return lstrcmpiW((*pOpcodeInfo)[(DWORD)lParam2].wszComment, (*pOpcodeInfo)[(DWORD)lParam1].wszComment);
	}

	return 0;
}

VOID CSettingsPacket::ListviewColumnClick(__in LPNMLISTVIEW lpnmlv)
{
	switch (lpnmlv->iSubItem)
	{
	case LVC_STATE:
		if (m_Sort == LVS_STATE_ASC)
			m_Sort = LVS_STATE_DESC;
		else
			m_Sort = LVS_STATE_ASC;
		break;

	case LVC_OPCODE:
		if (m_Sort == LVS_OPCODE_ASC)
			m_Sort = LVS_OPCODE_DESC;
		else
			m_Sort = LVS_OPCODE_ASC;
		break;

	case LVC_ALIAS:
		if (m_Sort == LVS_ALIAS_ASC)
			m_Sort = LVS_ALIAS_DESC;
		else
			m_Sort = LVS_ALIAS_ASC;
		break;

	case LVC_COMMENT:
		if (m_Sort == LVS_COMMENT_ASC)
			m_Sort = LVS_COMMENT_DESC;
		else
			m_Sort = LVS_COMMENT_ASC;
		break;
	}

	ListView_SortItems(GetDlgItem(m_hDialog, IDC_OPCODELIST), CSettingsPacket::ListviewSort, (LPARAM)m_hDialog);
}

INT_PTR CALLBACK CSettingsPacket::ListviewEditProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	WNDPROC wpOld = (WNDPROC)GetWindowLongPtrW(hDialog, GWLP_USERDATA);

	if (uMessage == WM_SIZE)
	{
		CSettingsPacket::LV_COLUMNS DoubleClick = GetClassInstance<CSettingsPacket>(GetParent(GetParent(hDialog)))->GetDoubleClicked();
		/***/
		INT   iFocused;
		RECT  rcSubItem;

		iFocused = ListView_GetNextItem(GetParent(hDialog), -1, LVNI_FOCUSED);
		ListView_GetSubItemRect(GetParent(hDialog), iFocused, DoubleClick, LVIR_BOUNDS, &rcSubItem);
		SetWindowPos(hDialog, NULL, rcSubItem.left, rcSubItem.top, rcSubItem.right - rcSubItem.left, rcSubItem.bottom - rcSubItem.top, SWP_NOZORDER);
	}

	return CallWindowProc(wpOld, hDialog, uMessage, wParam, lParam);
}

VOID CSettingsPacket::ListviewBeginEdit(__in HWND* hEdit, __in NMLVDISPINFO* lpnmlvdi)
{
	UNREFERENCED_PARAMETER(lpnmlvdi);

	HWND hListview = GetDlgItem(m_hDialog, IDC_OPCODELIST);
	if (hListview == NULL)
		return;

	*hEdit = ListView_GetEditControl(hListview);
	if (*hEdit == NULL)
		return;

	SetWindowLongPtrW(*hEdit, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtrW(*hEdit, GWLP_WNDPROC));
	SetWindowLongPtrW(*hEdit, GWLP_WNDPROC, (LONG_PTR)CSettingsPacket::ListviewEditProc);

	if (m_DoubleClick == LVC_ALIAS)
	{
		SetWindowLong(*hEdit, GWL_STYLE, GetWindowLong(*hEdit, GWL_STYLE) | ES_CENTER | ES_UPPERCASE);
		Edit_LimitText(*hEdit, OP_ALIAS_MAXCC - 1);
	}
	else if (m_DoubleClick == LVC_COMMENT)
	{
		SetWindowLong(*hEdit, GWL_STYLE, GetWindowLong(*hEdit, GWL_STYLE) | ES_LEFT);
		Edit_LimitText(*hEdit, OP_COMMENT_MAXCC - 1);

		if (pOpcodeInfo != NULL)
		{
			WORD        wOpcode = (WORD)lpnmlvdi->item.lParam;
			LVFINDINFO  lvfi;

			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = wOpcode;

			INT iIndex = ListView_FindItem(hListview, -1, &lvfi);
			Edit_SetText(*hEdit, (*pOpcodeInfo)[wOpcode].wszComment);
		}
	}
}

VOID CSettingsPacket::ListviewEndEdit(__in HWND* hEdit, __in NMLVDISPINFO* lpnmlvdi)
{
	HWND hListview = GetDlgItem(m_hDialog, IDC_OPCODELIST);
	if (hListview == NULL)
		return;

	if (*hEdit == NULL)
		return;

	WORD  wOpcode = (WORD)lpnmlvdi->item.lParam;
	INT   nLength = Edit_GetTextLength(*hEdit);

	if (m_DoubleClick == LVC_ALIAS)
	{
		LPWSTR lpwszBuffer = (*pOpcodeInfo)[wOpcode].wszAlias;

		if (nLength > 0)
			Edit_GetText(*hEdit, lpwszBuffer, OP_ALIAS_MAXCC);
		else
			StringCchPrintfW(lpwszBuffer, OP_ALIAS_MAXCC, L"", wOpcode);

		PostMessage(GetParent(GetParent(m_hDialog)), WM_FORMATALIAS, (WPARAM)wOpcode, (LPARAM)lpwszBuffer);

		LVFINDINFO  lvfi;

		lvfi.flags = LVFI_PARAM;
		lvfi.lParam = wOpcode;

		INT iIndex = ListView_FindItem(hListview, -1, &lvfi);

		if (iIndex != -1)
			ListView_SetItemText(hListview, iIndex, LVC_ALIAS, lpwszBuffer);
	}
	else if (m_DoubleClick == LVC_COMMENT)
	{
		LPWSTR lpwszBuffer = (*pOpcodeInfo)[wOpcode].wszComment;

		Edit_GetText(*hEdit, lpwszBuffer, OP_COMMENT_MAXCC);

		LVFINDINFO lvfi;

		lvfi.flags = LVFI_PARAM;
		lvfi.lParam = wOpcode;

		INT iIndex = ListView_FindItem(hListview, -1, &lvfi);

		if (iIndex != -1)
			ListView_SetItemText(hListview, iIndex, LVC_COMMENT, lpwszBuffer);
	}

	*hEdit = NULL;
}

VOID CSettingsPacket::ListviewDoubleClick(__in LPNMITEMACTIVATE lpnmia)
{
	// do not respond if no item is selected or if the double click was outside of the alias column
	if (lpnmia->iItem < 0)
		return;

	m_DoubleClick = (CSettingsPacket::LV_COLUMNS)lpnmia->iSubItem;

	if (m_DoubleClick != LVC_ALIAS && m_DoubleClick != LVC_COMMENT)
		return;

	SendMessage(GetDlgItem(m_hDialog, IDC_OPCODELIST), LVM_EDITLABEL, lpnmia->iItem, 0);
}

VOID CSettingsPacket::ListviewRightClick(__in LPNMITEMACTIVATE lpnmia)
{
	HWND hListview = GetDlgItem(m_hDialog, IDC_OPCODELIST);
	if (hListview == NULL)
		return;

	// store all of the selected opcodes
	std::vector<DWORD> vdwSelected;
	/***/
	BOOL          bAllBlocked = TRUE;
	BOOL          bAllIgnored = TRUE;
	UINT          uCount = 0;

	LVITEM lvi = { LVIF_PARAM, -1, 0 };

	// populate selection vector
	while ((lvi.iItem = ListView_GetNextItem(hListview, lvi.iItem, LVNI_SELECTED)) != -1)
	{
		ListView_GetItem(hListview, &lvi);
		vdwSelected.push_back((DWORD)lvi.lParam);
		if (!(*pOpcodeInfo)[(DWORD)lvi.lParam].bBlock)
			bAllBlocked = FALSE;
		if (!(*pOpcodeInfo)[(DWORD)lvi.lParam].bIgnore)
			bAllIgnored = FALSE;
	}

	// stop if nothing is selected
	if (vdwSelected.empty())
		return;

	HMENU hMenu = CreatePopupMenu();
	if (hMenu == NULL)
		return;

	std::wstring wstrTemp;
	/***/
	MENUITEMINFOW mii = { sizeof(MENUITEMINFOW) };

	wstrTemp = pStrings->Get(IDS_IGNORE_PACKET);

	// menu - ignore
	mii.fMask = MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_STRING | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	if (vdwSelected.size() == 1)
		mii.fState = (*pOpcodeInfo)[vdwSelected.at(0)].bBlock ? MFS_DISABLED : (MFS_ENABLED | ((*pOpcodeInfo)[vdwSelected.at(0)].bIgnore ? MFS_CHECKED : MFS_UNCHECKED));
	else
		mii.fState = (bAllBlocked ? MFS_DISABLED : MFS_ENABLED) | (bAllIgnored ? MFS_CHECKED : MFS_UNCHECKED);
	mii.wID = IDM_IGNORE;
	mii.hbmpChecked = NULL;
	mii.hbmpUnchecked = NULL;
	mii.dwTypeData = (LPWSTR)wstrTemp.c_str();
	mii.cch = wstrTemp.length() + 1;

	InsertMenuItemW(hMenu, uCount++, TRUE, &mii);

	// menu - block
	wstrTemp = pStrings->Get(IDS_BLOCK_PACKET);

	if (vdwSelected.size() == 1)
		mii.fState = MFS_ENABLED | ((*pOpcodeInfo)[vdwSelected.at(0)].bBlock ? MFS_CHECKED : MFS_UNCHECKED);
	else
		mii.fState = MFS_ENABLED | (bAllBlocked ? MFS_CHECKED : MFS_UNCHECKED);
	mii.wID = IDM_BLOCK;
	mii.dwTypeData = (LPWSTR)wstrTemp.c_str();
	mii.cch = wstrTemp.length() + 1;

	InsertMenuItemW(hMenu, uCount++, TRUE, &mii);

	InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

	HMENU hColorMenu = CreatePopupMenu();
	if (hColorMenu == NULL)
	{
		DestroyMenu(hMenu);
		return;
	}

	mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_BITMAP;
	mii.fType = MFT_STRING;

	// color menu - load all colors
	for (INT i = 0; i < _countof(g_XPIColors); i++)
	{
		wstrTemp = pStrings->Get(g_XPIColors[i].wName);
		/***/
		mii.wID = IDM_COLOR + i;
		mii.hbmpItem = m_pXPIGUI->hColorIcon[i];
		mii.dwTypeData = (LPWSTR)wstrTemp.c_str();
		mii.cch = wstrTemp.length() + 1;

		InsertMenuItemW(hColorMenu, i, TRUE, &mii);
	}

	// menu - color
	wstrTemp = pStrings->Get(IDS_OPCODE_COLOR);
	/***/
	mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_SUBMENU;
	mii.fType = MFT_STRING;
	mii.hSubMenu = hColorMenu;
	mii.dwTypeData = (LPWSTR)wstrTemp.c_str();
	mii.cch = wstrTemp.length() + 1;

	InsertMenuItemW(hMenu, uCount++, TRUE, &mii);

	POINT pt;

	if (GetCursorPos(&pt))
	{
		ULONG ulMenuSelection = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, m_hDialog, NULL);

		switch (ulMenuSelection)
		{
		case IDM_IGNORE:
			if (vdwSelected.size() > 1)
			{
				foreach(DWORD& dwOpcode, vdwSelected)
				{
					(*pOpcodeInfo)[dwOpcode].bIgnore = !bAllIgnored;
					OnUpdateOpcode(dwOpcode);
				}
			}
			else
			{
				(*pOpcodeInfo)[vdwSelected.at(0)].bIgnore = !(*pOpcodeInfo)[vdwSelected.at(0)].bIgnore;
				OnUpdateOpcode(vdwSelected.at(0));
			}
			break;

		case IDM_BLOCK:
			if (vdwSelected.size() > 1)
			{
				foreach(DWORD& dwOpcode, vdwSelected)
				{
					(*pOpcodeInfo)[dwOpcode].bBlock = !bAllBlocked;
					if (!bAllBlocked)
						(*pOpcodeInfo)[dwOpcode].bIgnore = FALSE;
					OnUpdateOpcode(dwOpcode);
				}
			}
			else
			{
				if (!((*pOpcodeInfo)[vdwSelected.at(0)].bBlock = !(*pOpcodeInfo)[vdwSelected.at(0)].bBlock))
					(*pOpcodeInfo)[vdwSelected.at(0)].bIgnore = FALSE;
				else
					(*pOpcodeInfo)[vdwSelected.at(0)].bIgnore = TRUE;
				OnUpdateOpcode(vdwSelected.at(0));
			}
			break;

		default:
			if (ulMenuSelection >= IDM_COLOR && ulMenuSelection < IDM_COLOR + _countof(g_XPIColors))
				foreach(DWORD& dwOpcode, vdwSelected)
				(*pOpcodeInfo)[dwOpcode].crColor = g_XPIColors[ulMenuSelection - IDM_COLOR].crColor;
			break;
		}
	}

	DestroyMenu(hMenu);
}

VOID CSettingsPacket::ListviewNotify(__in LPNMHDR lpnmhdr)
{
	switch (lpnmhdr->code)
	{
	case HDN_ENDTRACK:
		ListView_SetColumnWidth(GetDlgItem(m_hDialog, IDC_OPCODELIST), LVC_COMMENT, LVSCW_AUTOSIZE_USEHEADER);
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

	case LVN_BEGINLABELEDIT:
		return ListviewBeginEdit(&m_hEdit, (NMLVDISPINFO*)lpnmhdr);

	case LVN_ENDLABELEDIT:
		return ListviewEndEdit(&m_hEdit, (NMLVDISPINFO*)lpnmhdr);

	case LVN_COLUMNCLICK:
		ListviewColumnClick((LPNMLISTVIEW)lpnmhdr);
		break;
	}
}

INT_PTR CALLBACK CSettingsPacket::DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		if (!(new CSettingsPacket(hDialog, (PXPIGUI)lParam))->OnCreate())
		{
			CMainWindow::NotifyErrorUI();
			EndDialog(GetParent(GetParent(hDialog)), EXIT_FAILURE);
		}
		break;

	case WM_UPDATEOPCODE:
		GetClassInstance<CSettingsPacket>(hDialog)->OnUpdateOpcode((DWORD)wParam);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_OPCODELIST)
			GetClassInstance<CSettingsPacket>(hDialog)->ListviewNotify((LPNMHDR)lParam);
		break;

	case WM_PAINT:
		GetClassInstance<CSettingsPacket>(hDialog)->OnPaint();
		break;

	case WM_SIZE:
		GetClassInstance<CSettingsPacket>(hDialog)->OnSize(HIWORD(lParam), LOWORD(lParam));
		break;

	case WM_DESTROY:
		delete GetClassInstance<CSettingsPacket>(hDialog);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}
