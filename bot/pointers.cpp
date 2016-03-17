#include "stdafx.h"

#include "pointers.h"
#include "structs.h"

SPMainGameManager **SPMainGameManager__msInstance = reinterpret_cast<SPMainGameManager**>(0x0087EC7C);
SPSubGameManager **SPSubGameManager__msInstance = reinterpret_cast<SPSubGameManager**>(0x008851F4);
SPLoggin** SPLoggin__msInstance = reinterpret_cast<SPLoggin**>(0x0087F824);
SPNetworkManager** SPNetworkManager__msInstance = reinterpret_cast<SPNetworkManager**>(0x0087FEB4);
SPListItemBase **SPListItemBase__msInstance = reinterpret_cast<SPListItemBase**>(0x0087F80C);
SPChat **SPChat__msInstance = reinterpret_cast<SPChat**>(0x0087F274);

// 0087EC7C SPSubGameManager? or something related to field

SPMainGameManager* SPMainGameManager__GetInstance()
{
	return *SPMainGameManager__msInstance;
}

SPSubGameManager* SPSubGameManager__GetInstance()
{
	return *SPSubGameManager__msInstance;
}

SPLoggin* SPLoggin__GetInstance()
{
	return *SPLoggin__msInstance;
}

SPNetworkManager* SPNetworkManager__GetInstance()
{
	return *SPNetworkManager__msInstance;
}

SPListItemBase* SPListItemBase__GetInstance()
{
	return *SPListItemBase__msInstance;
}

COOL* COOL__GetInstance()
{
	// skill cooldown? 0069D140
	// item cooldown? 0069D2D0

	typedef COOL*(*pfnCOOL__GetInstance)();
	pfnCOOL__GetInstance COOL__GetInstance = reinterpret_cast<pfnCOOL__GetInstance>(0x0069D250);

	return COOL__GetInstance();
}

SPChat* SPChat__GetInstance()
{
	return *SPChat__msInstance;
}