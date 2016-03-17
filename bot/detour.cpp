#include "stdafx.h"

#include "detour.h"

#include <detours.h>
#pragma comment( lib, "detours.lib" )

BOOL DetourFunction(__in BOOL fStatus, __inout LPVOID* lppvFunction, __in LPVOID lpvRedirection)
{
	if (DetourTransactionBegin() != NO_ERROR)
		return FALSE;

	if (DetourUpdateThread(GetCurrentThread()) == NO_ERROR)
		if ((fStatus ? DetourAttach : DetourDetach)(lppvFunction, lpvRedirection) == NO_ERROR)
			if (DetourTransactionCommit() == NO_ERROR)
				return TRUE;

	DetourTransactionAbort();

	return FALSE;
}