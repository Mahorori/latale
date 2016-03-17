#include "stdafx.h"

#include "FormattedInject.hpp"

#include "CInstanceManager.hpp"
#include "CMaplePacket.hpp"
#include "Hooks.h"
#include "extvars.hpp"

namespace FormattedInject
{
	VOID DoEncode1(CPacket* pckt, BOOL bHidden, BYTE b)
	{
		if (!bHidden)
			Encode1(pckt, 0, b);
		else
			_Encode1(pckt, b);
	}

	VOID DoEncode2(CPacket* pckt, BOOL bHidden, WORD w)
	{
		if (!bHidden)
			Encode2(pckt, 0, w);
		else
			_Encode2(pckt, w);
	}

	VOID DoEncode4(CPacket* pckt, BOOL bHidden, DWORD dw)
	{
		if (!bHidden)
			Encode4(pckt, 0, dw);
		else
			_Encode4(pckt, dw);
	}

	VOID DoEncode8(CPacket* pckt, BOOL bHidden, ULONGLONG ull)
	{
		if (!bHidden)
			Encode8(pckt, 0, ull);
		else
			_Encode8(pckt, ull);
	}

	VOID DoEncodeString(CPacket* pckt, BOOL bHidden, std::string& str)
	{
		if (!bHidden)
		{
			CMaplePacket* p = pInstances->Find(pckt);

			if (p != NULL)
				p->AddString(str.c_str(), 0);
		}

		_Encode1(pckt, str.length());
		_EncodeBuffer(pckt, (LPBYTE)str.c_str(), str.length());
	}

	VOID DoEncodeBuffer(CPacket* pckt, BOOL bHidden, std::vector<BYTE>& vb)
	{
		if (!bHidden)
			EncodeBuffer(pckt, 0, &vb[0], vb.size());
		else
			_EncodeBuffer(pckt, &vb[0], vb.size());
	}
}