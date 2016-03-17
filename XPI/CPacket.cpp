#include "stdafx.h"

#include "CPacket.hpp"

/*********************
* SENDING FUNCTIONS *
*********************/

VOID(__thiscall * _Encode1)(__inout CPacket* pPacket, __in BYTE b) = NULL;
VOID(__thiscall * _Encode2)(__inout CPacket* pPacket, __in WORD w) = NULL;
VOID(__thiscall * _Encode4)(__inout CPacket* pPacket, __in DWORD dw) = NULL;
VOID(__thiscall * _Encode8)(__inout CPacket* pPacket, __in ULONGLONG ull) = NULL;
VOID(__thiscall * _EncodeStr)(__inout CPacket* pPacket, __in LPCSTR lpcsz) = NULL;
VOID(__thiscall * _EncodeBuffer)(__inout CPacket* pPacket, __in_bcount(uLength) LPBYTE pb, __in UINT uLength) = NULL;
VOID(__thiscall * _SendPacket)(__in LPVOID pSocket, __in CPacket* pPacket) = NULL;

/***********************
* RECIEVING FUNCTIONS *
***********************/

VOID(__thiscall * _Decode1)(__inout CPacket* pPacket, __inout BYTE *pb) = NULL;
VOID(__thiscall * _Decode2)(__inout CPacket* pPacket, __inout WORD *pw) = NULL;
VOID(__thiscall * _Decode4)(__inout CPacket* pPacket, __inout DWORD *pdw) = NULL;
VOID(__thiscall * _Decode8)(__inout CPacket* pPacket, __inout ULONGLONG *pull) = NULL;
VOID(__thiscall * _DecodeStr)(__inout CPacket* pPacket, __out_bcount(uLength) LPCSTR lpcsz, __in UINT uLength) = NULL;
VOID(__thiscall * _DecodeBuffer)(__inout CPacket* pPacket, __out_bcount(uLength) PBYTE pb, __in UINT uLength) = NULL;
VOID(__cdecl * _ProcessPacket)(__in CPacket* pPacket) = NULL;

CPacket::CPacket()
{
	memset(m_aBuff, 0, BUF_SIZE);
	m_uLength = 4;
	m_uOffset = 4;
	m_uCrypted = 0;
}

CPacket::~CPacket()
{
	// no pointer
}

VOID CPacket::write1(__in BYTE b)
{
	// m_aBuff[m_uLength] = b;
	_Encode1(this, b);
}

VOID CPacket::write2(__in WORD w)
{
	_Encode2(this, w);
}

VOID CPacket::write4(__in DWORD dw)
{
	_Encode4(this, dw);
}

VOID CPacket::write8(__in ULONGLONG ull)
{
	_Encode8(this, ull);
}

VOID CPacket::write_buf(__in_bcount(uLength) LPBYTE pb, __in UINT uLength)
{
	_EncodeBuffer(this, pb, uLength);
}

VOID CPacket::send(__in LPVOID pSocket)
{
	_SendPacket(pSocket, this);
}
