// xigncode.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

#define XIGNAPI __stdcall

typedef unsigned int (XIGNAPI* _XignCode_heartbeat_callback_t)(void *reserved1, void *reserved2, unsigned char* response, unsigned int size, void *reserved3);
typedef unsigned int (XIGNAPI * _XignCode_error_callback_t)(unsigned int error_code, unsigned int param_size, void* param, void* unknown);

void XIGNAPI xc3_heartbeat_callback(void *reserved1, void *reserved2, unsigned char* response, unsigned int size, void *reserved3)
{
}

long XIGNAPI _XignCode_make_response(unsigned char* request, unsigned int size, _XignCode_heartbeat_callback_t response_callback, int unknown)
{
	typedef unsigned int (XIGNAPI * _XignCode_a_t)(unsigned char* request, unsigned int size, _XignCode_heartbeat_callback_t response_callback, int unknown);

	// cc0, cc4
	static LONG u_04D0ECC0 = 0, u_04D0ECC4 = 0;
	static _XignCode_a_t call = nullptr;

	unsigned long local_value = u_04D0ECC0;
	unsigned int ret = 0;

	InterlockedIncrement(&u_04D0ECC0);

	if (InterlockedCompareExchange(&u_04D0ECC4, 0, 0) == 0)
	{
		// 04CF8A85
		if (call)
		{
			ret = call(request, size, response_callback, unknown);
		}
		else
		{
			call = 0;
			ret = call(request, size, response_callback, unknown);
		}
	}

	// 04CF8ABD or 04CF8B15

	InterlockedDecrement(&u_04D0ECC0);

	return 0;
}

long XIGNAPI _XignCode_make_response2(unsigned char* request, unsigned int size, _XignCode_heartbeat_callback_t response_callback, int unknown)
{
	// 08855FFD
	// 08856089

	// ローカル変数 ebp-0D
}