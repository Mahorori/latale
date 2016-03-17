#ifndef CPACKET_INFO_HPP_
#define CPACKET_INFO_HPP_

#include "CMaplePacket.hpp"
#include "CMainWindow.hpp"

// initializer structure (passed as LPARAM)
typedef struct _PACKETINFO
{
	CMaplePacket* pPacket;
	PXPIGUI       pXPIGUI;
} PACKETINFO, far *LPPACKETINFO, near *PPACKETINFO;

class CPacketInfo
{
private:
	// window handle
	HWND          m_hDialog;
	// XPI GUI
	PXPIGUI       m_pXPIGUI;
	// packet
	CMaplePacket* m_pPacket;
	// CMaplePacket wParam masks
	enum
	{
		CMPW_MUTEX = 0x1,
		CMPW_SIZE = 0x2,
		CMPW_OPCODE = 0x4,
		CMPW_CALLER = 0x8
	};
	// listview columns
	enum
	{
		LVC_TIME = 0,
		LVC_RETADD,
		LVC_TYPE,
		LVC_DATA
	};
	// listview lParam data
	enum
	{
		LVL_DATA_HEX = 0,
		LVL_DATA_DEC
	};
	// right-click menu item IDs
	enum
	{
		IDM_COPYMEMBER = 1,
		IDM_COPYMEMBERDATA
	};
public:
	CPacketInfo(__in HWND hDialog, __in PPACKETINFO pPacketInfo);
	~CPacketInfo();
	BOOL OnCreate();
	BOOL InitializeListview();
	VOID ListviewDoubleClick(__in LPNMITEMACTIVATE lpnmia);
	VOID ListviewRightClick(__in LPNMITEMACTIVATE lpnmia);
	VOID CopyMember(__in BOOL bHeaders);
	LONG_PTR ListviewCustomDraw(__in LPNMLVCUSTOMDRAW lpnmlvcd);
	VOID ListviewNotify(__in LPNMHDR lpnmhdr);
	VOID SizeDoubleClicked();
	VOID OpcodeDoubleClicked();
	VOID CallerDoubleClicked();
	VOID DrawColor(__in LPDRAWITEMSTRUCT lpdis);
	VOID DrawDirection(__in LPDRAWITEMSTRUCT lpdis);
	static INT_PTR CALLBACK DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
};

#endif // CPACKET_INFO_HPP_