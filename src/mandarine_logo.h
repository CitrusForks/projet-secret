/*
	Data for "mandarine_logo" Image
*/

#include <exec/types.h>
#include <intuition/intuition.h>


extern UWORD mandarine_logoPaletteRGB4[16];

extern UWORD mandarine_logoData[6400];

struct Image mandarine_logo =
{
	0, 0,		/* LeftEdge, TopEdge */
	320, 80, 4,	/* Width, Height, Depth */
	mandarine_logoData,	/* ImageData */
	0x001F, 0x0000,	/* PlanePick, PlaneOnOff */
	NULL		/* NextImage */
};
