#ifndef CPLAIN_VIEW_HPP_
#define CPLAIN_VIEW_HPP_

#include "CMainWindow.hpp"
#include "CMaplePacket.hpp"

class CPlainView
{
private:
	// window handle
	HWND m_hDialog;
	// XPI GUI
	PXPIGUI m_pXPIGUI;
	// injection direction
	PACKET_DIRECTION m_InjectDirection;
	// right-click menu item IDs
	enum
	{
		IDM_COPYPACKET = 1,
		IDM_COPYPACKETDATA,
		IDM_PACKETINFO,
		IDM_AUTOSCROLL
	};
	// inject drop-down menu item IDs
	enum
	{
		IDM_SEND = 1,
		IDM_RECEIVE,
		IDM_SPAMPACKET,
	};
	// listview column IDs
	enum
	{
		LVC_DIRECTION = 0,
		LVC_SIZE,
		LVC_DATA
	};
	// UI element coordinates
	enum
	{
		UI_LIST_X = 6,
		UI_LIST_Y = 6,
		UI_LIST_PAD_RIGHT = 6,
		UI_LIST_PAD_BOTTOM = 36,

		UI_EDIT_X = 6,
		UI_EDIT_HEIGHT = 19,
		UI_EDIT_PAD_RIGHT = 56,
		UI_EDIT_PAD_BOTTOM = 30,

		UI_INJECT_WIDTH = 48,
		UI_INJECT_HEIGHT = 26,
		UI_INJECT_PAD_RIGHT = 6,
		UI_INJECT_PAD_BOTTOM = 33
	};
public:
	CPlainView(__in HWND hDialog, __in PXPIGUI pXPIGUI);
	BOOL OnCreate();
	VOID OnSize(__in WORD wHeight, __in WORD wWidth);
	VOID CopyPackets(__in BOOL bHeaders);
	VOID ListviewCustomDraw(__inout LPNMLVCUSTOMDRAW lpnmlvcd);
	VOID ListviewNotify(__in LPNMHDR lpnmhdr);
	VOID ListviewDoubleClick(__in LPNMITEMACTIVATE lpnmia);
	VOID ListviewRightClick(__in LPNMITEMACTIVATE lpnmia);
	static INT_PTR CALLBACK MaskedEdit(__in HWND hCombo, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
	VOID AddToScrollBack(__in LPCWSTR lpcwszPacket, __in PACKET_DIRECTION Direction);
	VOID OnInjectReady(__in BOOL bReady);
	VOID OnInjectClick();
	VOID OnAddPacket(__inout CMaplePacket* pPacket);
	VOID ShowInjectMenu();
	VOID CreateSpamDialog();
	VOID ClearAllPackets();
	VOID OnComboChange();
	VOID ScrollCombo(__in INT iOffset);
	static INT_PTR CALLBACK DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
};

#endif // CPLAIN_VIEW_HPP_