#pragma once

#define BUF_SIZE 0x2000

struct CPacket
{
	unsigned short m_uOffset;
	unsigned short m_uSize; // struct size (buffer len + 4)
	union
	{
		unsigned char m_aBuff[BUF_SIZE];
		struct
		{
			unsigned short m_uLength; // buffer length
			unsigned short m_uCrypted;
			unsigned long m_uHeader;
			unsigned char m_aData[BUF_SIZE - 8];
		};
	};

public:
	CPacket();
	~CPacket();

public:
	VOID write1(__in BYTE b);
	VOID write2(__in WORD b);
	VOID write4(__in DWORD b);
	VOID write8(__in ULONGLONG ull);
	VOID write_buf(__in_bcount(uLength) LPBYTE pb, __in UINT uLength);
	VOID send(__in LPVOID pSocket);
};

extern VOID(__thiscall * _Encode1)(__inout CPacket* pPacket, __in BYTE b);
extern VOID(__thiscall * _Encode2)(__inout CPacket* pPacket, __in WORD w);
extern VOID(__thiscall * _Encode4)(__inout CPacket* pPacket, __in DWORD dw);
extern VOID(__thiscall * _Encode8)(__inout CPacket* pPacket, __in ULONGLONG ull);
extern VOID(__thiscall * _EncodeBuffer)(__inout CPacket* pPacket, __in_bcount(uLength) LPBYTE pb, __in UINT uLength);
extern VOID(__thiscall * _SendPacket)(__in LPVOID pSocket, __in CPacket* oPacket);

extern VOID(__thiscall * _Decode1)(__inout CPacket* pPacket, __inout BYTE *pb);
extern VOID(__thiscall * _Decode2)(__inout CPacket* pPacket, __inout WORD *pw);
extern VOID(__thiscall * _Decode4)(__inout CPacket* pPacket, __inout DWORD *pdw);
extern VOID(__thiscall * _Decode8)(__inout CPacket* pPacket, __inout ULONGLONG *pull);
extern VOID(__thiscall * _DecodeStr)(__inout CPacket* pPacket, __out_bcount(uLength) LPCSTR lpcsz, __in UINT uLength);
extern VOID(__thiscall * _DecodeBuffer)(__inout CPacket* pPacket, __out_bcount(uLength) PBYTE pb, __in UINT uLength);
extern VOID(__cdecl * _ProcessPacket)(__in CPacket* pPacket);