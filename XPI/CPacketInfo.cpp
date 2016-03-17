#include "stdafx.h"

#include "CPacketInfo.hpp"

#include <windowsx.h>
#include <psapi.h>

#include "CResourceString.hpp"
#include "XPIUtilities.hpp"
#include "extvars.hpp"
#include "resource.h"

#pragma  comment(lib, "psapi")

CPacketInfo::CPacketInfo(__in HWND hDialog, __in PPACKETINFO pPacketInfo)
{
	// initialize class data
	m_hDialog = hDialog;
	m_pPacket = pPacketInfo->pPacket;
	m_pXPIGUI = pPacketInfo->pXPIGUI;

	// release temporary data
	delete pPacketInfo;

	// set window data to hold class instance
	SetWindowLongPtrW(m_hDialog, GWLP_USERDATA, (LONG_PTR)this);
}

CPacketInfo::~CPacketInfo()
{
	// clear mutex and flags
	m_pPacket->SetWParam(0);
}

BOOL CPacketInfo::OnCreate()
{
	if (m_pPacket->GetWParam() & CMPW_MUTEX)
		return FALSE;

	m_pPacket->SetWParam(CMPW_MUTEX | CMPW_OPCODE);

	WCHAR wszTemp[16];

	// initialize window title
	std::wstring wstrTitle = pStrings->Get(IDS_PACKET_INFO_TITLE);
	_itow_s(m_pPacket->GetID(), wszTemp, _countof(wszTemp), 10);
	wstrTitle += wszTemp;
	SetWindowText(m_hDialog, wstrTitle.c_str());

	// window icon
	SendMessageW(m_hDialog, WM_SETICON, ICON_BIG, (LPARAM)m_pXPIGUI->hIcon);
	SendMessageW(m_hDialog, WM_SETICON, ICON_SMALL, (LPARAM)m_pXPIGUI->hIconSmall);

	std::wstring wstrTemp;

	// opcode text
	DWORD         dwOpcode = m_pPacket->GetOpcode();
	OPCODE_INFO*  pTempOpcode = &(*pOpcodeInfo)[dwOpcode];

	StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%08X", dwOpcode);
	wstrTemp = wszTemp;

	if (pTempOpcode->wszAlias[0] != 0)
	{
		wstrTemp += L" (";
		wstrTemp += pTempOpcode->wszAlias;
		wstrTemp += L")";
	}

	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETINFOOPCODE), wstrTemp.c_str());
	SendMessageW(GetDlgItem(m_hDialog, IDC_PACKETINFOOPCODE), WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETLABELOPCODE), pStrings->Get(IDS_PACKET_INFO_OPCODE).c_str());

	// size text
	_itow_s(m_pPacket->GetSize(), wszTemp, _countof(wszTemp), 10);
	wstrTemp = wszTemp;
	wstrTemp += L" bytes";

	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETINFOSIZE), wstrTemp.c_str());
	SendMessageW(GetDlgItem(m_hDialog, IDC_PACKETINFOSIZE), WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETLABELSIZE), pStrings->Get(IDS_PACKET_INFO_SIZE).c_str());

	// time text
	SYSTEMTIME SystemTime;

	if (FileTimeToSystemTime(&m_pPacket->GetCallerTime()->Time, &SystemTime))
	{
		GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &SystemTime, L"HH:mm:ss", wszTemp, _countof(wszTemp));
		wstrTemp = wszTemp;
		if (SUCCEEDED(StringCchPrintfW(wszTemp, _countof(wszTemp), L".%03d", SystemTime.wMilliseconds)))
			wstrTemp += wszTemp;
	}

	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETINFOTIME), wstrTemp.c_str());
	SendMessageW(GetDlgItem(m_hDialog, IDC_PACKETINFOTIME), WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETLABELTIME), pStrings->Get(IDS_PACKET_INFO_TIME).c_str());

	// caller text
	if ((m_pPacket->GetState() & PACKET_INJECTED) == 0)
	{
//#ifdef _DEBUG
		StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%p", m_pPacket->GetCallerTime()->lpCaller);
//#else
	//	StringCchCopyW(wszTemp, _countof(wszTemp), L"MapleStory");
//#endif
		wstrTemp = wszTemp;
	}
	else
		wstrTemp = pStrings->Get(IDS_XPI);

	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETINFOCALLER), wstrTemp.c_str());
	SendMessageW(GetDlgItem(m_hDialog, IDC_PACKETINFOCALLER), WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETLABELCALLER), pStrings->Get(IDS_PACKET_INFO_CALLER).c_str());

	// edit control
	wstrTemp.clear();
	foreach(BYTE b, *m_pPacket->GetData())
	{
		StringCchPrintfW(wszTemp, _countof(wszTemp), L"%02X ", b);
		wstrTemp += wszTemp;
	}

	if (wstrTemp.length() > 0)
		wstrTemp.erase(wstrTemp.end() - 1, wstrTemp.end());

	Edit_SetText(GetDlgItem(m_hDialog, IDC_PACKETINFOEDIT), wstrTemp.c_str());
	SendMessageW(GetDlgItem(m_hDialog, IDC_PACKETINFOEDIT), WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);

	// listview
	if (!InitializeListview())
		return FALSE;

	// tooltips
	TOOLINFO  ti;
	HWND      hIconTips;

	hIconTips = CreateWindowEx(0, TOOLTIPS_CLASS, L"", TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hDialog, 0, m_pXPIGUI->hInstance, NULL);

	if (hIconTips == NULL)
		return FALSE;

	SetWindowPos(hIconTips, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = m_hDialog;

	if (m_pPacket->GetState() & PACKET_INJECTED)
		wstrTemp = m_pPacket->GetDirection() == PACKET_RECV ? pStrings->Get(IDS_INJRECVPACKET) : pStrings->Get(IDS_INJSENTPACKET);
	else
		wstrTemp = m_pPacket->GetDirection() == PACKET_RECV ? pStrings->Get(IDS_RECVPACKET) : pStrings->Get(IDS_SENTPACKET);

	ti.uId = (UINT_PTR)GetDlgItem(m_hDialog, IDC_PACKETINFODIR);
	ti.lpszText = (LPWSTR)wstrTemp.c_str();
	SendMessage(hIconTips, TTM_ADDTOOL, 0, (LPARAM)&ti);

	return TRUE;
}

