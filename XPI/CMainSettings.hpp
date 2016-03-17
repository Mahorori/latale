#ifndef CMAIN_SETTINGS_HPP_
#define CMAIN_SETTINGS_HPP_

#include <map>

#include "CMainWindow.hpp"

typedef std::map<HTREEITEM, HWND> TREEITEM_MAP;

typedef struct _TREE_INFO
{
	std::wstring wstrCaption;
	/***/
	LPWSTR      lpwszTemplate;
	DLGPROC     DlgProc;
	_TREE_INFO* lpNodes;
	USHORT      usNodeCount;
} TREE_INFO, far *LPTREE_INFO, near *PTREE_INFO;

class CMainSettings
{
private:
	// window handle
	HWND m_hDialog;
	// XPI GUI
	PXPIGUI m_pXPIGUI;
	// tree item map
	TREEITEM_MAP m_mTreeItems;
	// UI coordinates
	enum
	{
		UI_TREE_WIDTH = 128,
		UI_TREE_SUBHEIGHT = 13,

		UI_ITEM_SUBWIDTH = 146,
		UI_ITEM_SUBHEIGHT = 13
	};

public:
	CMainSettings(__in HWND hDialog, __in PXPIGUI pXPIGUI);
	BOOL PopulateTree(__in_ecount(uItems) PTREE_INFO Items, __in UINT uCount, __in_opt HTREEITEM hItemParent);

	BOOL OnCreate();
	VOID OnSize(__in WORD wWidth, __in WORD wHeight);
	VOID RelayToChildren(__in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
	VOID OnTreeSelection(__in LPNMTREEVIEW lpnmtv);

	static INT_PTR CALLBACK DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
};

#endif // CMAIN_SETTINGS_HPP_