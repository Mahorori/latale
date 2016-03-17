#include "stdafx.h"

#include "CFormattedView.hpp"
#include "CLog.hpp"

#include <uxtheme.h>
#include <windowsx.h>
#include <set>

#include <boost/scoped_array.hpp>

#include "FormattedInject.hpp"
#include "CResourceString.hpp"
#include "CInstanceManager.hpp"
#include "CPacketInfo.hpp"
#include "CSpamPacket.hpp"
#include "XPIUtilities.hpp"
#include "extvars.hpp"
#include "Hooks.h"
#include "resource.h"

CFormattedView::CFormattedView(__in HWND hDialog, __in PXPIGUI pXPIGUI)
{
	// initialize class data
	m_hDialog = hDialog;
	m_pXPIGUI = pXPIGUI;
	m_InjectDirection = PACKET_SEND;
	m_iTooltipsSel = -1;
	m_hEdit = NULL;

	// set window data to hold class instance
	SetWindowLongPtrW(m_hDialog, GWLP_USERDATA, (LONG_PTR)this);
}

BOOL CFormattedView::OnCreate()
{
	// set the dialog theme
	EnableThemeDialogTexture(m_hDialog, ETDT_ENABLETAB);

	HWND hCombo = GetDlgItem(m_hDialog, IDC_FORMATTEDCOMBO);

	// sanity check
	if (hCombo == NULL)
		return FALSE;

	// combobox control text + font
	ComboBox_SetCueBannerText(hCombo, pStrings->Get(IDS_FORMAT_INJECT_CUE).c_str());
	SendMessageW(hCombo, WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	// no limit on text length
	ComboBox_LimitText(hCombo, 0);

	// combo box edit control mask
	HWND hComboEdit = FindWindowEx(hCombo, NULL, L"Edit", NULL);

	if (hComboEdit != NULL)
	{
		SetWindowLongW(hComboEdit, GWL_STYLE, GetWindowLongW(hComboEdit, GWL_STYLE) | ES_WANTRETURN);
		SetWindowLongPtrW(hComboEdit, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtrW(hComboEdit, GWLP_WNDPROC));
		SetWindowLongPtrW(hComboEdit, GWLP_WNDPROC, (LONG_PTR)CFormattedView::MaskedEdit);
	}

	HWND hInject = GetDlgItem(m_hDialog, IDC_FORMATTEDINJECT);

	// sanity check
	if (hInject == NULL)
		return FALSE;

	// set button icon (inject send by default)
	SendMessageW(hInject, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectOut);

	HWND hListview = GetDlgItem(m_hDialog, IDC_FORMATTEDLIST);

	// sanity check
	if (hListview == NULL)
		return FALSE;

	// initialize columns for list view
	LVCOLUMN lvc;

	// FIRST COLUMN
	// - direction
	lvc.mask = LVCF_WIDTH;
	lvc.cx = 26;
	if (ListView_InsertColumn(hListview, LVC_DIRECTION, &lvc) == -1)
		return FALSE;

	std::wstring wstrTemp;

	// SECOND COLUMN
	// - size
	wstrTemp = pStrings->Get(IDS_OPCODE);
	/***/
	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
	lvc.fmt = LVCFMT_CENTER;
	lvc.cx = 80;
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_OPCODE, &lvc) == -1)
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

	// tooltips
	m_hTooltips = CreateWindowEx(0, TOOLTIPS_CLASS, L"", TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetParent(m_hDialog), 0, m_pXPIGUI->hInstance, NULL);

	SetWindowPos(m_hTooltips, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO  ti;
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = m_hDialog;
	ti.uId = (UINT_PTR)hListview;
	ti.lpszText = NULL;

	SendMessage(m_hTooltips, TTM_ADDTOOL, 0, (LPARAM)&ti);
	SendMessage(m_hTooltips, TTM_SETDELAYTIME, TTDT_INITIAL, 0);

	return TRUE;
}

VOID CFormattedView::OnSize(__in WORD wHeight, __in WORD wWidth)
{
	// gather items to be resized
	HWND hCombo = GetDlgItem(m_hDialog, IDC_FORMATTEDCOMBO);
	HWND hListview = GetDlgItem(m_hDialog, IDC_FORMATTEDLIST);
	HWND hInject = GetDlgItem(m_hDialog, IDC_FORMATTEDINJECT);

	// sanity check
	if (hCombo == NULL || hListview == NULL || hInject == NULL)
		return;

	SetWindowPos(hListview, NULL, UI_LIST_X, UI_LIST_Y, wWidth - UI_LIST_PAD_RIGHT - UI_LIST_X, wHeight - UI_LIST_PAD_BOTTOM - UI_LIST_Y, 0);
	SetWindowPos(hCombo, NULL, UI_EDIT_X, wHeight - UI_EDIT_PAD_BOTTOM, wWidth - UI_EDIT_PAD_RIGHT - UI_EDIT_X, UI_EDIT_HEIGHT, 0);
	SetWindowPos(hInject, NULL, wWidth - UI_INJECT_PAD_RIGHT - UI_INJECT_WIDTH, wHeight - UI_INJECT_PAD_BOTTOM, UI_INJECT_WIDTH, UI_INJECT_HEIGHT, 0);
	ListView_SetColumnWidth(hListview, LVC_DATA, LVSCW_AUTOSIZE_USEHEADER);
}

