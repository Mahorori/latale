#include "stdafx.h"

#include "CHookManager.hpp"
#include "CLog.hpp"
#include "extvars.hpp"

CHookManager::CHookManager()
{
	m_bEnabled = FALSE;
}

CHookManager::~CHookManager()
{
	Disable();
}

VOID CHookManager::Add(__in PVOID* ppv, __in PVOID pDetour)
{
	m_Hooks[ppv] = pDetour;

	if (Disable())
		Install();
}

VOID CHookManager::Remove(__in PVOID* ppv)
{
	m_Hooks.erase(ppv);

	if (Disable())
		Install();
}

BOOL CHookManager::Set(__in BOOL bInstall)
{
	DETOUR_FUNC DetourSet;
	LONG        lError = NO_ERROR;

	if (m_bEnabled == bInstall || m_Hooks.empty())
		return FALSE;

	if (DetourTransactionBegin() != NO_ERROR)
		return FALSE;

	DetourSet = bInstall ? DetourAttach : DetourDetach;

	if (DetourUpdateThread(GetCurrentThread()) == NO_ERROR)
	{
		foreach(HOOK_MAP::iterator::value_type& i, m_Hooks)
		{
			lError = DetourSet(i.first, i.second);
			if (lError != NO_ERROR)
			{
#ifdef _DEBUG
				pLog->Write(LOG_WF_ERROR, L"CHookManager::Set ADDRESS = 0x%08X.", *i.first);
				pLog->Write(LOG_WF_ERROR, L"CHookManager::Set Error = 0x%08X.", lError);
#endif
				break;
			}
		}
		if (lError == NO_ERROR)
		{
			if (DetourTransactionCommit() == NO_ERROR)
			{
				m_bEnabled = bInstall;
				return TRUE;
			}
		}
	}

	DetourTransactionAbort();
	return FALSE;
}

inline BOOL CHookManager::Install()
{
	return Set(TRUE);
}

inline BOOL CHookManager::Disable()
{
	return Set(FALSE);
}
