#include "stdafx.h"

#include "oprst.h"
#include "detour.h"
#include "functions.h"
#include "pointers.h"
#include "structs.h"
#include "COptionRefreshTab.hpp"

BOOL ToggleOptionRstHook(__in BOOL fEnable)
{
	// start
	typedef BOOL(__fastcall * pfnSPWindowOptionRst__Start)(
		__in SPWindowOptionRst *pWindowOptionRst, __in DWORD dwEDX, __in LPVOID lpv1, __in LPVOID lpv2);
	static pfnSPWindowOptionRst__Start SPWindowOptionRst__Start = reinterpret_cast<pfnSPWindowOptionRst__Start>(0x005A9360);

	// set item
	typedef BOOL(__fastcall * pfnSPWindowOptionRst__Set)(__in SPWindowOptionRst *pWindowOptionRst);
	static pfnSPWindowOptionRst__Set SPWindowOptionRst__Set = reinterpret_cast<pfnSPWindowOptionRst__Set>(0x005A9B60);

	// on4 ... is running after option refresh
	typedef void(__fastcall * pfnSPWindowOptionRst__OnUpdate4)(__in SPWindowOptionRst *pWindowOptionRst, __in DWORD dwEDX, __in LPVOID lpv1);
	static pfnSPWindowOptionRst__OnUpdate4 SPWindowOptionRst__OnUpdate4 = reinterpret_cast<pfnSPWindowOptionRst__OnUpdate4>(0x005A9730);

	static pfnSPWindowOptionRst__Start SPWindowOptionRst__Start__Hook = [](
		__in SPWindowOptionRst *pWindowOptionRst, __in DWORD dwEDX, __in LPVOID lpv1, __in LPVOID lpv2) -> BOOL
	{
		if (!pWindowOptionRst->ppItem)
			return TRUE;

		if (pOptionRefreshTab->canStart(*pWindowOptionRst->ppItem))
			SPWindowOptionRst__Start(pWindowOptionRst, dwEDX, lpv1, lpv2);
		else
		{
			HMODULE hModule;

			if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<TCHAR*>(_ReturnAddress()), &hModule))
			{
				if (hModule == GetModuleHandle(NULL))
				{
					SPChat *pChat = SPChat__GetInstance();
					pChat->pBaseclass->ProcessCommand(pChat, SPChat::COMMAND::GREEN_CHAT, "[LATALE_BOT] オプションリフレッシュを開始できません。", 0);
				}
			}
		}

		return TRUE;
	};

	// 手動アイテムセット、オプションリフレッシュ完了時によばれる(成功失敗にかかわらず)。
	static pfnSPWindowOptionRst__Set SPWindowOptionRst__Set__Hook = [](__in SPWindowOptionRst *pWindowOptionRst) -> BOOL
	{
		SPItem **ppItem = pWindowOptionRst->ppItem;

		if (!SPWindowOptionRst__Set(pWindowOptionRst))
			return FALSE;

		if (ppItem != pWindowOptionRst->ppItem)
		{
			// 違うアイテムがセットされた場合、UIをアップデート
			pOptionRefreshTab->OnUpdateItem(*pWindowOptionRst->ppItem);
		}
		else
		{
			// アイテムオプション UIアップデート
			pOptionRefreshTab->OnUpdateOption(*pWindowOptionRst->ppItem);
		}
		return TRUE;
	};

	static pfnSPWindowOptionRst__OnUpdate4 SPWindowOptionRst__OnUpdate4__Hook = [](
		__in SPWindowOptionRst *pWindowOptionRst, __in DWORD dwEDX, __in LPVOID lpv1) -> void
	{
		SPWindowOptionRst__OnUpdate4(pWindowOptionRst, dwEDX, lpv1);

		SPWindowOptionRst__Start__Hook(pWindowOptionRst, 0, 0, 0);
	};

	DetourFunction(fEnable, reinterpret_cast<LPVOID*>(&SPWindowOptionRst__Start), SPWindowOptionRst__Start__Hook);
	DetourFunction(fEnable, reinterpret_cast<LPVOID*>(&SPWindowOptionRst__Set), SPWindowOptionRst__Set__Hook);
	DetourFunction(fEnable, reinterpret_cast<LPVOID*>(&SPWindowOptionRst__OnUpdate4), SPWindowOptionRst__OnUpdate4__Hook);

	return TRUE;
}

DWORD dwHook = 0x005A960E;

void __declspec(naked) option_rst_faster_asm()
{
	__asm
	{
		mov dword ptr [esi + 0x0000028C], 0x00000064
		jmp dword ptr [dwHook]
	}
}

BOOL ToggleOptionRstFaster(__in BOOL fEnable)
{
	return DetourFunction(fEnable, reinterpret_cast<LPVOID*>(&dwHook), option_rst_faster_asm);
}