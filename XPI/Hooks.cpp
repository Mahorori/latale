#include "stdafx.h"

#include "Hooks.h"

#include "CInstanceManager.hpp"
#include "CMaplePacket.hpp"
#include "extvars.hpp"
#include "XPIUtilities.hpp"

#include <intrin.h>

#pragma  intrinsic(_ReturnAddress)

VOID __fastcall Encode1(__inout CPacket* pPacket, __in DWORD dwEDX, __in BYTE b)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt == NULL)
	{
		// add instance
		CMAPLEPACKETSTRUCT cmps;

		cmps.pInstance = pPacket;
		cmps.Direction = PACKET_SEND;
		cmps.ulState = 0;
		cmps.lpv = _ReturnAddress();

		pInstances->Add(pPacket, pckt = pPacketPool->construct(&cmps));
	}

	if (pckt)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if (pPacket->m_uLength > (pckt->GetSize() + 4))
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uLength - pckt->GetSize() - 4);

			pckt->Add1(b, _ReturnAddress());
		}
	}

	_Encode1(pPacket, b);
}

VOID __fastcall Encode2(__inout CPacket* pPacket, __in DWORD dwEDX, __in WORD w)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt == NULL)
	{
		// add instance
		CMAPLEPACKETSTRUCT cmps;

		cmps.pInstance = pPacket;
		cmps.Direction = PACKET_SEND;
		cmps.ulState = 0;
		cmps.lpv = _ReturnAddress();

		pInstances->Add(pPacket, pckt = pPacketPool->construct(&cmps));
	}

	if (pckt)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if (pPacket->m_uLength > (pckt->GetSize() + 4))
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uLength - pckt->GetSize() - 4);

			pckt->Add2(w, _ReturnAddress());
		}
	}

	_Encode2(pPacket, w);
}

VOID __fastcall Encode4(__inout CPacket* pPacket, __in DWORD dwEDX, __in DWORD dw)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt == NULL)
	{
		// add instance
		CMAPLEPACKETSTRUCT cmps;

		cmps.pInstance = pPacket;
		cmps.Direction = PACKET_SEND;
		cmps.ulState = 0;
		cmps.lpv = _ReturnAddress();

		pInstances->Add(pPacket, pckt = pPacketPool->construct(&cmps));
	}

	if (pckt)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0 && pPacket->m_uLength != 4)
				pckt->Add4(pPacket->m_uHeader);

			if (pPacket->m_uLength > (pckt->GetSize() + 4))
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uLength - pckt->GetSize() - 4);

			pckt->Add4(dw, _ReturnAddress());
		}
	}

	_Encode4(pPacket, dw);
}

VOID __fastcall Encode8(__inout CPacket* pPacket, __in DWORD dwEDX, __in ULONGLONG ull)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt == NULL)
	{
		// add instance
		CMAPLEPACKETSTRUCT cmps;

		cmps.pInstance = pPacket;
		cmps.Direction = PACKET_SEND;
		cmps.ulState = 0;
		cmps.lpv = _ReturnAddress();

		pInstances->Add(pPacket, pckt = pPacketPool->construct(&cmps));
	}

	if (pckt)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if (pPacket->m_uLength > (pckt->GetSize() + 4))
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uLength - pckt->GetSize() - 4);

			pckt->Add8(ull, _ReturnAddress());
		}
	}

	_Encode8(pPacket, ull);
}

VOID __fastcall EncodeBuffer(__inout CPacket* pPacket, __in DWORD dwEDX, __in_bcount(uLength) PBYTE pb, __in UINT uLength)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt == NULL)
	{
		// add instance
		CMAPLEPACKETSTRUCT cmps;

		cmps.pInstance = pPacket;
		cmps.Direction = PACKET_SEND;
		cmps.ulState = 0;
		cmps.lpv = _ReturnAddress();

		pInstances->Add(pPacket, pckt = pPacketPool->construct(&cmps));
	}

	if (pckt)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if (pPacket->m_uLength > (pckt->GetSize() + 4))
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uLength - pckt->GetSize() - 4);

			pckt->AddBuffer(pb, uLength, _ReturnAddress());
		}
	}

	_EncodeBuffer(pPacket, pb, uLength);
}

VOID __fastcall SendPacket(__in LPVOID thisSocket, __in DWORD dwEDX, __in CPacket* pPacket)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	pClientSocket = thisSocket;

	// send message to main window
	HWND hWnd = GetXPIWindow();

	if (hWnd != NULL)
		PostMessage(hWnd, WM_INJECTREADY, pClientSocket != NULL, 0);

	if (bLogging && pckt == NULL)
	{
		// add instance
		CMAPLEPACKETSTRUCT cmps;

		cmps.pInstance = pPacket;
		cmps.Direction = PACKET_SEND;
		cmps.ulState = 0;
		cmps.lpv = _ReturnAddress();

		pInstances->Add(pPacket, pckt = pPacketPool->construct(&cmps));
	}

	if (pckt != NULL)
	{
		if (bLogging)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if (pPacket->m_uLength > (pckt->GetSize() + 4))
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uLength - pckt->GetSize() - 4);

			if (IsOpcodeBlocked(pPacket->m_uHeader))
			{
				// pckt->SetState(pckt->GetState() | PACKET_BLOCKED);
				pInstances->Remove(pPacket);
				return;
			}

			if (hWnd != NULL)
				PostMessage(hWnd, WM_ADDPACKET, 0, (LPARAM)pckt);
		}

		pInstances->Remove(pPacket);
	}

	_SendPacket(thisSocket, pPacket);
}

