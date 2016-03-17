#include "stdafx.h"

#include "CMaplePacket.hpp"
#include "XPIUtilities.hpp"

volatile LONG lPacketCount = 0;

VOID CMaplePacket::SetCallerTime(__out LPCALLER_TIME lpCT, __in LPVOID lpv)
{
	SYSTEMTIME  LocalTime;

	lpCT->lpCaller = lpv;

	GetLocalTime(&LocalTime);

	if (!SystemTimeToFileTime(&LocalTime, &lpCT->Time))
	{
		lpCT->Time.dwHighDateTime = 0;
		lpCT->Time.dwLowDateTime = 0;
	}
}

CMaplePacket::~CMaplePacket()
{
	foreach(PACKET_MEMBER i, m_dqMembers)
	{
		if (i.Type == MEMBER_STRING)
			delete i.data.str;
		else if (i.Type == MEMBER_BUFFER)
			delete i.data.buff;
	}

	DeleteCriticalSection(&m_CriticalSection);
}

CMaplePacket::CMaplePacket(__in LPCMAPLEPACKETSTRUCT lpCMaplePacketStruct)
{
	SetCallerTime(&m_CallerTime, lpCMaplePacketStruct->lpv);

	m_lID = InterlockedIncrement(&lPacketCount);
	m_pInstance = lpCMaplePacketStruct->pInstance;
	m_Direction = lpCMaplePacketStruct->Direction;
	m_ulState = lpCMaplePacketStruct->ulState;

	InitializeCriticalSection(&m_CriticalSection);
}

VOID CMaplePacket::CopyMembersFrom(__in CMaplePacket* pPacket)
{
	if (pPacket == NULL)
		return;

	foreach(PACKET_MEMBER i, *pPacket->GetMembers())
		m_dqMembers.push_back(i);

	foreach(BYTE b, *pPacket->GetData())
		m_vbData.push_back(b);
}

VOID CMaplePacket::Add1(__in BYTE b, __in LPVOID lpv)
{
	PACKET_MEMBER pm;

	EnterCriticalSection(&m_CriticalSection);

	SetCallerTime(&pm.CallerTime, lpv);
	pm.Type = MEMBER_BYTE;
	pm.data.b = b;

	m_dqMembers.push_back(pm);
	m_vbData.push_back(b);

	LeaveCriticalSection(&m_CriticalSection);
}

VOID CMaplePacket::Add2(__in WORD w, __in LPVOID lpv)
{
	PACKET_MEMBER pm;

	EnterCriticalSection(&m_CriticalSection);

	SetCallerTime(&pm.CallerTime, lpv);
	pm.Type = MEMBER_WORD;
	pm.data.w = w;

	m_dqMembers.push_back(pm);
	m_vbData.push_back(LOBYTE(w));
	m_vbData.push_back(HIBYTE(w));

	LeaveCriticalSection(&m_CriticalSection);
}

VOID CMaplePacket::Add4(__in DWORD dw, __in LPVOID lpv)
{
	PACKET_MEMBER pm;

	EnterCriticalSection(&m_CriticalSection);

	SetCallerTime(&pm.CallerTime, lpv);
	pm.Type = MEMBER_DWORD;
	pm.data.dw = dw;

	m_dqMembers.push_back(pm);
	m_vbData.push_back(LOBYTE(LOWORD(dw)));
	m_vbData.push_back(HIBYTE(LOWORD(dw)));
	m_vbData.push_back(LOBYTE(HIWORD(dw)));
	m_vbData.push_back(HIBYTE(HIWORD(dw)));

	LeaveCriticalSection(&m_CriticalSection);
}

#define HIDWORD(x)	(((x)>>32) & 0xffffffff)
#define LODWORD(x)	((x) & 0xffffffff)

