#ifndef MAPLE_HOOKS_H_
#define MAPLE_HOOKS_H_

#include "CPacket.hpp"

extern HWND GetXPIWindow();

// hook functions
extern VOID __fastcall Encode1(__inout CPacket* pPacket, __in DWORD dwEDX, __in BYTE b);
extern VOID __fastcall Encode2(__inout CPacket* pPacket, __in DWORD dwEDX, __in WORD w);
extern VOID __fastcall Encode4(__inout CPacket* pPacket, __in DWORD dwEDX, __in DWORD dw);
extern VOID __fastcall Encode8(__inout CPacket* pPacket, __in DWORD dwEDX, __in ULONGLONG ull);
extern VOID __fastcall EncodeBuffer(__inout CPacket* pPacket, __in DWORD dwEDX, __in_bcount(uLength) PBYTE pb, __in UINT uLength);
extern VOID __fastcall SendPacket(__in LPVOID thisSocket, __in DWORD dwEDX, __in CPacket* pPacket);

extern VOID __fastcall Decode1(__inout CPacket* pPacket, __in DWORD dwEDX, __inout BYTE *pb);
extern VOID __fastcall Decode2(__inout CPacket* pPacket, __in DWORD dwEDX, __inout WORD *pw);
extern VOID __fastcall Decode4(__inout CPacket* pPacket, __in DWORD dwEDX, __inout DWORD *pdw);
extern VOID __fastcall Decode8(__inout CPacket* pPacket, __in DWORD dwEDX, __inout ULONGLONG *pull);
extern VOID __fastcall DecodeStr(__inout CPacket* pPacket, __in DWORD dwEDX, __out_bcount(uLength) LPCSTR lpcsz, __in UINT uLength);
extern VOID __fastcall DecodeBuffer(__inout CPacket* pPacket, __in DWORD dwEDX, __out_bcount(uLength) PBYTE pb, __in UINT uLength);
extern VOID __cdecl ProcessPacket(__in CPacket* pPacket);

enum OFFTYPE { OFF_NONE = 0, OFF_ADD, OFF_SUB, OFF_PTR, OFF_JMP, OFF_CALL };

typedef struct _MAPLE_FUNCTION
{
	LPCWSTR lpcwszName;
	PVOID   pTarget;
	OFFTYPE OffsetType;
	UINT    uOffset;
	LPCWSTR lpcwszSignature;
} MAPLE_FUNCTION, far *LPMAPLE_FUNCTION, near *PMAPLE_FUNCTION;

typedef struct _MAPLE_HOOK
{
	PVOID           pHook;
	MAPLE_FUNCTION  Function;
} MAPLE_HOOK, far *LPMAPLE_HOOK, near *PMAPLE_HOOK;

const MAPLE_HOOK MapleHooks[] =
{
	// send functions
	{ Encode1,			{ L"CPacket::Encode1",		&_Encode1,			OFF_NONE,	0,	L"55 8B EC 0F B7 ?? 04 8A ?? 08 88 ?? ?? 04 66" } },
	{ Encode2,			{ L"CPacket::Encode2",		&_Encode2,			OFF_NONE,	0,	L"55 8B EC 0F B7 ?? 04 66 ?? 45 08 66 89 ?? ?? 04 66" } },
	{ Encode4,			{ L"CPacket::Encode4",		&_Encode4,			OFF_NONE,	0,	L"55 8B EC 0F B7 ?? 04 8B 45 08 89 ?? ?? 04 66 83 41 04 04" } },
	{ Encode8,			{ L"CPacket::Encode8",		&_Encode8,			OFF_NONE,	0,	L"55 8B EC 0F B7 ?? 04 8B 45 08 89 ?? ?? 04 8B 45 0C 89 ?? ?? 08 66" } },
	{ EncodeBuffer,		{ L"CPacket::EncodeBuffer",	&_EncodeBuffer,		OFF_NONE,	0,	L"55 8B EC 56 66 8B ?? 0C 57 0F B7 ?? 50 FF 75 08 8B" } },
	{ SendPacket,		{ L"CSocket::SendPacket",	&_SendPacket,		OFF_SUB,	0x2D,	L"8B ?? 8B ?? 08 8D ?? ?? ?? FF FF E8 ?? ?? ?? ?? 0F B7 ?? 04 66 ?? ?? 04 0F B7" } },
	// recv functions
	{ Decode1,			{ L"CPacket::Decode1",		&_Decode1,			OFF_NONE,	0,	L"55 8B EC 0F B7 ?? 8A ?? ?? 04 8B 45 08 88 ?? 66" } },
	{ Decode2,			{ L"CPacket::Decode2",		&_Decode2,			OFF_NONE,	0,	L"55 8B EC 0F B7 ?? 66 ?? ?? ?? 04 8B 45 08 66 89 ?? 66" } },
	{ Decode4,			{ L"CPacket::Decode4",		&_Decode4,			OFF_NONE,	0,	L"55 8B EC 0F B7 ?? 8B ?? ?? 04 8B 45 08 89 ?? 66" } },
	{ Decode8,			{ L"CPacket::Decode8",		&_Decode8,			OFF_NONE,	0,	L"55 8B EC 8B ?? 08 ?? 0F B7 ?? 8B ?? ?? 04 89" } },
	{ DecodeStr,		{ L"CPacket::DecodeStr",	&_DecodeStr,		OFF_NONE,	0,	L"55 8B EC 56 66 8B 75 0C 57 0F B7 C6 50 8B F9 0F B7 07 83 C0 04 03 C7 50 FF 75 08 E8 ?? 6? 02 00" } },
	{ DecodeBuffer,		{ L"CPacket::DecodeBuffer",	&_DecodeBuffer,		OFF_NONE,	0,	L"55 8B EC 56 66 8B 75 0C 57 0F B7 C6 50 8B F9 0F B7 07 83 C0 04 03 C7 50 FF 75 08 E8 ?? 5? 02 00" } },
	{ ProcessPacket,	{ L"ProcessPacket",			&_ProcessPacket,	OFF_NONE,	0,	L"55 8B EC 8B 4D 08 8B 41 08 66 83 01 04 3D" } },
};

#endif // MAPLE_HOOKS_H_
