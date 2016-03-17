#ifndef EXT_VARS_HPP_
#define EXT_VARS_HPP_

#include <boost/pool/object_pool.hpp>

#include <map>

// opcode specs
#define OP_ALIAS_MAXCC        17
#define OP_COMMENT_MAXCC      100
//
#define Padding(x) struct { unsigned char __padding##x[(x)]; };

struct OPCODE_INFO
{
	WCHAR     wszAlias[OP_ALIAS_MAXCC];
	WCHAR     wszComment[OP_COMMENT_MAXCC];
	BOOL      bIgnore;
	BOOL      bBlock;
	COLORREF  crColor;
};

typedef std::map<DWORD, OPCODE_INFO> OPCODE_MAP;

// class references for pointer declarations
class CResourceString;
class CLog;
class CInstanceManager;
class CHookManager;
class CMaplePacket;

extern CResourceString*		pStrings;
#ifdef _DEBUG
extern CLog*				pLog;
#endif
extern CInstanceManager*	pInstances;
extern CHookManager*		pHookManager;
extern OPCODE_MAP*			pOpcodeInfo;
extern LPVOID				pClientSocket;
extern BOOL					bLogging;
extern BOOL					bAutoscroll;
extern PVOID				pMapleBase;
extern DWORD				dwMapleSize;
/***/
extern boost::object_pool<CMaplePacket>*  pPacketPool;

#endif // EXT_VARS_HPP_
