#include "stdafx.h"

#include "CMainSettings.hpp"

#include <uxtheme.h>

#include "CResourceString.hpp"
#include "CLog.hpp"
#include "resource.h"
#include "XPIUtilities.hpp"
#include "extvars.hpp"

// children pages (tree items)
#include "CSettingsPacket.hpp"

CMainSettings::CMainSettings(__in HWND hDialog, __in PXPIGUI pXPIGUI)
{
	// initialize class data
	m_hDialog = hDialog;
	m_pXPIGUI = pXPIGUI;

	// set window data to hold class instance
	SetWindowLongPtrW(m_hDialog, GWLP_USERDATA, (LONG_PTR)this);
}

BOOL CMainSettings::PopulateTree(__in_ecount(uItems) PTREE_INFO Items, __in UINT uCount, __in_opt HTREEITEM hItemParent)
{
	HWND hTree = GetDlgItem(m_hDialog, IDC_SETTINGSTREE);

	// sanity check
	if (hTree == NULL)
		return FALSE;

	// end recursive call
	if (Items == NULL || uCount == 0)
		return TRUE;

	for (UINT i = 0; i < uCount; i++)
	{
		HTREEITEM hTreeItem;

		TVITEM tvi;
		tvi.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
		tvi.pszText = (LPWSTR)Items[i].wstrCaption.c_str();
		tvi.cChildren = Items[i].usNodeCount > 0 ? 1 : 0;
		tvi.lParam = (LPARAM)m_pXPIGUI;
		/****/
		TVINSERTSTRUCT tvis;
		tvis.hParent = hItemParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item = tvi;

		if ((hTreeItem = TreeView_InsertItem(hTree, &tvis)) == NULL)
		{
#ifdef _DEBUG
			pLog->Write(LOG_WF_ERROR, L"Couldn't create tree item \"%s\".", Items[i].wstrCaption);
#endif
			return FALSE;
		}

		if (Items[i].lpwszTemplate != NULL && Items[i].DlgProc != NULL)
		{
			HWND hWnd = CreateDialogParamW(m_pXPIGUI->hInstance, Items[i].lpwszTemplate, m_hDialog, Items[i].DlgProc, (LPARAM)m_pXPIGUI);
			if (hWnd == NULL)
			{
#ifdef _DEBUG
				pLog->Write(LOG_WF_ERROR, L"Couldn't create window for tree item \"%s\".", Items[i].wstrCaption);
#endif
				return FALSE;
			}
			m_mTreeItems[hTreeItem] = hWnd;
		}

		// recursively populate the tree nodes (function ends if there are no children set)
		if (!PopulateTree(Items[i].lpNodes, Items[i].usNodeCount, hTreeItem))
			return FALSE;
	}

	return TRUE;
}

BOOL CMainSettings::OnCreate()
{
	// apply tab child style
	EnableThemeDialogTexture(m_hDialog, ETDT_ENABLETAB);

	TREE_INFO TreeItems[] =
	{
		{ pStrings->Get(IDS_SETTINGS_PACKET), MAKEINTRESOURCEW(IDD_SETTINGSPACKET), CSettingsPacket::DialogProc, NULL, 0 }
	};

	if (!PopulateTree(TreeItems, _countof(TreeItems), NULL))
		return FALSE;

	HWND hTree = GetDlgItem(m_hDialog, IDC_SETTINGSTREE);

	// sanity check
	if (hTree == NULL)
		return FALSE;

	if (_countof(TreeItems) > 0)
		if (_countof(TreeItems) != 1 || TreeItems[0].usNodeCount != 0)
			SetWindowLong(hTree, GWL_STYLE, GetWindowLong(hTree, GWL_STYLE) | TVS_LINESATROOT);

	return TRUE;
}

VOID CMainSettings::OnSize(__in WORD wHeight, __in WORD wWidth)
{
	SetWindowPos(GetDlgItem(m_hDialog, IDC_SETTINGSTREE), NULL, 0, 0, UI_TREE_WIDTH, wHeight - UI_TREE_SUBHEIGHT, SWP_NOMOVE);

	for (TREEITEM_MAP::iterator i = m_mTreeItems.begin(); i != m_mTreeItems.end(); ++i)
		if (i->second != NULL)
			SetWindowPos(i->second, NULL, 0, 0, wWidth - UI_ITEM_SUBWIDTH, wHeight - UI_ITEM_SUBHEIGHT, SWP_NOMOVE);
}

VOID CMainSettings::RelayToChildren(__in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	for (TREEITEM_MAP::iterator i = m_mTreeItems.begin(); i != m_mTreeItems.end(); ++i)
		if (i->second != NULL)
			PostMessage(i->second, uMessage, wParam, lParam);
}

VOID CMainSettings::OnTreeSelection(__in LPNMTREEVIEW lpnmtv)
{
	HWND hWnd = m_mTreeItems[lpnmtv->itemOld.hItem];

	// hide old selected child item
	if (hWnd != NULL)
		ShowWindow(hWnd, SW_HIDE);

	hWnd = m_mTreeItems[lpnmtv->itemNew.hItem];

	// show new selected child item
	if (hWnd != NULL)
		ShowWindow(hWnd, SW_SHOW);
	else
	{
		// a child group was selected, show the first child item in the group
		hWnd = m_mTreeItems[TreeView_GetChild(GetDlgItem(m_hDialog, IDC_SETTINGSTREE), lpnmtv->itemNew.hItem)];

		if (hWnd != NULL)
			ShowWindow(hWnd, SW_SHOW);
	}
}

INT_PTR CALLBACK CMainSettings::DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		if (!(new CMainSettings(hDialog, (PXPIGUI)lParam))->OnCreate())
		{
			CMainWindow::NotifyErrorUI();
			EndDialog(GetParent(hDialog), EXIT_FAILURE);
		}
		break;

	case WM_UPDATEOPCODE:
		GetClassInstance<CMainSettings>(hDialog)->RelayToChildren(uMessage, wParam, lParam);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_SETTINGSTREE && ((LPNMHDR)lParam)->code == TVN_SELCHANGED)
			GetClassInstance<CMainSettings>(hDialog)->OnTreeSelection((LPNMTREEVIEW)lParam);
		break;

	case WM_SIZE:
		GetClassInstance<CMainSettings>(hDialog)->OnSize(HIWORD(lParam), LOWORD(lParam));
		break;

	case WM_DESTROY:
		delete GetClassInstance<CMainSettings>(hDialog);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}
