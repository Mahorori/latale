#ifndef XPI_COLORS_H_
#define XPI_COLORS_H_

typedef struct _XPI_COLOR_INFO
{
	// resource identifier
	WORD      wIcon;
	// resource identifier
	WORD      wName;
	// relative color
	COLORREF  crColor;
} XPI_COLOR_INFO, far *LPXPI_COLOR_INFO, near *PXPI_COLOR_INFO;

enum
{
	XPI_COLOR_BLACK = 0,
	XPI_COLOR_GRAY,
	XPI_COLOR_RED,
	XPI_COLOR_ORANGE,
	XPI_COLOR_GREEN,
	XPI_COLOR_BLUE,
	XPI_COLOR_PURPLE,
	COUNT_XPI_COLOR
};

extern const XPI_COLOR_INFO g_XPIColors[COUNT_XPI_COLOR];

#endif // XPI_COLORS_H_