INT_PTR CALLBACK CFormattedView::MaskedEdit(__in HWND hEdit, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	switch (uMessage)
	{
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;

		case WM_KEYDOWN:
			if (wParam == VK_RETURN)
			{
				CFormattedView* pFormattedView = GetClassInstance<CFormattedView>(GetParent(GetParent(hEdit)));

				if (pFormattedView != NULL)
					pFormattedView->OnInjectClick();
			}
			else if (wParam == VK_UP)
			{
				CFormattedView* pFormattedView = GetClassInstance<CFormattedView>(GetParent(GetParent(hEdit)));

				if (pFormattedView != NULL)
					pFormattedView->ScrollCombo(1);
			}
			else if (wParam == VK_DOWN)
			{
				CFormattedView* pFormattedView = GetClassInstance<CFormattedView>(GetParent(GetParent(hEdit)));

				if (pFormattedView != NULL)
					pFormattedView->ScrollCombo(-1);
			}
			else break;
			return 0;
	}

	WNDPROC WndProc = (WNDPROC)GetWindowLongPtrW(hEdit, GWLP_USERDATA);

	return WndProc != NULL ? CallWindowProcW(WndProc, hEdit, uMessage, wParam, lParam) : 0;
}

VOID CFormattedView::CopyPackets(__in BOOL bHeaders)
{
	std::vector<CMaplePacket*> vPackets;
	/***/
	LVITEM  lvi = { LVIF_PARAM, -1, 0 };
	HWND    hListview = GetDlgItem(m_hDialog, IDC_FORMATTEDLIST);

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
	CHAR  szTemp[OP_ALIAS_MAXCC + 1];
	DWORD dwOpcode;

	// form string
	foreach(CMaplePacket* pPacket, vPackets)
	{
		dwOpcode = pPacket->GetOpcode();

		if (bHeaders) // show packet headers (direction + size)
		{
			strBuffer += pPacket->GetDirection() == PACKET_SEND ? "->" : "<-";
			strBuffer += " || ";

			if (pOpcodeInfo != NULL)
			{
				if ((*pOpcodeInfo)[dwOpcode].wszAlias[0] == 0)
					StringCchPrintfA(szTemp, _countof(szTemp), "%08X", dwOpcode);
				else
					WideCharToMultiByte(CP_ACP, 0, (*pOpcodeInfo)[dwOpcode].wszAlias, -1, szTemp, _countof(szTemp), NULL, NULL);
			}

			strBuffer += szTemp;
			strBuffer += " || ";
		}
		else
		{
			StringCchPrintfA(szTemp, _countof(szTemp), "%08X ", dwOpcode);
			strBuffer += szTemp;
		}

		// skip the first member (opcode)
		for (UINT i = 1; i < pPacket->GetMemberCount(); i++)
		{
			const PACKET_MEMBER *pMember = &pPacket->GetMembers()->at(i);

			switch (pMember->Type)
			{
				case MEMBER_BYTE:
					StringCchPrintfA(szTemp, _countof(szTemp), "%02X ", pMember->data.b);
					strBuffer += szTemp;
					break;

				case MEMBER_WORD:
					StringCchPrintfA(szTemp, _countof(szTemp), "%04X ", pMember->data.w);
					strBuffer += szTemp;
					break;

				case MEMBER_DWORD:
					StringCchPrintfA(szTemp, _countof(szTemp), "%08X ", pMember->data.dw);
					strBuffer += szTemp;
					break;

				case MEMBER_ULONGLONG:
					StringCchPrintfA(szTemp, _countof(szTemp), "%016llX ", pMember->data.ull);
					strBuffer += szTemp;
					break;

				case MEMBER_BUFFER:
					strBuffer += "[";
					foreach(BYTE b, *pMember->data.buff)
					{
						StringCchPrintfA(szTemp, _countof(szTemp), "%02X", b);
						strBuffer += szTemp;
					}
					strBuffer += "] ";
					break;

				case MEMBER_STRING:
					strBuffer += "\"";
					strBuffer += *pMember->data.str;
					strBuffer += "\" ";
					break;
			}
		}

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

VOID CFormattedView::OnAddPacket(__inout CMaplePacket* pPacket)
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
	HWND    hListview = GetDlgItem(m_hDialog, IDC_FORMATTEDLIST);

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
	// - opcode
	WCHAR wszTemp[OP_ALIAS_MAXCC + 1];

	if ((*pOpcodeInfo)[pPacket->GetOpcode()].wszAlias[0] == 0)
	{
		StringCchPrintfW(wszTemp, _countof(wszTemp), L"%08X", pPacket->GetOpcode());
		lvi.pszText = wszTemp;
	}
	else
		lvi.pszText = (*pOpcodeInfo)[pPacket->GetOpcode()].wszAlias;
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = LVC_OPCODE;

	if (ListView_SetItem(hListview, &lvi) == -1)
		return;

	SendMessage(GetParent(m_hDialog), WM_UPDATEOPCODE, pPacket->GetOpcode(), 0);

	// THIRD COLUMN
	// - data
	std::wstring wstrBuffer;
	std::wstring wstrTemp;

	if (pPacket->GetMemberCount() == 1)
	{
		wstrBuffer = pStrings->Get(IDS_FORMATTED_NO_DATA);
	}
	else
	{
		for (UINT i = 1; i < pPacket->GetMemberCount(); i++)
		{
			const PACKET_MEMBER *pMember = &pPacket->GetMembers()->at(i);

			switch (pMember->Type)
			{
				case MEMBER_BYTE:
					StringCchPrintfW(wszTemp, _countof(wszTemp), L"%02X ", pMember->data.b);
					wstrBuffer += wszTemp;
					break;

				case MEMBER_WORD:
					StringCchPrintfW(wszTemp, _countof(wszTemp), L"%04X ", pMember->data.w);
					wstrBuffer += wszTemp;
					break;

				case MEMBER_DWORD:
					StringCchPrintfW(wszTemp, _countof(wszTemp), L"%08X ", pMember->data.dw);
					wstrBuffer += wszTemp;
					break;

				case MEMBER_ULONGLONG:
					StringCchPrintfW(wszTemp, _countof(wszTemp), L"%016llX ", pMember->data.ull);
					wstrBuffer += wszTemp;
					break;

				case MEMBER_BUFFER:
				{
					wstrBuffer += L"[";
					foreach(BYTE b, *pMember->data.buff)
					{
						StringCchPrintfW(wszTemp, _countof(wszTemp), L"%02X", b);
						wstrBuffer += wszTemp;
					}
					wstrBuffer += L"] ";
					break;
				}
				case MEMBER_STRING:
					wstrBuffer += L"\"";
					StringToWString(*pMember->data.str, wstrTemp);
					wstrBuffer += wstrTemp;
					wstrBuffer += L"\" ";
					break;
			}
		}

		// trim last whitespace
		wstrBuffer.erase(wstrBuffer.end() - 1, wstrBuffer.end());
	}

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

VOID CFormattedView::ListviewCustomDraw(__in LPNMLVCUSTOMDRAW lpnmlvcd)
{
	HWND hListview = GetDlgItem(m_hDialog, IDC_FORMATTEDLIST);

	// sanity check
	if (hListview == NULL)
	{
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
		return;
	}

	lpnmlvcd->nmcd.uItemState &= ~CDIS_FOCUS;

	if (lpnmlvcd->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT);
		return;
	}

	if (lpnmlvcd->nmcd.dwDrawStage == CDDS_POSTPAINT)
	{
		lpnmlvcd->clrTextBk = RGB(255, 255, 255);
		ListView_SetBkColor(hListview, lpnmlvcd->clrTextBk);
		SetBkColor(lpnmlvcd->nmcd.hdc, RGB(255, 255, 255));
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NEWFONT);
		return;
	}

	CMaplePacket* pPacket = (CMaplePacket*)lpnmlvcd->nmcd.lItemlParam;
	if (pPacket == NULL)
	{
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
		return;
	}

	if (pPacket->GetMemberCount() == 0)
	{
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
		return;
	}

	if (lpnmlvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		// highlight injected packets
		if (pPacket->GetState() & PACKET_INJECTED)
		{
			lpnmlvcd->clrTextBk = RGB(255, 225, 225);
			ListView_SetBkColor(hListview, lpnmlvcd->clrTextBk);
			SetBkColor(lpnmlvcd->nmcd.hdc, RGB(255, 225, 225));
		}
		else
		{
			lpnmlvcd->clrTextBk = RGB(255, 255, 255);
			ListView_SetBkColor(hListview, lpnmlvcd->clrTextBk);
			SetBkColor(lpnmlvcd->nmcd.hdc, RGB(255, 255, 255));
		}

		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW | CDRF_NOTIFYPOSTPAINT);
		return;
	}

	if (lpnmlvcd->nmcd.dwDrawStage != (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
	{
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
		return;
	}

	if (lpnmlvcd->iSubItem == LVC_OPCODE)
	{
		lpnmlvcd->clrText = (*pOpcodeInfo)[pPacket->GetOpcode()].crColor;
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NEWFONT);
		return;
	}

	// if it's not the data sub-item, continue
	if (lpnmlvcd->iSubItem != LVC_DATA)
	{
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, pPacket->GetState() & PACKET_INJECTED ? CDRF_NEWFONT : CDRF_DODEFAULT);
		return;
	}

	// if there is nothing besides the object skip
	if (pPacket->GetMemberCount() == 1)
	{
		lpnmlvcd->clrText = RGB(95, 95, 95);
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NEWFONT);
		return;
	}

	// let selected items draw normally
	if ((lpnmlvcd->nmcd.uItemState & CDIS_SELECTED) != 0)
	{
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
		return;
	}

	RECT SubItem;
	SIZE Size;

	if (!ListView_GetSubItemRect(hListview, lpnmlvcd->nmcd.dwItemSpec, lpnmlvcd->iSubItem, LVIR_BOUNDS, &SubItem))
	{
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
		return;
	}

	// width of one character

	std::wstring wstrData;

	SubItem.left -= 1;

	for (UINT i = 1; i < pPacket->GetMemberCount(); i++)
	{
		PACKET_MEMBER pm = pPacket->GetMembers()->at(i);

		wstrData.clear();
		wstrData += L" ";

		// no point in drawing anything more
		if (SubItem.left >= SubItem.right)
			break;

		WCHAR wszBuffer[32];

		switch (pm.Type)
		{
			case MEMBER_BYTE:
				if (FAILED(StringCchPrintfW(wszBuffer, _countof(wszBuffer), L"%02X", pm.data.b)))
				{
					SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
					return;
				}
				wstrData += wszBuffer;
				lpnmlvcd->clrText = MCLR_BYTE;
				break;

			case MEMBER_WORD:
				if (FAILED(StringCchPrintfW(wszBuffer, _countof(wszBuffer), L"%04X", pm.data.w)))
				{
					SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
					return;
				}
				wstrData += wszBuffer;
				lpnmlvcd->clrText = MCLR_WORD;
				break;

			case MEMBER_DWORD:
				if (FAILED(StringCchPrintfW(wszBuffer, _countof(wszBuffer), L"%08X", pm.data.dw)))
				{
					SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
					return;
				}
				wstrData += wszBuffer;
				lpnmlvcd->clrText = MCLR_DWORD;
				break;

			case MEMBER_ULONGLONG:
				if (FAILED(StringCchPrintfW(wszBuffer, _countof(wszBuffer), L"%016llX", pm.data.ull)))
				{
					SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
					return;
				}
				wstrData += wszBuffer;
				lpnmlvcd->clrText = MCLR_ULONGLONG;
				break;

			case MEMBER_STRING:
			{
				int iLength = MultiByteToWideChar(CP_ACP, 0, pm.data.str->c_str(), -1, NULL, 0);
				if (iLength == 0)
				{
					SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
					return;
				}
				boost::scoped_array<WCHAR> lpwszBuffer(new WCHAR[iLength]);
				if (MultiByteToWideChar(CP_ACP, 0, pm.data.str->c_str(), -1, lpwszBuffer.get(), iLength) == 0)
				{
					SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
					return;
				}
				wstrData += L"\"";
				wstrData += lpwszBuffer.get();
				wstrData += L"\"";
				lpnmlvcd->clrText = MCLR_STRING;
				break;
			}

			case MEMBER_BUFFER:
				wstrData += L"[";
				foreach(BYTE b, *pm.data.buff)
				{
					if (FAILED(StringCchPrintfW(wszBuffer, _countof(wszBuffer), L"%02X", b)))
					{
						SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
						return;
					}
					wstrData += wszBuffer;
				}
				wstrData += L"]";
				lpnmlvcd->clrText = MCLR_BUFFER;
				break;
		}

		ListView_SetTextColor(hListview, lpnmlvcd->clrText);
		SetTextColor(lpnmlvcd->nmcd.hdc, lpnmlvcd->clrText);

		// trim to the maximum length that ExtTextOutW can handle, just in case
		if (wstrData.length() > 8192 - 1)
			wstrData.resize(8192 - 1);

		ExtTextOutW(lpnmlvcd->nmcd.hdc, SubItem.left, SubItem.top + 2, ETO_OPAQUE | ETO_CLIPPED, &SubItem, wstrData.c_str(), wstrData.length(), NULL);

		if (!GetTextExtentPoint32W(lpnmlvcd->nmcd.hdc, wstrData.c_str(), wstrData.length(), &Size))
		{
			SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_DODEFAULT);
			return;
		}

		SubItem.left += Size.cx;
	}

	ListView_SetTextColor(hListview, RGB(0, 0, 0));
	SetTextColor(lpnmlvcd->nmcd.hdc, RGB(0, 0, 0));

	SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, CDRF_NEWFONT | CDRF_SKIPDEFAULT | CDRF_NOTIFYPOSTPAINT);
}

