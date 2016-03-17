#ifndef CSETTINGS_PACKET_HPP_
#define CSETTINGS_PACKET_HPP_

#include "CMainWindow.hpp"
#include "CHeader.hpp"

class CSettingsPacket
{
public:
	// listview sort types
	enum LV_ORDER
	{
		LVS_STATE_ASC = 0,
		LVS_STATE_DESC,
		LVS_OPCODE_ASC,
		LVS_OPCODE_DESC,
		LVS_ALIAS_ASC,
		LVS_ALIAS_DESC,
		LVS_COMMENT_ASC,
		LVS_COMMENT_DESC
	};
	// listview column IDs
	enum LV_COLUMNS
	{
		LVC_STATE = 0,
		LVC_OPCODE,
		LVC_ALIAS,
		LVC_COMMENT
	};
private:
	// window handle
	HWND      m_hDialog;
	// XPI GUI
	PXPIGUI   m_pXPIGUI;
	// graphical header
	CHeader*  m_pHeader;
	// listview edit control
	HWND      m_hEdit;
	// listview sort
	CSettingsPacket::LV_ORDER   m_Sort;
	// last double-click (related to listview edit)
	CSettingsPacket::LV_COLUMNS m_DoubleClick;
	// UI coordinates
	enum
	{
		UI_LIST_SUBHEIGHT = 59
	};
	// right-click menu item IDs
	enum
	{
		IDM_IGNORE = 1,
		IDM_BLOCK,
		IDM_COLOR
	};

public:
	CSettingsPacket(__in HWND hDialog, __in PXPIGUI pXPIGUI);
	~CSettingsPacket();
	BOOL OnCreate();
	VOID OnSize(__in WORD wWidth, __in WORD wHeight);
	VOID OnPaint();
	VOID OnUpdateOpcode(__in DWORD dwOpcode);
	static INT_PTR CALLBACK ListviewEditProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
	VOID ListviewBeginEdit(__in HWND* hEdit, __in NMLVDISPINFO* lpnmlvdi);
	VOID ListviewEndEdit(__in HWND* hEdit, __in NMLVDISPINFO* lpnmlvdi);
	static INT CALLBACK ListviewSort(__in LPARAM lParam1, __in LPARAM lParam2, __in LPARAM lParamSort);
	VOID ListviewDoubleClick(__in LPNMITEMACTIVATE lpnmia);
	VOID ListviewRightClick(__in LPNMITEMACTIVATE lpnmia);
	VOID ListviewColumnClick(__in LPNMLISTVIEW lpnmlv);
	VOID ListviewCustomDraw(__inout LPNMLVCUSTOMDRAW lpnmlvcd);
	VOID ListviewNotify(__in LPNMHDR lpnmhdr);
	LV_ORDER GetSort() const { return m_Sort; }
	LV_COLUMNS GetDoubleClicked() const { return m_DoubleClick; }
	static INT_PTR CALLBACK DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
};

#endif // CSETTINGS_PACKET_HPP_