#include "stdafx.h"

#include "autologin.h"
#include "detour.h"
#include "pointers.h"
#include "structs.h"
#include "CLoginTab.hpp"
#include "CUserData.hpp"
#include <string>

// 00705C30 SPNetworkManager::ConnectToLoginServer
// 004DA640 OnLogin
// 004BC540 playsound
// 00413620->00658F60->00705EF0 SPNetworkManager::Disconnect2
// 00707947 - FF 50 20              - call dword ptr [eax+20]	// this is somethignedjgnerigiregnreng
/*
004DA89C - 8B 48 4C              - mov ecx,[eax+4C]
004DA8A6 - 8B 01                 - mov eax,[ecx]
004DA8A8 - 56                    - push esi
004DA8A9 - 52                    - push edx
004DA8AA - 68 10300002           - push 02003010
004DA8AF - FF 50 20              - call dword ptr [eax+20]
*/

VOID Login(__in std::string id, __in std::string pw)
{
	typedef LONG(__thiscall * pfnSPLoggin__OnReqConnectTry)(__in SPLoggin *lpECX, __in LPCSTR lpcszId, __in LPCSTR lpcszPw);
	pfnSPLoggin__OnReqConnectTry SPLoggin__OnReqConnectTry = reinterpret_cast<pfnSPLoggin__OnReqConnectTry>(0x00678760);

	SPLoggin__OnReqConnectTry(SPLoggin__GetInstance(), id.c_str(), pw.c_str());
}

DWORD WINAPI LoginWithDelay(__in LPVOID lpThreadParameter)
{
	// this is not thread safe but w/e
	SPNetworkManager *pNetworkManager;
	CLoginTab *pLoginTab;
	CUserData *pUserData;

	while (pNetworkManager = SPNetworkManager__GetInstance())
	{
		if (!pNetworkManager->p)
			return FALSE;

		if (pNetworkManager->p->fConnected == FALSE)
		{
			// try to login
			pLoginTab = CLoginTab::getLoginTab();
			if (!pLoginTab)
				return FALSE;

			pUserData = pLoginTab->getUserData();
			if (!pUserData)
				return FALSE;

			Login(pUserData->getId(), pUserData->getPw());
			return TRUE;
		}
		Sleep(100);
	}

	return FALSE;
}

BOOL ToggleAutoLogin(__in BOOL fEnable)
{
	typedef BOOL(__fastcall * pfnSPWindowLogin__Init)(__in SPWindowLogin *lpECX);
	static pfnSPWindowLogin__Init SPWindowLogin__Init = reinterpret_cast<pfnSPWindowLogin__Init>(0x004DB870);

	static pfnSPWindowLogin__Init SPWindowLogin__Init__Hook = [](__in SPWindowLogin *lpECX) -> BOOL
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LoginWithDelay, NULL, 0, NULL);

		return SPWindowLogin__Init(lpECX);
	};

	return DetourFunction(fEnable, reinterpret_cast<LPVOID*>(&SPWindowLogin__Init), SPWindowLogin__Init__Hook);
}


/*
0040E28B - 6A 00                 - push 00 { 00000000 }
0040E28D - 8D 04 90              - lea eax,[eax+edx*4]
0040E290 - 50                    - push eax
0040E291 - 68 10400002           - push 02004010 { 33570832 }
0040E296 - FF 56 20              - call dword ptr [esi+20] // select char
*/

BOOL ToggleAutoSelectChar(__in BOOL fEnable)
{
	typedef BOOL(__fastcall * pfnSPLoggin__AllCharacterLoaded)(__in SPLoggin *pLoggin);
	static pfnSPLoggin__AllCharacterLoaded SPLoggin__AllCharacterLoaded = reinterpret_cast<pfnSPLoggin__AllCharacterLoaded>(0x00655030);

	typedef BOOL(__thiscall * pfnSPLoggin__SelectCharacter)(__in SPLoggin *lpECX, __in INT iIndex, __in LPVOID lp);
	static pfnSPLoggin__SelectCharacter SPLoggin__SelectCharacter = reinterpret_cast<pfnSPLoggin__SelectCharacter>(0x00657220);

	static pfnSPLoggin__AllCharacterLoaded SPLoggin__AllCharacterLoaded__Hook = [](
		__in SPLoggin *pLoggin) -> BOOL
	{
		SPLoggin__AllCharacterLoaded(pLoggin);

		SPNetworkManager *pNetworkManager = SPNetworkManager__GetInstance();
		if (!pNetworkManager)
			return TRUE;

		//
		CLoginTab *pLoginTab = CLoginTab::getLoginTab();
		if (!pLoginTab)
			return TRUE;

		CUserData *pUserData = pLoginTab->getUserData();
		if (!pUserData)
			return FALSE;

		if (strcmp(pNetworkManager->strId.c_str(), pUserData->getId().c_str()) == 0)
		{
			pUserData->loadChars(&pLoggin->vCharacters);
			SPLoggin__SelectCharacter(pLoggin, pUserData->getIndexByName(pLoginTab->getCharName()), NULL);
		}

		return TRUE; // always true
	};

	return DetourFunction(fEnable, reinterpret_cast<LPVOID*>(&SPLoggin__AllCharacterLoaded), SPLoggin__AllCharacterLoaded__Hook);
}