VOID CFormattedView::ListviewDoubleClick(__in LPNMITEMACTIVATE lpnmia)
{
	HWND hListview = GetDlgItem(m_hDialog, IDC_FORMATTEDLIST);

	// sanity check
	if (hListview == NULL)
		return;

	m_iTooltipsSel = lpnmia->iItem;
	SendMessage(m_hTooltips, TTM_POP, 0, 0);

	// do not respond if no item is selected or if the double click was outside of the opcode column
	if (lpnmia->iItem < 0)
		return;

	if (lpnmia->iSubItem == LVC_OPCODE)
		SendMessage(hListview, LVM_EDITLABEL, lpnmia->iItem, 0);
	else
	{
		LVITEM  lvi;

		lvi.iItem = lpnmia->iItem;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;

		if (!ListView_GetItem(hListview, &lvi))
			return;

		CMaplePacket* pPacket = (CMaplePacket*)lvi.lParam;

		if (pPacket == NULL)
			return;

		std::wstring wstrText;
		/***/
		wstrText = pStrings->Get(IDS_OPCODE);
		wstrText += L": ";
		DWORD dwOpcode = pPacket->GetOpcode();
		WCHAR wszTemp[48];

		StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%08X", dwOpcode);
		wstrText += wszTemp;

		if ((*pOpcodeInfo)[dwOpcode].wszAlias[0] != 0)
		{
			StringCchPrintfW(wszTemp, _countof(wszTemp), L" <%s>", (*pOpcodeInfo)[dwOpcode].wszAlias);
			wstrText += wszTemp;
		}

		wstrText += L"\r\n";
		wstrText += pStrings->Get(IDS_SIZE);
		wstrText += L": ";

		StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%X (%d bytes)", pPacket->GetSize(), pPacket->GetSize());
		wstrText += wszTemp;

		wstrText += L"\r\n";
		wstrText += pStrings->Get(IDS_TIME);
		wstrText += L": ";

		SYSTEMTIME SystemTime;
		if (FileTimeToSystemTime(&pPacket->GetCallerTime()->Time, &SystemTime))
		{

			GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &SystemTime, L"HH:mm:ss", wszTemp, _countof(wszTemp));
			wstrText += wszTemp;

			StringCchPrintfW(wszTemp, _countof(wszTemp), L".%03d", SystemTime.wMilliseconds);
			wstrText += wszTemp;

			if ((*pOpcodeInfo)[dwOpcode].wszComment[0] != 0)
			{
				wstrText += L"\r\n";
				wstrText += pStrings->Get(IDS_COMMENT);
				wstrText += L": ";
				wstrText += (*pOpcodeInfo)[dwOpcode].wszComment;
			}
		}

		StringCchPrintfW(wszTemp, _countof(wszTemp), L"%s #%d", pStrings->Get(IDS_PACKET).c_str(), pPacket->GetID());

		TOOLINFOW ti = { sizeof(TOOLINFOW) };
		ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
		ti.hwnd = m_hDialog;
		ti.uId = (UINT_PTR)hListview;
		ti.lpszText = (LPWSTR)wstrText.c_str();
		SendMessage(m_hTooltips, TTM_SETTITLE, TTI_INFO, (LPARAM)wszTemp);
		SendMessage(m_hTooltips, TTM_SETMAXTIPWIDTH, 0, (LPARAM)400);
		SendMessage(m_hTooltips, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	}
}