BOOL CPacketInfo::InitializeListview()
{
	HWND hListview = GetDlgItem(m_hDialog, IDC_PACKETINFOLIST);

	if (hListview == NULL)
		return FALSE;

	std::wstring wstrTemp;

	// initialize columns
	LVCOLUMN lvc;

	lvc.mask = LVCF_WIDTH | LVCF_TEXT;
	lvc.cx = 70;
	wstrTemp = pStrings->Get(IDS_TIME);
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_TIME, &lvc) == -1)
		return FALSE;

	lvc.mask |= LVCF_FMT;
	lvc.fmt = LVCFMT_CENTER;
	lvc.cx = 86;
	wstrTemp = pStrings->Get(IDS_CALLER);
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_RETADD, &lvc) == -1)
		return FALSE;

	lvc.cx = 58;
	wstrTemp = pStrings->Get(IDS_TYPE);
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_TYPE, &lvc) == -1)
		return FALSE;

	lvc.mask = LVCF_WIDTH | LVCF_TEXT;
	wstrTemp = pStrings->Get(IDS_DATA);
	lvc.pszText = (LPWSTR)wstrTemp.c_str();
	if (ListView_InsertColumn(hListview, LVC_DATA, &lvc) == -1)
		return FALSE;

	const FILETIME*  lpPacketTime = &m_pPacket->GetCallerTime()->Time;
	/***/ WCHAR      wszTemp[32];

	// insert packet information data into listview
	for (UINT i = 1; i < m_pPacket->GetMemberCount(); i++)
	{
		// calculate time difference between packet creation and member creation
		const PACKET_MEMBER*  pMember = &m_pPacket->GetMembers()->at(i);
		const FILETIME*       pMemberTime = &pMember->CallerTime.Time;
		/***/ FILETIME        OffsetTime;
		/***/ SYSTEMTIME      SystemOffsetTime;
		OffsetTime.dwLowDateTime = pMemberTime->dwLowDateTime - lpPacketTime->dwLowDateTime;
		OffsetTime.dwHighDateTime = pMemberTime->dwHighDateTime - lpPacketTime->dwHighDateTime;
		FileTimeToSystemTime(&OffsetTime, &SystemOffsetTime);

		StringCchPrintfW(wszTemp, _countof(wszTemp), L"+%02d.%03d", SystemOffsetTime.wSecond, SystemOffsetTime.wMilliseconds);

		LVITEM lvi;
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iItem = i - 1;
		lvi.iSubItem = LVC_TIME;
		lvi.pszText = wszTemp;
		lvi.lParam = LVL_DATA_HEX;

		if (ListView_InsertItem(hListview, &lvi) == -1)
			return FALSE;

		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = LVC_RETADD;

		if (m_pPacket->GetState() & PACKET_INJECTED)
		{
			wstrTemp = pStrings->Get(IDS_XPI);
			lvi.pszText = (LPWSTR)wstrTemp.c_str();
		}
		else
		{
//#ifdef _DEBUG
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%p", pMember->CallerTime.lpCaller);
//#else
	//		StringCchCopyW(wszTemp, _countof(wszTemp), L"MapleStory");
//#endif
			lvi.pszText = wszTemp;
		}

		if (ListView_SetItem(hListview, &lvi) == -1)
			return FALSE;

		lvi.iSubItem = LVC_TYPE;

		switch (pMember->Type)
		{
		case MEMBER_BYTE:
			lvi.pszText = L"BYTE";
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%02X", pMember->data.b);
			wstrTemp = wszTemp;
			break;

		case MEMBER_WORD:
			lvi.pszText = L"WORD";
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%04X", pMember->data.w);
			wstrTemp = wszTemp;
			break;

		case MEMBER_DWORD:
			lvi.pszText = L"DWORD";
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%08X", pMember->data.dw);
			wstrTemp = wszTemp;
			break;

		case MEMBER_ULONGLONG:
			lvi.pszText = L"ULONGLONG";
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%016llX", pMember->data.ull);
			wstrTemp = wszTemp;
			break;

		case MEMBER_STRING:
			wstrTemp = L"";
			lvi.pszText = L"STRING";
			StringToWString(*pMember->data.str, wstrTemp);
			break;

		case MEMBER_BUFFER:
			wstrTemp = L"";
			lvi.pszText = L"BUFFER";
			foreach(BYTE b, *pMember->data.buff)
			{
				StringCchPrintfW(wszTemp, _countof(wszTemp), L"%02X ", b);
				wstrTemp += wszTemp;
			}
			wstrTemp.erase(wstrTemp.end() - 1, wstrTemp.end());
			break;
		}

		if (ListView_SetItem(hListview, &lvi) == -1)
			return FALSE;

		lvi.iSubItem = LVC_DATA;
		lvi.pszText = (LPWSTR)wstrTemp.c_str();

		if (ListView_SetItem(hListview, &lvi) == -1)
			return FALSE;
	}

	// aesthetic properties
	ListView_SetColumnWidth(hListview, LVC_DATA, LVSCW_AUTOSIZE_USEHEADER);
	SendMessageW(hListview, WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	ListView_SetExtendedListViewStyle(hListview, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

	return TRUE;
}

VOID CPacketInfo::ListviewDoubleClick(__in LPNMITEMACTIVATE lpnmia)
{
	// do not respond if no item is selected or if the double click was outside of the opcode column
	if (lpnmia->iItem < 0 || lpnmia->iSubItem != LVC_DATA)
		return;

	if (lpnmia->iItem + 1 >= (INT)m_pPacket->GetMemberCount())
		return;

	const PACKET_MEMBER* pMember = &m_pPacket->GetMembers()->at(lpnmia->iItem + 1);
	if (pMember == NULL)
		return;

	if (pMember->Type != MEMBER_BYTE && pMember->Type != MEMBER_WORD && pMember->Type != MEMBER_DWORD && pMember->Type != MEMBER_ULONGLONG)
		return;

	HWND    hListview = GetDlgItem(m_hDialog, IDC_PACKETINFOLIST);
	LVITEM  lvi = { LVIF_PARAM, lpnmia->iItem, 0 };

	WCHAR wszTemp[32];

	ListView_GetItem(hListview, &lvi);

	if (lvi.lParam == LVL_DATA_HEX)
	{
		switch (pMember->Type)
		{
		case MEMBER_BYTE:
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"%d", pMember->data.b);
			break;

		case MEMBER_WORD:
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"%d", pMember->data.w);
			break;

		case MEMBER_DWORD:
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"%d", pMember->data.dw);
			break;

		case MEMBER_ULONGLONG:
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"%lld", pMember->data.ull);
			break;
		}

		lvi.lParam = LVL_DATA_DEC;
		ListView_SetItem(hListview, &lvi);
	}
	else
	{
		switch (pMember->Type)
		{
		case MEMBER_BYTE:
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%02X", pMember->data.b);
			break;

		case MEMBER_WORD:
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%04X", pMember->data.w);
			break;

		case MEMBER_DWORD:
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%08X", pMember->data.dw);
			break;

		case MEMBER_ULONGLONG:
			StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%016llX", pMember->data.ull);
			break;
		}

		lvi.lParam = LVL_DATA_HEX;
		ListView_SetItem(hListview, &lvi);
	}

	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = LVC_DATA;
	lvi.pszText = wszTemp;
	ListView_SetItem(hListview, &lvi);
}

