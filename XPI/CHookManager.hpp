#ifndef CHOOK_MANAGER_HPP_
#define CHOOK_MANAGER_HPP_

#include <map>
#include <boost/foreach.hpp>

#include <detours.h>

#pragma  comment(lib, "detours")

#ifndef foreach
#define foreach BOOST_FOREACH
#endif

typedef LONG(WINAPI * DETOUR_FUNC)(__in PVOID* ppv, __in PVOID pDetour);
typedef std::map<PVOID*, PVOID> HOOK_MAP;

class CHookManager
{
private:
	HOOK_MAP m_Hooks;
	BOOL m_bEnabled;
	BOOL Set(__in BOOL bInstall);
public:
	CHookManager();
	~CHookManager();
	VOID Add(__in PVOID* ppv, __in PVOID pDetour);
	VOID Remove(__in PVOID* ppv);
	BOOL Install();
	BOOL Disable();
};

#endif // CHOOK_MANAGER_HPP_