VOID CFormattedView::ListviewHotTrack(__in LPNMLISTVIEW lpnmlv)
{
	if (m_iTooltipsSel != lpnmlv->iItem)
	{
		m_iTooltipsSel = -1;

		TOOLINFOW ti = { sizeof(TOOLINFOW) };
		ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
		ti.hwnd = m_hDialog;
		ti.uId = (UINT_PTR)GetDlgItem(m_hDialog, IDC_FORMATTEDLIST);
		ti.lpszText = L"";

		SendMessage(m_hTooltips, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage(m_hTooltips, TTM_POP, 0, 0);
	}
}

VOID CFormattedView::ListviewRightClick(__in LPNMITEMACTIVATE lpnmia)
{
	HWND hListview = GetDlgItem(m_hDialog, IDC_FORMATTEDLIST);
	if (hListview == NULL)
		return;

	std::set<DWORD> sdwSelected;
	/***/
	CMaplePacket* pPacket = NULL;
	UINT          uCount = 0;
	MENUITEMINFOW mii = { sizeof(MENUITEMINFOW) };
	std::wstring  wstrTemp;
	BOOL          bAllBlocked = TRUE;
	BOOL          bAllIgnored = TRUE;
	OPCODE_INFO*  pTempOpcode = NULL;
	DWORD         dwOpcode;

	LVITEM lvi = { LVIF_PARAM, -1, 0 };

	while ((lvi.iItem = ListView_GetNextItem(hListview, lvi.iItem, LVNI_SELECTED)) != -1)
	{
		ListView_GetItem(hListview, &lvi);

		pPacket = (CMaplePacket*)lvi.lParam;

		if (pPacket != NULL)
		{
			if (pPacket->GetMemberCount() > 0)
			{
				dwOpcode = pPacket->GetOpcode();
				pTempOpcode = &(*pOpcodeInfo)[dwOpcode];
				if (pTempOpcode == NULL)
					break;
			}
		}
		else
			break;

		sdwSelected.insert(dwOpcode);

		if (!pTempOpcode->bBlock)
			bAllBlocked = FALSE;
		if (!pTempOpcode->bIgnore)
			bAllIgnored = FALSE;
	}

	HMENU hMenu = CreatePopupMenu();
	if (hMenu == NULL)
		return;

	if (lpnmia->iItem >= 0) // if a packet is selected, show packet menu items
	{
		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_STRING, IDM_COPYPACKET, pStrings->Get(IDS_COPY_PACKET).c_str());
		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_STRING, IDM_COPYPACKETDATA, pStrings->Get(IDS_COPY_PACKET_DATA).c_str());
		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_STRING, IDM_PACKETINFO, pStrings->Get(IDS_PACKET_INFO).c_str());
		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

		wstrTemp = pStrings->Get(IDS_IGNORE_PACKET);

		mii.fMask = MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_STRING | MIIM_STATE | MIIM_ID;
		mii.fType = MFT_STRING;
		if (sdwSelected.size() == 1)
			mii.fState = pTempOpcode->bBlock ? MFS_DISABLED : (MFS_ENABLED | (pTempOpcode->bIgnore ? MFS_CHECKED : MFS_UNCHECKED));
		else
			mii.fState = (bAllBlocked ? MFS_DISABLED : MFS_ENABLED) | (bAllIgnored ? MFS_CHECKED : MFS_UNCHECKED);
		mii.wID = IDM_IGNORE;
		mii.hbmpChecked = NULL;
		mii.hbmpUnchecked = NULL;
		mii.dwTypeData = (LPWSTR)wstrTemp.c_str();
		mii.cch = wstrTemp.length() + 1;

		InsertMenuItemW(hMenu, uCount++, TRUE, &mii);

		wstrTemp = pStrings->Get(IDS_BLOCK_PACKET);

		if (sdwSelected.size() == 1)
			mii.fState = MFS_ENABLED | (pTempOpcode->bBlock ? MFS_CHECKED : MFS_UNCHECKED);
		else
			mii.fState = MFS_ENABLED | (bAllBlocked ? MFS_CHECKED : MFS_UNCHECKED);
		mii.wID = IDM_BLOCK;
		mii.dwTypeData = (LPWSTR)wstrTemp.c_str();
		mii.cch = wstrTemp.length() + 1;

		InsertMenuItemW(hMenu, uCount++, TRUE, &mii);

		InsertMenuW(hMenu, uCount++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	}

	wstrTemp = pStrings->Get(IDS_AUTOSCROLL);

	mii.fMask = MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_STRING | MIIM_STATE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED | (bAutoscroll ? MFS_CHECKED : MFS_UNCHECKED);
	mii.wID = IDM_AUTOSCROLL;
	mii.cch = wstrTemp.length();
	mii.dwTypeData = (LPWSTR)wstrTemp.c_str();

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

			case IDM_IGNORE:
				if (sdwSelected.size() > 1)
				{
					foreach(DWORD dwTemp, sdwSelected)
					{
						(*pOpcodeInfo)[dwTemp].bIgnore = !bAllIgnored;
						SendMessage(GetParent(m_hDialog), WM_UPDATEOPCODE, dwTemp, 0);
					}
				}
				else
				{
					(*pOpcodeInfo)[dwOpcode].bIgnore = !(*pOpcodeInfo)[dwOpcode].bIgnore;
					SendMessage(GetParent(m_hDialog), WM_UPDATEOPCODE, dwOpcode, 0);
				}
				break;

			case IDM_BLOCK:
				if (sdwSelected.size() > 1)
				{
					foreach(DWORD dwTemp, sdwSelected)
					{
						(*pOpcodeInfo)[dwTemp].bBlock = !bAllBlocked;
						if (!bAllBlocked)
							(*pOpcodeInfo)[dwTemp].bIgnore = FALSE;
						SendMessage(GetParent(m_hDialog), WM_UPDATEOPCODE, dwTemp, 0);
					}
				}
				else
				{
					if (!((*pOpcodeInfo)[dwOpcode].bBlock = !(*pOpcodeInfo)[dwOpcode].bBlock))
						(*pOpcodeInfo)[dwOpcode].bIgnore = FALSE;
					else
						(*pOpcodeInfo)[dwOpcode].bIgnore = TRUE;

					SendMessage(GetParent(m_hDialog), WM_UPDATEOPCODE, dwOpcode, 0);
				}
				break;

			case IDM_AUTOSCROLL:
				bAutoscroll = !bAutoscroll;
				break;
		}
	}

	DestroyMenu(hMenu);
}

