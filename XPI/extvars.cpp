#include "stdafx.h"

#include "extvars.hpp"

CResourceString*  pStrings      = NULL;
#ifdef _DEBUG
CLog*             pLog          = NULL;
#endif
CInstanceManager* pInstances		= NULL;
CHookManager*     pHookManager		= NULL;
OPCODE_MAP*       pOpcodeInfo		= NULL;
LPVOID			  pClientSocket		= NULL;
BOOL              bLogging			= TRUE;
BOOL              bAutoscroll		= FALSE;
PVOID             pMapleBase		= NULL;
DWORD             dwMapleSize		= 0;
/***/
boost::object_pool<CMaplePacket>* pPacketPool = NULL;