VOID CMaplePacket::Add8(__in ULONGLONG ull, __in LPVOID lpv)
{
	PACKET_MEMBER pm;

	EnterCriticalSection(&m_CriticalSection);

	SetCallerTime(&pm.CallerTime, lpv);
	pm.Type = MEMBER_ULONGLONG;
	pm.data.ull = ull;

	m_dqMembers.push_back(pm);
	m_vbData.push_back(LOBYTE(LOWORD(LODWORD(ull)))); // ‰ºˆÊdword ‰ºˆÊword ‰ºˆÊbyte
	m_vbData.push_back(HIBYTE(LOWORD(LODWORD(ull)))); // ‰ºˆÊdword ‰ºˆÊword ãˆÊbyte
	m_vbData.push_back(LOBYTE(HIWORD(LODWORD(ull)))); // ‰ºˆÊdword ãˆÊword ‰ºˆÊbyte
	m_vbData.push_back(HIBYTE(HIWORD(LODWORD(ull)))); // ‰ºˆÊdword ãˆÊword ãˆÊbyte
	m_vbData.push_back(LOBYTE(LOWORD(HIDWORD(ull)))); // ãˆÊdword ‰ºˆÊword ‰ºˆÊbyte
	m_vbData.push_back(HIBYTE(LOWORD(HIDWORD(ull)))); // ãˆÊdword ‰ºˆÊword ãˆÊbyte
	m_vbData.push_back(LOBYTE(HIWORD(HIDWORD(ull)))); // ãˆÊdword ãˆÊword ‰ºˆÊbyte
	m_vbData.push_back(HIBYTE(HIWORD(HIDWORD(ull)))); // ãˆÊdword ãˆÊword ãˆÊbyte

	LeaveCriticalSection(&m_CriticalSection);
}

VOID CMaplePacket::AddString(__in LPCSTR lpcsz, __in LPVOID lpv)
{
	PACKET_MEMBER pm;

	EnterCriticalSection(&m_CriticalSection);

	SetCallerTime(&pm.CallerTime, lpv);
	pm.Type = MEMBER_STRING;
	pm.data.str = new PACKET_STRING(lpcsz);

	m_dqMembers.push_back(pm);
	size_t unLength;

	StringCchLengthA(lpcsz, STRSAFE_MAX_CCH, &unLength);

	m_vbData.push_back(LOBYTE((WORD)unLength));
	m_vbData.push_back(HIBYTE((WORD)unLength));

	m_vbData.insert(m_vbData.end(), pm.data.str->begin(), pm.data.str->end());

	LeaveCriticalSection(&m_CriticalSection);
}

VOID CMaplePacket::AddBuffer(__in_bcount(uSize) const PBYTE pcbData, __in UINT uSize, __in LPVOID lpv)
{
	PACKET_MEMBER pm;

	EnterCriticalSection(&m_CriticalSection);

	SetCallerTime(&pm.CallerTime, lpv);
	pm.Type = MEMBER_BUFFER;
	pm.data.buff = new PACKET_BUFFER(pcbData, pcbData + uSize);

	m_dqMembers.push_back(pm);
	m_vbData.insert(m_vbData.end(), pm.data.buff->begin(), pm.data.buff->end());

	LeaveCriticalSection(&m_CriticalSection);
}

const CALLER_TIME* CMaplePacket::GetCallerTime() const
{
	return &m_CallerTime;
}

const PACKET_MEMBERS* CMaplePacket::GetMembers() const
{
	return &m_dqMembers;
}

DWORD CMaplePacket::GetOpcode() const
{
	return *(DWORD*)&m_vbData[0];
}

UINT CMaplePacket::GetSize() const
{
	return m_vbData.size();
}

UINT CMaplePacket::GetMemberCount() const
{
	return m_dqMembers.size();
}

ULONG_PTR CMaplePacket::GetState() const
{
	return m_ulState;
}

VOID CMaplePacket::SetState(__in ULONG_PTR ulState)
{
	m_ulState = ulState;
}

LONG CMaplePacket::GetID() const
{
	return m_lID;
}

PACKET_DIRECTION CMaplePacket::GetDirection() const
{
	return m_Direction;
}

std::vector<BYTE>* CMaplePacket::GetData()
{
	return &m_vbData;
}

LPARAM CMaplePacket::GetLParam() const
{
	return m_lParam;
}

VOID CMaplePacket::SetLParam(__in LPARAM lParam)
{
	m_lParam = lParam;
}

WPARAM CMaplePacket::GetWParam() const
{
	return m_wParam;
}

VOID CMaplePacket::SetWParam(__in WPARAM wParam)
{
	m_wParam = wParam;
}