VOID __fastcall Decode1(__inout CPacket* pPacket, __in DWORD dwEDX, __inout BYTE *pb)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if ((UINT)(pPacket->m_uOffset - 4) > pckt->GetSize())
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uOffset - pckt->GetSize() - 4);
		}
	}

	_Decode1(pPacket, pb);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
			pckt->Add1(*pb, _ReturnAddress());
	}
}

VOID __fastcall Decode2(__inout CPacket* pPacket, __in DWORD dwEDX, __inout WORD *pw)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if ((UINT)(pPacket->m_uOffset - 4) > pckt->GetSize())
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uOffset - pckt->GetSize() - 4);
		}
	}

	_Decode2(pPacket, pw);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
			pckt->Add2(*pw, _ReturnAddress());
	}
}

VOID __fastcall Decode4(__inout CPacket* pPacket, __in DWORD dwEDX, __inout DWORD *pdw)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if ((UINT)(pPacket->m_uOffset - 4) > pckt->GetSize())
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uOffset - pckt->GetSize() - 4);
		}
	}

	_Decode4(pPacket, pdw);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
			pckt->Add4(*pdw, _ReturnAddress());
	}
}

VOID __fastcall Decode8(__inout CPacket* pPacket, __in DWORD dwEDX, __inout ULONGLONG *pull)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if ((UINT)(pPacket->m_uOffset - 4) > pckt->GetSize())
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uOffset - pckt->GetSize() - 4);
		}
	}

	_Decode8(pPacket, pull);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
			pckt->Add8(*pull, _ReturnAddress());
	}
}

VOID __fastcall DecodeStr(__inout CPacket* pPacket, __in DWORD dwEDX, __out_bcount(uLength) LPCSTR lpcsz, __in UINT uLength)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if ((UINT)(pPacket->m_uOffset - 4) > pckt->GetSize())
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uOffset - pckt->GetSize() - 4);
		}
	}

	_DecodeStr(pPacket, lpcsz, uLength);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
			pckt->AddString(lpcsz, _ReturnAddress());
	}
}

VOID __fastcall DecodeBuffer(__inout CPacket* pPacket, __in DWORD dwEDX, __out_bcount(uLength) PBYTE pb, __in UINT uLength)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if ((UINT)(pPacket->m_uOffset - 4) > pckt->GetSize())
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uOffset - pckt->GetSize() - 4);
		}
	}

	_DecodeBuffer(pPacket, pb, uLength);

	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
			pckt->AddBuffer(pb, uLength, _ReturnAddress());
	}
}

VOID CDECL ProcessPacket(__in CPacket* pPacket)
{
	CMaplePacket* pckt = pInstances->Find(pPacket);

	// add instance
	if (bLogging && pckt == NULL)
	{
		// do not process packet hehe
		if (IsOpcodeBlocked(pPacket->m_uHeader))
			return;

		// create a new CMaplePacket if one does not already exist
		CMAPLEPACKETSTRUCT cmps;

		cmps.pInstance = pPacket;
		cmps.Direction = PACKET_RECV;
		cmps.ulState = 0;
		cmps.lpv = _ReturnAddress();

		pckt = pPacketPool->construct(&cmps);
		pInstances->Add(pPacket, pckt);
	}

	_ProcessPacket(pPacket);

	if ((pckt = pInstances->Find(pPacket)) != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (pckt->GetSize() == 0)
				pckt->Add4(pPacket->m_uHeader);

			if ((UINT)(pPacket->m_uOffset - 4) > pckt->GetSize())
				pckt->AddBuffer(&pPacket->m_aBuff[pckt->GetSize() + 4], pPacket->m_uOffset - pckt->GetSize() - 4);

			HWND hWnd = GetXPIWindow();

			if (hWnd != NULL)
				PostMessage(hWnd, WM_ADDPACKET, 0, (LPARAM)pckt);

		}
		pInstances->Remove(pPacket);
	}
}

HWND GetXPIWindow()
{
	TCHAR szBuffer[MAX_PATH];
	DWORD dwTemp;

	for (HWND hWnd = GetTopWindow(NULL); hWnd != NULL; hWnd = GetNextWindow(hWnd, GW_HWNDNEXT))
	{
		GetWindowThreadProcessId(hWnd, &dwTemp);

		if (dwTemp != GetCurrentProcessId())
			continue;

		if (!GetWindowText(hWnd, szBuffer, MAX_PATH))
			continue;

		if (!_tcscmp(szBuffer, TEXT("XPI")))
			return hWnd;
	}
	return NULL;
}