VOID CPacketInfo::ListviewRightClick(__in LPNMITEMACTIVATE lpnmia)
{
	INT   iMenuCount = 0;
	POINT Location;

	if (lpnmia->iItem < 0)
		return;

	if (!GetCursorPos(&Location))
		return;

	HMENU hMenu = CreatePopupMenu();
	if (hMenu == NULL)
		return;

	if (!InsertMenu(hMenu, iMenuCount++, MF_BYPOSITION | MF_STRING, IDM_COPYMEMBER, pStrings->Get(IDS_COPY).c_str()))
	{
		DestroyMenu(hMenu);
		return;
	}

	if (!InsertMenu(hMenu, iMenuCount++, MF_BYPOSITION | MF_STRING, IDM_COPYMEMBERDATA, pStrings->Get(IDS_COPY_MEMBER_DATA).c_str()))
	{
		DestroyMenu(hMenu);
		return;
	}

	INT nChoice = (INT)TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, Location.x, Location.y, 0, m_hDialog, NULL);

	if (nChoice == IDM_COPYMEMBER)
		CopyMember(TRUE);
	else if (nChoice == IDM_COPYMEMBERDATA)
		CopyMember(FALSE);

	DestroyMenu(hMenu);
}

VOID CPacketInfo::CopyMember(__in BOOL bHeaders)
{
	HWND hListview = GetDlgItem(m_hDialog, IDC_PACKETINFOLIST);
	if (hListview == NULL)
		return;

	// store index and hex/dec setting for data
	std::map<const PACKET_MEMBER*, BOOL> mMembers;
	/***/
	LVITEM lvi = { LVIF_PARAM, -1, 0 };

	while ((lvi.iItem = ListView_GetNextItem(hListview, lvi.iItem, LVNI_SELECTED)) != -1)
	{
		ListView_GetItem(hListview, &lvi);
		if (lvi.iItem + 1 > (INT)m_pPacket->GetMemberCount())
			return;
		const PACKET_MEMBER* pTempMember = &m_pPacket->GetMembers()->at(lvi.iItem + 1);
		mMembers[pTempMember] = lvi.lParam;
	}

	if (mMembers.size() == 0)
		return;

	std::string     strBuffer;
	CHAR            szTemp[32];
	const FILETIME* lpPacketTime = &m_pPacket->GetCallerTime()->Time;

	for (std::map<const PACKET_MEMBER*, BOOL>::iterator i = mMembers.begin(); i != mMembers.end(); ++i)
	{
		const PACKET_MEMBER* pMember = i->first;

		if (bHeaders)
		{
			const FILETIME*   pMemberTime = &pMember->CallerTime.Time;
			/***/ FILETIME    OffsetTime;
			/***/ SYSTEMTIME  SystemOffsetTime;

			OffsetTime.dwLowDateTime = pMemberTime->dwLowDateTime - lpPacketTime->dwLowDateTime;
			OffsetTime.dwHighDateTime = pMemberTime->dwHighDateTime - lpPacketTime->dwHighDateTime;
			FileTimeToSystemTime(&OffsetTime, &SystemOffsetTime);

			StringCchPrintfA(szTemp, _countof(szTemp), "+%02d.%03d", SystemOffsetTime.wSecond, SystemOffsetTime.wMilliseconds);
			strBuffer += szTemp;
			strBuffer += " || ";

			if (m_pPacket->GetState() & PACKET_INJECTED)
			{
				std::wstring wstrXPI = pStrings->Get(IDS_XPI);
				if (WideCharToMultiByte(CP_ACP, 0, wstrXPI.c_str(), -1, szTemp, _countof(szTemp), NULL, NULL) == 0)
					return;
			}
			else
			{
//#ifdef _DEBUG
				StringCchPrintfA(szTemp, _countof(szTemp), "0x%p", pMember->CallerTime.lpCaller);
//#else
	//			StringCchCopyA(szTemp, _countof(szTemp), "MapleStory");
//#endif
			}

			strBuffer += szTemp;
			strBuffer += " || ";

			switch (pMember->Type)
			{
			case MEMBER_BYTE:
				strBuffer += "BYTE";
				break;

			case MEMBER_WORD:
				strBuffer += "WORD";
				break;

			case MEMBER_DWORD:
				strBuffer += "DWORD";
				break;

			case MEMBER_ULONGLONG:
				strBuffer += "ULONGLONG";
				break;

			case MEMBER_STRING:
				strBuffer += "STRING";
				break;

			case MEMBER_BUFFER:
				strBuffer += "BUFFER";
				break;
			}

			strBuffer += " || ";
		}

		// copy data
		switch (pMember->Type)
		{
		case MEMBER_BYTE:
			if (i->second == LVL_DATA_HEX)
				StringCchPrintfA(szTemp, _countof(szTemp), "0x%02X", pMember->data.b);
			else
				_itoa_s(pMember->data.b, szTemp, _countof(szTemp), 10);
			strBuffer += szTemp;
			break;

		case MEMBER_WORD:
			if (i->second == LVL_DATA_HEX)
				StringCchPrintfA(szTemp, _countof(szTemp), "0x%04X", pMember->data.w);
			else
				_itoa_s(pMember->data.w, szTemp, _countof(szTemp), 10);
			strBuffer += szTemp;
			break;

		case MEMBER_DWORD:
			if (i->second == LVL_DATA_HEX)
				StringCchPrintfA(szTemp, _countof(szTemp), "0x%08X", pMember->data.dw);
			else
				_itoa_s(pMember->data.dw, szTemp, _countof(szTemp), 10);
			strBuffer += szTemp;
			break;

		case MEMBER_ULONGLONG:
			if (i->second == LVL_DATA_HEX)
				StringCchPrintfA(szTemp, _countof(szTemp), "0x%016llX", pMember->data.ull);
			else
				_i64toa_s(pMember->data.ull, szTemp, _countof(szTemp), 10);
			strBuffer += szTemp;
			break;

		case MEMBER_STRING:
			strBuffer += *pMember->data.str;
			break;

		case MEMBER_BUFFER:
			foreach(BYTE b, *pMember->data.buff)
			{
				StringCchPrintfA(szTemp, _countof(szTemp), "%02X ", b);
				strBuffer += szTemp;
			}
			strBuffer.erase(strBuffer.end() - 1, strBuffer.end());
			break;
		}

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

LONG_PTR CPacketInfo::ListviewCustomDraw(__in LPNMLVCUSTOMDRAW lpnmlvcd)
{
	if (lpnmlvcd->nmcd.dwDrawStage == CDDS_PREPAINT)
		return CDRF_NOTIFYITEMDRAW;

	if (lpnmlvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
		return CDRF_NOTIFYSUBITEMDRAW;

	if (lpnmlvcd->nmcd.dwDrawStage != (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
		return CDRF_DODEFAULT;

	if (lpnmlvcd->iSubItem != LVC_TYPE && lpnmlvcd->iSubItem != LVC_DATA)
	{
		// black color (default)
		lpnmlvcd->clrText = RGB(0, 0, 0);
		return CDRF_NEWFONT;
	}

	if (lpnmlvcd->nmcd.dwItemSpec + 1 >= (INT)m_pPacket->GetMemberCount())
		return CDRF_DODEFAULT;

	const PACKET_MEMBER* pMember = &m_pPacket->GetMembers()->at(lpnmlvcd->nmcd.dwItemSpec + 1);

	switch (pMember->Type)
	{
	case MEMBER_BYTE:
		lpnmlvcd->clrText = MCLR_BYTE;
		break;

	case MEMBER_WORD:
		lpnmlvcd->clrText = MCLR_WORD;
		break;

	case MEMBER_DWORD:
		lpnmlvcd->clrText = MCLR_DWORD;
		break;

	case MEMBER_ULONGLONG:
		lpnmlvcd->clrText = MCLR_ULONGLONG;
		break;

	case MEMBER_STRING:
		lpnmlvcd->clrText = MCLR_STRING;
		break;

	case MEMBER_BUFFER:
		lpnmlvcd->clrText = MCLR_BUFFER;
		break;
	}

	return CDRF_NEWFONT;
}

VOID CPacketInfo::ListviewNotify(__in LPNMHDR lpnmhdr)
{
	switch (lpnmhdr->code)
	{
	case NM_DBLCLK:
		ListviewDoubleClick((LPNMITEMACTIVATE)lpnmhdr);
		break;

	case NM_RCLICK:
		ListviewRightClick((LPNMITEMACTIVATE)lpnmhdr);
		break;

	case LVN_KEYDOWN:
		if (((LPNMLVKEYDOWN)lpnmhdr)->wVKey == 0x43 && GetAsyncKeyState(VK_CONTROL)) // ctrl+c
			CopyMember(TRUE);

	case NM_CUSTOMDRAW:
		SetWindowLongPtr(m_hDialog, DWLP_MSGRESULT, ListviewCustomDraw((LPNMLVCUSTOMDRAW)lpnmhdr));
		break;
	}
}

VOID CPacketInfo::SizeDoubleClicked()
{
	if (m_pPacket->GetWParam() & CMPW_SIZE)
		m_pPacket->SetWParam(m_pPacket->GetWParam() & ~CMPW_SIZE);
	else
		m_pPacket->SetWParam(m_pPacket->GetWParam() | CMPW_SIZE);

	std::wstring wstrSize;
	/***/
	WCHAR wszTemp[16];

	if (!(m_pPacket->GetWParam() & CMPW_SIZE) || m_pPacket->GetSize() < 10)
		_itow_s(m_pPacket->GetSize(), wszTemp, _countof(wszTemp), 10);
	else
		StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%X", m_pPacket->GetSize());

	wstrSize += wszTemp;
	wstrSize += L" bytes";

	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETINFOSIZE), wstrSize.c_str());
}

VOID CPacketInfo::OpcodeDoubleClicked()
{
	if (m_pPacket->GetWParam() & CMPW_OPCODE)
		m_pPacket->SetWParam(m_pPacket->GetWParam() & ~CMPW_OPCODE);
	else
		m_pPacket->SetWParam(m_pPacket->GetWParam() | CMPW_OPCODE);

	std::wstring wstrOpcode;
	/***/
	DWORD         dwOpcode = m_pPacket->GetOpcode();
	OPCODE_INFO*  pTempOpcode = &(*pOpcodeInfo)[dwOpcode];
	WCHAR         wszTemp[32];

	if (m_pPacket->GetWParam() & CMPW_OPCODE)
		StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%08X", dwOpcode);
	else
		_itow_s(dwOpcode, wszTemp, _countof(wszTemp), 10);

	wstrOpcode += wszTemp;

	if (pTempOpcode->wszAlias[0] != 0)
	{
		wstrOpcode += L" (";
		wstrOpcode += pTempOpcode->wszAlias;
		wstrOpcode += L")";
	}

	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETINFOOPCODE), wstrOpcode.c_str());
}

VOID CPacketInfo::CallerDoubleClicked()
{
	if (m_pPacket->GetWParam() & CMPW_CALLER)
		m_pPacket->SetWParam(m_pPacket->GetWParam() & ~CMPW_CALLER);
	else
		m_pPacket->SetWParam(m_pPacket->GetWParam() | CMPW_CALLER);

	WCHAR wszTemp[32];

	std::wstring wstrCaller;

	if ((m_pPacket->GetState() & PACKET_INJECTED) == 0)
	{
//#ifdef _DEBUG
		if (m_pPacket->GetWParam() & CMPW_CALLER)
		{
			HMODULE hModule;
			if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)m_pPacket->GetCallerTime()->lpCaller, &hModule))
				if (GetModuleBaseName(GetCurrentProcess(), hModule, wszTemp, _countof(wszTemp)))
				{
					wstrCaller += L"\"";
					wstrCaller += wszTemp;
					wstrCaller += L"\"->";
				}
		}
		StringCchPrintfW(wszTemp, _countof(wszTemp), L"0x%p", m_pPacket->GetCallerTime()->lpCaller);
