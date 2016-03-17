#include "stdafx.h"

#include "loot.h"
#include "detour.h"

FLOAT *pfDistance = reinterpret_cast<FLOAT*>(0x007D97BC);

BOOL ToggleFullMapLoot(__in BOOL fEnable)
{
	MEMORY_BASIC_INFORMATION mbi;
	DWORD flOldProtect;

	if (VirtualQuery(pfDistance, &mbi, sizeof(mbi)) != sizeof(mbi))
		return FALSE;

	if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &flOldProtect))
		return FALSE;

	*pfDistance = fEnable ? FLT_MAX : 1600;

	return VirtualProtect(mbi.BaseAddress, mbi.RegionSize, flOldProtect, &flOldProtect);
}