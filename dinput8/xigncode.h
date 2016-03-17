#pragma once

#define XIGNCODE_BUFSIZE	624
#define XIGNCODE_SIGNATURE	0x345821AB

VOID xigncode_bypass();

namespace buffer
{
	struct request
	{
		unsigned long size;
		unsigned long signature;
		unsigned long key;
		unsigned long operation;
		unsigned char *response;
		unsigned long reserved;
		union
		{
			unsigned long params[(XIGNCODE_BUFSIZE - 24) / sizeof(unsigned long)];
			wchar_t text[(XIGNCODE_BUFSIZE - 24) / sizeof(wchar_t)];
		};
	};

	struct response
	{
		unsigned long size;
		unsigned long signature;
		unsigned long auth;
		unsigned long status;
		union
		{
			unsigned char buffer[XIGNCODE_BUFSIZE - 16];
			unsigned long params[(XIGNCODE_BUFSIZE - 16) / sizeof(unsigned long)];
		};
	};
};