//#else
	//	StringCchCopyW(wszTemp, _countof(wszTemp), L"MapleStory");
//#endif
		wstrCaller += wszTemp;
	}
	else
		wstrCaller += pStrings->Get(IDS_XPI);

	Static_SetText(GetDlgItem(m_hDialog, IDC_PACKETINFOCALLER), wstrCaller.c_str());
}

VOID CPacketInfo::DrawColor(__in LPDRAWITEMSTRUCT lpdis)
{
	COLORREF crColor = (*pOpcodeInfo)[m_pPacket->GetOpcode()].crColor;
	if (crColor != RGB(0, 0, 0))
	{
		SelectObject(lpdis->hDC, GetStockObject(BLACK_PEN));
		HBRUSH hBrush = CreateSolidBrush(crColor);
		SelectObject(lpdis->hDC, hBrush);
		Rectangle(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right, lpdis->rcItem.bottom);
		DeleteObject(hBrush);
	}
}

VOID CPacketInfo::DrawDirection(__in LPDRAWITEMSTRUCT lpdis)
{
	if ((m_pPacket->GetState() & PACKET_INJECTED) == 0)
		ImageList_Draw(m_pXPIGUI->hImageList, m_pPacket->GetDirection() == PACKET_SEND ? XPI_ILI_SEND : XPI_ILI_RECV, lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, ILD_TRANSPARENT);
	else
		DrawIconEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, m_pPacket->GetDirection() == PACKET_SEND ? m_pXPIGUI->hInjectOut : m_pXPIGUI->hInjectIn, 16, 16, 0, NULL, DI_NORMAL);
}