INT_PTR CALLBACK CFormattedView::ListviewEditProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	WNDPROC wpOld = (WNDPROC)GetWindowLongPtrW(hDialog, GWLP_USERDATA);

	if (uMessage == WM_SIZE)
	{
		INT   iFocused;
		RECT  rcSubItem;

		iFocused = ListView_GetNextItem(GetParent(hDialog), -1, LVNI_FOCUSED);
		ListView_GetSubItemRect(GetParent(hDialog), iFocused, LVC_OPCODE, LVIR_BOUNDS, &rcSubItem);
		SetWindowPos(hDialog, NULL, rcSubItem.left, rcSubItem.top, rcSubItem.right - rcSubItem.left, rcSubItem.bottom - rcSubItem.top, SWP_NOZORDER);
	}

	return CallWindowProc(wpOld, hDialog, uMessage, wParam, lParam);
}

VOID CFormattedView::ListviewBeginEdit(__in HWND* hEdit, __in NMLVDISPINFO* lpnmlvDisplayInfo)
{
	*hEdit = ListView_GetEditControl(GetDlgItem(m_hDialog, IDC_FORMATTEDLIST));

	if (*hEdit == NULL)
		return;

	SetWindowLongPtrW(*hEdit, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtrW(*hEdit, GWLP_WNDPROC));
	SetWindowLongPtrW(*hEdit, GWLP_WNDPROC, (LONG_PTR)CFormattedView::ListviewEditProc);

	SetWindowLong(*hEdit, GWL_STYLE, GetWindowLong(*hEdit, GWL_STYLE) | ES_CENTER | ES_UPPERCASE);
	Edit_LimitText(*hEdit, OP_ALIAS_MAXCC - 1);
}

