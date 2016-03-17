#include "stdafx.h"

#include "structs.h"
#include "functions.h"
#include "pointers.h"

pfnSPListItemBase__GetItem SPListItemBase__GetItem = reinterpret_cast<pfnSPListItemBase__GetItem>(0x0047BD20);
pfnSPListItemBase__GetOption SPListItemBase__GetOption = reinterpret_cast<pfnSPListItemBase__GetOption>(0x00667FE0);
pfnCOOL__IsItemCooldown COOL__IsItemCooldown = reinterpret_cast<pfnCOOL__IsItemCooldown>(0x0069D2D0);
pfnSPListItemBase__GetAvailableOptions SPListItemBase__GetAvailableOptions = reinterpret_cast<pfnSPListItemBase__GetAvailableOptions>(0x00667A30);
pfnSPItem__GetOptionCount SPItem__GetOptionCount = reinterpret_cast<pfnSPItem__GetOptionCount>(0x00675B00);

std::string getItemName(__in unsigned int uItemId)
{
	SPListItemBase *p = SPListItemBase__GetInstance();
	DWORD dwItemId = uItemId;
	SPItem *pItem;

	if (p)
	{
		SPListItemBase__GetItem(&p->lpECX, &pItem, &dwItemId);
		if (pItem != p->lpECX && pItem->uItemId == uItemId)
		{
			return pItem->pBaseItem->name.c_str();
		}
	}

	return std::string("");
}

std::string getOptionText(__in WORD wOption)
{
	SPListItemBase	*pListItemBase;
	SPItemOption	*pItemOption;

	pListItemBase = SPListItemBase__GetInstance();
	if (pListItemBase)
	{
		SPItemOption *pItemOption = SPListItemBase__GetOption(pListItemBase, wOption);
		if (pItemOption)
			return pItemOption->desc1;
	}

	return std::string("");
}