INT_PTR CALLBACK CPacketInfo::DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		if (!(new CPacketInfo(hDialog, (PPACKETINFO)lParam))->OnCreate())
		{
			EndDialog(hDialog, EXIT_FAILURE);
			DestroyWindow(hDialog);
			return FALSE;
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == STN_DBLCLK)
		{
			if (LOWORD(wParam) == IDC_PACKETINFOSIZE)
				GetClassInstance<CPacketInfo>(hDialog)->SizeDoubleClicked();
			else if (LOWORD(wParam) == IDC_PACKETINFOOPCODE)
				GetClassInstance<CPacketInfo>(hDialog)->OpcodeDoubleClicked();
			else if (LOWORD(wParam) == IDC_PACKETINFOCALLER)
				GetClassInstance<CPacketInfo>(hDialog)->CallerDoubleClicked();
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_PACKETINFOLIST)
			GetClassInstance<CPacketInfo>(hDialog)->ListviewNotify((LPNMHDR)lParam);
		break;

	case WM_DRAWITEM:
		if (wParam == IDC_PACKETINFOCOLOR)
			GetClassInstance<CPacketInfo>(hDialog)->DrawColor((LPDRAWITEMSTRUCT)lParam);
		else if (wParam == IDC_PACKETINFODIR)
			GetClassInstance<CPacketInfo>(hDialog)->DrawDirection((LPDRAWITEMSTRUCT)lParam);
		break;

	case WM_CLOSE:
		EndDialog(hDialog, EXIT_SUCCESS);
		DestroyWindow(hDialog);
		break;

	case WM_DESTROY:
		delete GetClassInstance<CPacketInfo>(hDialog);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}