VOID CFormattedView::UpdateOpcodeAlias(__in DWORD dwOpcode, __in LPWSTR lpwszBuffer)
{
	HWND hListview = GetDlgItem(m_hDialog, IDC_FORMATTEDLIST);
	if (hListview == NULL)
		return;

	LVITEM lvi = { LVIF_PARAM, 0, 0 };

	for (lvi.iItem; lvi.iItem < ListView_GetItemCount(hListview); lvi.iItem++)
	{
		ListView_GetItem(hListview, &lvi);
		CMaplePacket* pPacket = (CMaplePacket*)lvi.lParam;
		if (pPacket == NULL)
			continue;
		if (pPacket->GetMemberCount() < 1)
			continue;
		if (pPacket->GetOpcode() == dwOpcode)
		{
			if (lpwszBuffer[0] == 0)
			{
				StringCchPrintfW(lpwszBuffer, OP_ALIAS_MAXCC, L"%08X", dwOpcode);
				ListView_SetItemText(hListview, lvi.iItem, LVC_OPCODE, lpwszBuffer);
				lpwszBuffer[0] = 0;
			}
			else
				ListView_SetItemText(hListview, lvi.iItem, LVC_OPCODE, lpwszBuffer);
		}
	}
}

VOID CFormattedView::ListviewEndEdit(__in HWND* hEdit, __in NMLVDISPINFO* lpnmlvdi)
{
	if (*hEdit == NULL)
		return;

	CMaplePacket* pPacket = (CMaplePacket*)lpnmlvdi->item.lParam;

	if (pPacket == NULL)
		return;

	if (pPacket->GetMemberCount() < 1)
		return;

	DWORD dwOpcode = pPacket->GetOpcode();
	INT   nLength = Edit_GetTextLength(*hEdit);
	WCHAR *lpwszBuffer = (*pOpcodeInfo)[dwOpcode].wszAlias;

	if (nLength > 0)
		Edit_GetText(*hEdit, lpwszBuffer, OP_ALIAS_MAXCC);
	else
		StringCchPrintfW(lpwszBuffer, OP_ALIAS_MAXCC, L"");

	UpdateOpcodeAlias(dwOpcode, lpwszBuffer);

	PostMessage(GetParent(m_hDialog), WM_UPDATEOPCODE, dwOpcode, 0);

	*hEdit = NULL;
	return;
}

