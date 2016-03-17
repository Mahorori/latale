#pragma once

struct SPItem;
struct COOL;
struct SPItemOption;
struct SPListItemBase;
struct SPClientOptionArchive;
struct SPWindowOptionRst;

typedef LPVOID(__thiscall * pfnSPListItemBase__GetItem)(__in LPVOID lpECX, __out SPItem** ppItem, __in DWORD *pdwItemId);
extern pfnSPListItemBase__GetItem SPListItemBase__GetItem;
typedef SPItemOption*(__thiscall * pfnSPListItemBase__GetOption)(__in LPVOID lpECX, __in WORD wOption);
extern pfnSPListItemBase__GetOption SPListItemBase__GetOption;

typedef BOOL(__thiscall * pfnCOOL__IsItemCooldown)(__in COOL* lpECX, __in unsigned long uFlag);
extern pfnCOOL__IsItemCooldown COOL__IsItemCooldown;

typedef int(__thiscall * pfnSPItem__GetOptionCount)(__in SPItem *pItem);
extern pfnSPItem__GetOptionCount SPItem__GetOptionCount;

// not void maybe.
typedef void(__thiscall * pfnSPListItemBase__GetAvailableOptions)(
	__in SPListItemBase *pListItemBase, __in DWORD dwEquipType, 
	__in LPVOID lpv, __in DWORD dwEnchantType, __inout std::vector<SPClientOptionArchive*> *pv);
extern pfnSPListItemBase__GetAvailableOptions SPListItemBase__GetAvailableOptions;

// my funcs
std::string getItemName(__in unsigned int uItemId);