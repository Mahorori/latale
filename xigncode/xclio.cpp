#include "stdafx.h"

#include "xclio.h"

namespace xclio
{
	/*static DWORD dword_100D6DE0, dword_100D6DE8;

	struct STRUCT_A
	{
		STRUCT_B **p;
		int dw4;

		// sub_1005A8D6
		STRUCT_A(int a2, __in STRUCT_B **a3)
		{
			p = 0;
			dw4 = a2;
			if (!a3)
				_invalid_parameter_noinfo();
			*p = *a3;
		}
	};

	struct STRUCT_B
	{
		HANDLE hEvent;
		DWORD dw20;
		CRITICAL_SECTION CriticalSection;

		int sub_10028930(int a2)
		{
			int v3; // [sp+4h] [bp-8h]@0
			int v4; // [sp+8h] [bp-4h]@0

			sub_1005A8D6(dw20, this);
			return sub_10028900(v3, v4, a2);
		}

		int sub_10028959(int a2, void *a3, size_t a4, int a5, int a6, int a7)
		{
			int v7; // esi@1
			int result; // eax@2
			int v9; // [sp+Ch] [bp-28h]@3
			void *v10; // [sp+10h] [bp-24h]@3
			int v11; // [sp+14h] [bp-20h]@3
			int v12; // [sp+18h] [bp-1Ch]@3
			int v13; // [sp+1Ch] [bp-18h]@3
			void *v15; // [sp+24h] [bp-10h]@1

			EnterCriticalSection(&CriticalSection);
			v15 = (void *)sub_10056823(a4);
			LeaveCriticalSection(&CriticalSection);
			if (v15)
			{
				memcpy(v15, a3, a4);
				v9 = a2;
				v10 = v15;
				v11 = a5;
				v12 = a6;
				v13 = a7;
				EnterCriticalSection(&CriticalSection);
				sub_10028930(&v9);
				LeaveCriticalSection(&CriticalSection);
				SetEvent(hEvent);
				result = 0;
			}
			else
			{
				result = -536805373;
			}
			return result;
		}
	};

	DWORD sub_10023FFD()
	{
		DWORD var_4;
		DWORD dwRet;

		InterlockedIncrement(&dword_100D6DE0);

		var_4 = 0;

		if (InterlockedCompareExchange(&dword_100D6DE8, 0, 0) == 0)
		{
			SetLastError(0x0E0190304);
			dwRet = 0;
		}
		else
		{

		}

		InterlockedDecrement(&dword_100D6DE0);

		return dwRet;
	}*/
}