VOID CFormattedView::ListviewNotify(__in LPNMHDR lpnmhdr)
{
	switch (lpnmhdr->code)
	{
		case HDN_ENDTRACK:
			ListView_SetColumnWidth(GetDlgItem(m_hDialog, IDC_FORMATTEDLIST), LVC_DATA, LVSCW_AUTOSIZE_USEHEADER);
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

		case LVN_BEGINLABELEDIT:
			ListviewBeginEdit(&m_hEdit, (NMLVDISPINFO*)lpnmhdr);
			break;

		case LVN_ENDLABELEDIT:
			ListviewEndEdit(&m_hEdit, (NMLVDISPINFO*)lpnmhdr);
			break;

		case LVN_HOTTRACK:
			ListviewHotTrack((LPNMLISTVIEW)lpnmhdr);
			break;
	}
}

VOID CFormattedView::AddToScrollBack(__in LPCWSTR lpcwszPacket, __in PACKET_DIRECTION Direction)
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_FORMATTEDCOMBO);

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

BOOL CFormattedView::InjectPacket(__in LPSTR lpszBuffer)
{
	CPacket				packet;
	CMAPLEPACKETSTRUCT  cmps;

	cmps.pInstance = &packet;
	cmps.Direction = PACKET_SEND;
	cmps.ulState = PACKET_INJECTED;
	cmps.lpv = _ReturnAddress();

	pInstances->Add(&packet, pPacketPool->construct(&cmps));

	formatted_packet_parser parser(&packet, FALSE);

	std::string strData(lpszBuffer);

	std::string::const_iterator iter = strData.begin();
	std::string::const_iterator end = strData.end();

	if (!boost::spirit::qi::phrase_parse(iter, end, parser, boost::spirit::ascii::space) || iter != end)
	{
		pInstances->Remove(&packet);
		return FALSE;
	}

	if (m_InjectDirection == PACKET_SEND)
		SendPacket(pClientSocket, 0, &packet);
	else // m_InjectDirection == PACKET_RECV
	{
		ProcessPacket(&packet);
	}

	pInstances->Remove(&packet);

	return TRUE;
}

VOID CFormattedView::OnInjectClick()
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_FORMATTEDCOMBO);

	// sanity check
	if (hCombo == NULL)
		return;

	INT nLength = ComboBox_GetTextLength(hCombo);

	// if there is less than an opcode's worth of data, show an error
	if (nLength < sizeof(WORD) * 2)
	{
		HWND hComboEdit = FindWindowEx(hCombo, NULL, L"Edit", NULL);

		if (hComboEdit == NULL)
			return;

		std::wstring wstrTitle, wstrText;
		wstrTitle = wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
		wstrText = pStrings->Get(nLength == 0 ? IDS_INJECT_ERROR_EMPTY : IDS_INJECT_ERROR_SHORT);

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

	boost::scoped_array<CHAR> szBuffer(new CHAR[nLength + 1]);

	if (WideCharToMultiByte(CP_ACP, 0, wszBuffer.get(), -1, szBuffer.get(), nLength + 1, NULL, NULL) == 0)
		return; // assume something went wrong and go on with silent error

	if (!InjectPacket(szBuffer.get()))
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

	// add text to scroll-back log
	AddToScrollBack(wszBuffer.get(), m_InjectDirection);

	// clear control
	ComboBox_SetText(hCombo, NULL);
}

VOID CFormattedView::OnInjectReady(__in BOOL bReady)
{
	HWND hInject = GetDlgItem(m_hDialog, IDC_FORMATTEDINJECT);

	// sanity check
	if (hInject == NULL)
		return;

	Button_Enable(hInject, bReady);
}

VOID CFormattedView::ShowInjectMenu()
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
				SendMessageW(GetDlgItem(m_hDialog, IDC_FORMATTEDINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectOut);
				break;

			case IDM_RECEIVE:
				m_InjectDirection = PACKET_RECV;
				/***/
				SendMessageW(GetDlgItem(m_hDialog, IDC_FORMATTEDINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectIn);
				break;

			case IDM_SPAMPACKET:
				CreateSpamDialog();
				break;
		}
	}

	DestroyMenu(hMenu);
}

VOID CFormattedView::ClearAllPackets()
{
	ListView_DeleteAllItems(GetDlgItem(m_hDialog, IDC_FORMATTEDLIST));
}

VOID CFormattedView::CreateSpamDialog()
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_FORMATTEDCOMBO);

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

	pSpamPacket->bFormatted = TRUE;
	pSpamPacket->Direction = m_InjectDirection;
	pSpamPacket->pXPIGUI = m_pXPIGUI;

	ComboBox_SetText(hCombo, NULL);

	ShowWindow(CreateDialogParamW(m_pXPIGUI->hInstance, MAKEINTRESOURCEW(IDD_SPAMPACKET), m_hDialog, CSpamPacket::DialogProc, (LPARAM)pSpamPacket), SW_SHOW);
}

VOID CFormattedView::OnComboChange()
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_FORMATTEDCOMBO);

	// sanity check
	if (hCombo == NULL)
		return;

	INT iIndex = ComboBox_GetCurSel(hCombo);

	if (iIndex == CB_ERR)
		return;

	m_InjectDirection = (PACKET_DIRECTION)ComboBox_GetItemData(hCombo, iIndex);
	/***/
	SendMessageW(GetDlgItem(m_hDialog, IDC_FORMATTEDINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)(m_InjectDirection == PACKET_SEND ? m_pXPIGUI->hInjectOut : m_pXPIGUI->hInjectIn));
}

VOID CFormattedView::ScrollCombo(__in INT iOffset)
{
	HWND hCombo = GetDlgItem(m_hDialog, IDC_FORMATTEDCOMBO);

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
		SendMessageW(GetDlgItem(m_hDialog, IDC_FORMATTEDINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)(m_InjectDirection == PACKET_SEND ? m_pXPIGUI->hInjectOut : m_pXPIGUI->hInjectIn));
	}
}

INT_PTR CALLBACK CFormattedView::DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	switch (uMessage)
	{
		case WM_INITDIALOG:
			if (!(new CFormattedView(hDialog, (PXPIGUI)lParam))->OnCreate())
			{
				CMainWindow::NotifyErrorUI();
				EndDialog(GetParent(hDialog), EXIT_FAILURE);
			}
			break;

		case WM_NOTIFY:
			if (((LPNMHDR)lParam)->idFrom == IDC_FORMATTEDLIST)
				GetClassInstance<CFormattedView>(hDialog)->ListviewNotify((LPNMHDR)lParam);
			else if (((LPNMHDR)lParam)->idFrom == IDC_FORMATTEDINJECT && ((LPNMHDR)lParam)->code == BCN_DROPDOWN)
				GetClassInstance<CFormattedView>(hDialog)->ShowInjectMenu();
			break;

		case WM_SIZE:
			GetClassInstance<CFormattedView>(hDialog)->OnSize(HIWORD(lParam), LOWORD(lParam));
			break;

		case WM_ADDPACKET:
			GetClassInstance<CFormattedView>(hDialog)->OnAddPacket((CMaplePacket*)lParam);
			break;

		case WM_CLEARPACKETS:
			GetClassInstance<CFormattedView>(hDialog)->ClearAllPackets();
			break;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_FORMATTEDINJECT)
				GetClassInstance<CFormattedView>(hDialog)->OnInjectClick();
			else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_FORMATTEDCOMBO)
				GetClassInstance<CFormattedView>(hDialog)->OnComboChange();
			break;

		case WM_INJECTREADY:
			GetClassInstance<CFormattedView>(hDialog)->OnInjectReady((BOOL)wParam);
			break;

		case WM_FORMATALIAS:
			GetClassInstance<CFormattedView>(hDialog)->UpdateOpcodeAlias((WORD)wParam, (LPWSTR)lParam);
			break;

		case WM_DESTROY:
			delete GetClassInstance<CFormattedView>(hDialog);
			break;

		default:
			return FALSE;
	}

	return TRUE;
}
