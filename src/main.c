/*
	Secret (demo) Project
	by Mandarine
*/

#include "includes.prl"
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>

/*
Common
*/
#include <board.h>
#include <ptreplay.h>
#include <ptreplay_protos.h>
#include <ptreplay_pragmas.h>

/*
Routines
*/
#include "bitmap_routines.h"
#include "font_routines.h"
#include "sound_routines.h"

/*
Graphic assets
*/
#include "screen_size.h"
#include "fx_routines.h"
#include "mandarine_logo.h"
#include "font_desc.h"
#include "font_bitmap.h"
#include "color_routines.h"
#include "font_routines.h"
#include "demo_strings.h"
#include "demo_mode_switches.h"

extern UWORD checkerboard_PaletteRGB4[8];
extern UWORD bob_32PaletteRGB4[8];
extern UWORD morph_iso_0PaletteRGB4[8];

/* Music */
struct Library *PTReplayBase;
struct Module *theMod;
UBYTE *mod = NULL;

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
extern struct Custom far custom;

struct View my_view;
struct View *my_old_view;

/* ViewPort 1 */
struct ViewPort view_port1;
struct RasInfo ras_info1;
struct BitMap bit_map1;
struct RastPort rast_port1;
UWORD color_table1[] =
{
	0x000, 0xFFF, 0xDDD, 0xBBB, 0x999, 0x777, 0x555, 0x333,
	0xF00, 0xD00, 0xB00, 0x900, 0x700, 0x500, 0x300, 0x100,
	0x0F0, 0x0D0, 0x0B0, 0x090, 0x070, 0x050, 0x030, 0x010,
	0x00F, 0x00D, 0x00B, 0x009, 0x007, 0x005, 0x003, 0x001
};

UWORD *mlogo_fade_in_pal = NULL;
UWORD *mlogo_fade_out_pal = NULL;

UWORD *zlogo_fade_in_pal = NULL;
UWORD *zlogo_fade_out_pal = NULL;

short *text_width_cache = NULL; 

/* ViewPort 2 */
struct ViewPort view_port2;
struct RasInfo ras_info2;
struct BitMap bit_map2;
struct RastPort rast_port2;

struct RasInfo ras_info2b;
struct BitMap bit_map2b;
struct RastPort rast_port2b;

/* ViewPort 3 */
struct ViewPort view_port3;
struct RasInfo ras_info3;
struct BitMap bit_map3;
struct RastPort rast_port3;

struct BitMap *bitmap_logo = NULL;
struct BitMap *bitmap_checkerboard = NULL;
struct BitMap *bitmap_font = NULL;
struct BitMap *bitmap_bob = NULL;
struct BitMap *bitmap_bob_mask = NULL;
struct  BitMap *bitmap_torus = NULL;
struct  BitMap *bitmap_torus_mask = NULL;

// #define PTREPLAY_MUSIC
struct SoundInfo *background = NULL;

UBYTE is_cpu_a_020(void)
{
	printf("Kickstart v%d, ", SysBase->LibNode.lib_Version);

	if (SysBase->AttnFlags && AFF_68020)
	{
		printf("CPU is a 68020 or higher.\n");
		return 1;
	}

	printf("CPU is a 68000 or 68010.\n");
	return 0;
}

// void list_avail_memory(void)
// {
// 	printf("%d Chip mem, ", AvailMem(MEMF_CHIP));
// 	printf("%d Fast mem, ", AvailMem(MEMF_FAST));
// 	printf("%d Total mem.\n", AvailMem(MEMF_CHIP) + AvailMem(MEMF_FAST));
// }

void initMusic(void)
{
#ifdef PTREPLAY_MUSIC
	if (SysBase->LibNode.lib_Version >= 36)
		if (!AssignPath("Libs","Libs"))
			exit(0); //FIXME // init_conerr((UBYTE *)"Failed to Assign the local Libs drawer. Please copy ptreplay.library into your Libs: drawer.\n");

	if (!(PTReplayBase = OpenLibrary((UBYTE *)"ptreplay.library", 0)))
	{
		exit(0); //FIXME
	}

	mod = load_getchipmem((UBYTE *)"assets/A_SYNTH.mod", 20092);
#else
	background = PrepareSound( "assets/music_loop.8svx" );
	if( !background )
	{
		printf( "Could not prepare the sound effect!\n" );
		exit(0); //FIXME
	}
#endif
}

void playMusic(void)
{
#ifdef PTREPLAY_MUSIC
	if (mod != NULL)
	{
		theMod = PTSetupMod((APTR)mod);
		PTPlay(theMod);
	}
#else
	if (background != NULL)
	{
		PlaySound( background, MAXVOLUME, LEFT0, NORMALRATE, NONSTOP );
		PlaySound( background, MAXVOLUME, RIGHT0, NORMALRATE, NONSTOP );
	}
#endif
}

void print_ascci_art_logo(void)
{
	int i;
	for(i = 0; i < ASCII_ART_MAX_INDEX; i++)
		printf(ascii_art[i]);
}

/* Returns all allocated resources: */
void close_demo(STRPTR message)
{
	int loop;

	/* Free automatically allocated display structures: */
	FreeVPortCopLists( &view_port1 );
	FreeVPortCopLists( &view_port2 );
	FreeVPortCopLists( &view_port3 );
	FreeCprList( my_view.LOFCprList );

	/* Deallocate the display memory, BitPlane for BitPlane: */
	for( loop = 0; loop < DEPTH1; loop++ )
		if( bit_map1.Planes[ loop ] )
			FreeRaster( bit_map1.Planes[ loop ], WIDTH1, HEIGHT1 );
	for( loop = 0; loop < DEPTH2; loop++ )
		if( bit_map2.Planes[ loop ] )
			FreeRaster( bit_map2.Planes[ loop ], WIDTH2, HEIGHT2 );
	for( loop = 0; loop < DEPTH2b; loop++ )
		if( bit_map2b.Planes[ loop ] )
			FreeRaster( bit_map2b.Planes[ loop ], WIDTH2b, (HEIGHT2b + HEIGHT2b_PAD));		
	for( loop = 0; loop < DEPTH3; loop++ )
		if( bit_map3.Planes[ loop ] )
			FreeRaster( bit_map3.Planes[ loop ], WIDTH3, HEIGHT3 );

	/* Deallocate the ColorMap: */
	if( view_port1.ColorMap ) FreeColorMap( view_port1.ColorMap );
	if( view_port2.ColorMap ) FreeColorMap( view_port2.ColorMap );
	if( view_port3.ColorMap ) FreeColorMap( view_port3.ColorMap );

	/* Deallocate various bitmaps */
	free_allocated_bitmap(bitmap_font);
	free_allocated_bitmap(bitmap_bob);
	free_allocated_bitmap(bitmap_bob_mask);
	free_allocated_bitmap(bitmap_torus);
	free_allocated_bitmap(bitmap_torus_mask);

	/* Free fade in/out palettes */
	if (mlogo_fade_in_pal != NULL)
		FreeMem(mlogo_fade_in_pal, sizeof(UWORD) * 8 * 16);
	if (mlogo_fade_out_pal != NULL)
		FreeMem(mlogo_fade_out_pal, sizeof(UWORD) * 8 * 16);
	if (zlogo_fade_in_pal != NULL)
		FreeMem(zlogo_fade_in_pal, sizeof(UWORD) * 8 * 16);
	if (zlogo_fade_out_pal != NULL)
		FreeMem(zlogo_fade_out_pal, sizeof(UWORD) * 8 * 16);

	/* Free text width cache */
	if (text_width_cache != NULL)
		FreeMem(text_width_cache, sizeof(short) * DEMO_STRINGS_MAX_INDEX);

#ifdef PTREPLAY_MUSIC
	/*	Stop music */
	if (mod != NULL)
	{
		PTStop(theMod);
		PTFreeMod(theMod);
		FreeMem(mod, 20092);
	}

	if (PTReplayBase) CloseLibrary(PTReplayBase);
#else
	if (background != NULL)
	{
		StopSound( LEFT0 );
		StopSound( RIGHT0 );
		RemoveSound( background );
	}
#endif
	/* Close the Graphics library: */
	if(GfxBase)
		CloseLibrary((struct Library *)GfxBase);

	/* C Close the Intuition library:  */
	if(IntuitionBase)
		CloseLibrary((struct Library *)IntuitionBase);

	/* Restore the old View: */
	LoadView( my_old_view );

	/* Print the message and leave: */
	printf( "%s\n", message );

	exit(0);
}

void main()
{
	USHORT loop, palette_idx;
	UWORD palette_fade;
	ULONG tmp_col;
	UWORD demo_string_index;
	UBYTE mode_switch, ubob_figure, text_switch;
	UWORD counter_before_next_text, text_duration;
	BOOL text_draw_switch = FALSE;
	BOOL logo_switch = FALSE;
	BOOL scrolltext_switch = FALSE;
	short text_width = 0;
	UWORD vp3_target_y;
	UBYTE faster_machine;

	/* Open the Intuition library: */
	IntuitionBase = (struct IntuitionBase *)
	OpenLibrary( "intuition.library", 0 );
	if( !IntuitionBase )
		close_demo( "Could NOT open the Intuition library!" );

	/* Open the Graphics library: */
	GfxBase = (struct GfxBase *)
	OpenLibrary( "graphics.library", 0 );
	if( !GfxBase )
		close_demo( "Could NOT open the Graphics library!" );

	faster_machine = is_cpu_a_020();

	print_ascci_art_logo();

	/* Save the current View, so we can restore it later: */
	my_old_view = GfxBase->ActiView;

	/* Prepare the View structure, and give it a pointer to */
	/* the first ViewPort:                                  */
	InitView( &my_view );
	my_view.ViewPort = &view_port1;

	/* 2. Prepare the ViewPort structures, and set some important values: */

	/* ViewPort 1 */
	InitVPort( &view_port1 );
	view_port1.DWidth = DISPL_WIDTH1;      /* Set the width.                */
	view_port1.DHeight = DISPL_HEIGHT1;    /* Set the height.               */
	view_port1.DxOffset = 0;         /* X position.                   */
	view_port1.DyOffset = 0;         /* Y position.                   */
	view_port1.RasInfo = &ras_info1; /* Give it a pointer to RasInfo. */
	view_port1.Modes = NULL;         /* Low resolution.               */
	view_port1.Next = &view_port3;   /* Pointer to next ViewPort.     */

	/* ViewPort 3 */
	InitVPort( &view_port3 );
	view_port3.DWidth = WIDTH3;      /* Set the width.                */
	view_port3.DHeight = DISPL_HEIGHT3;    /* Set the height.               */
	view_port3.DxOffset = 0;         /* X position.                   */
	view_port3.DyOffset = DISPL_HEIGHT1 + 2; /* Y position (5 lines under).   */
	view_port3.RasInfo = &ras_info3; /* Give it a pointer to RasInfo. */
	view_port3.Modes = NULL;        /* High resolution.              */
	view_port3.Next = &view_port2;          /* Last ViewPort in the list.    */

	/* ViewPort 2 */
	InitVPort( &view_port2 );
	view_port2.DWidth = DISPL_WIDTH2;      /* Set the width.                */
	view_port2.DHeight = DISPL_HEIGHT2;    /* Set the height.               */
	view_port2.DxOffset = WIDTH2 - DISPL_WIDTH2;         /* X position.                   */
	view_port2.DyOffset = DISPL_HEIGHT1 + DISPL_HEIGHT3 + 4; /* Y position (5 lines under).   */
	view_port2.RasInfo = &ras_info2; /* Give it a pointer to RasInfo. */
	view_port2.Modes = DUALPF|PFBA; 
	view_port2.Next = NULL;          /* Last ViewPort in the list.    */

	/* 3. Get a colour map, link it to the ViewPort, and prepare it: */

	/* ViewPort 1 */
	view_port1.ColorMap = (struct ColorMap *) GetColorMap(COLOURS1);
	if( view_port1.ColorMap == NULL )
		close_demo( "Could NOT get a ColorMap!" );

	for( loop = 0; loop < 8; loop++)
		SetRGB4(&view_port1, loop, 0, 0, 0);

	/* ViewPort 2 */
	view_port2.ColorMap = (struct ColorMap *) GetColorMap(COLOURS2 * 4);
	if( view_port2.ColorMap == NULL )
		close_demo( "Could NOT get a ColorMap!" );

	/* ViewPort 3 */
	view_port3.ColorMap = (struct ColorMap *) GetColorMap(COLOURS3);
	if( view_port3.ColorMap == NULL )
		close_demo( "Could NOT get a ColorMap!" );

	for( loop = 0; loop < COLOURS3; loop++)
		SetRGB4(&view_port3, loop, 0, 0, 0);

	/* Prepare the BitMap */
	/* ViewPort 1 */
	InitBitMap( &bit_map1, DEPTH1, WIDTH1, HEIGHT1 );
	/* Allocate memory for the Raster: */ 
	for( loop = 0; loop < DEPTH1; loop++ )
	{
		bit_map1.Planes[ loop ] = (PLANEPTR) AllocRaster( WIDTH1, HEIGHT1 );
		if( bit_map1.Planes[ loop ] == NULL )
			close_demo( "Could NOT allocate enough memory for the raster!" );
	/* Clear the display memory with help of the Blitter: */
		BltClear( bit_map1.Planes[ loop ], RASSIZE( WIDTH1, HEIGHT1 ), 0 );
	}

	/* ViewPort 2 */
	InitBitMap( &bit_map2, DEPTH2, WIDTH2, HEIGHT2 );
	/* Allocate memory for the Raster: */ 
	for( loop = 0; loop < DEPTH2; loop++ )
	{
		bit_map2.Planes[ loop ] = (PLANEPTR) AllocRaster( WIDTH2, HEIGHT2 );
		if( bit_map2.Planes[ loop ] == NULL )
			close_demo( "Could NOT allocate enough memory for the raster!" );
		/* Clear the display memory with help of the Blitter: */
		BltClear( bit_map2.Planes[ loop ], RASSIZE( WIDTH2, HEIGHT2 ), 0 );
	}	

	InitBitMap( &bit_map2b, DEPTH2b, WIDTH2b, (HEIGHT2b + HEIGHT2b_PAD));
	/* Allocate memory for the Raster: */ 
	for( loop = 0; loop < DEPTH2b; loop++ )
	{
		bit_map2b.Planes[ loop ] = (PLANEPTR) AllocRaster( WIDTH2b, (HEIGHT2b + HEIGHT2b_PAD));
		if( bit_map2b.Planes[ loop ] == NULL )
			close_demo( "Could NOT allocate enough memory for the raster!" );
		/* Clear the display memory with help of the Blitter: */
		BltClear( bit_map2b.Planes[ loop ], RASSIZE( WIDTH2b, (HEIGHT2b + HEIGHT2b_PAD)), 0 );
	}

	for( loop = 0; loop < COLOURS2b; loop++)
	{
		tmp_col = bob_32PaletteRGB4[loop]; // RGB4toRGB8(bob_32PaletteRGB4[loop]);
		// tmp_col = addRGB8Colors(tmp_col, COLOUR_PURPLE);
		// tmp_col = RGB8toRGB4(tmp_col);

		SetRGB4(&view_port2, (COLOURS2 * 2) + loop, (tmp_col & 0x0f00) >> 8, (tmp_col & 0x00f0) >> 4, tmp_col & 0x000f);
	}

	/* ViewPort 3 */
	InitBitMap( &bit_map3, DEPTH3, WIDTH3, HEIGHT3 );
	/* Allocate memory for the Raster: */ 
	for( loop = 0; loop < DEPTH3; loop++ )
	{
		bit_map3.Planes[ loop ] = (PLANEPTR) AllocRaster( WIDTH3, HEIGHT3 );
		if( bit_map3.Planes[ loop ] == NULL )
			close_demo( "Could NOT allocate enough memory for the raster!" );
		/* Clear the display memory with help of the Blitter: */
		BltClear( bit_map3.Planes[ loop ], RASSIZE( WIDTH3, HEIGHT3 ), 0 );
	}	

	/* Prepare the RasInfo structure */

	/* ViewPort 1 */
	ras_info1.BitMap = &bit_map1; /* Pointer to the BitMap structure.  */
	ras_info1.RxOffset = 0;       /* The top left corner of the Raster */
	ras_info1.RyOffset = 0;       /* should be at the top left corner  */
	              /* of the display.                   */
	ras_info1.Next = NULL;        /* Single playfield - only one       */
	              /* RasInfo structure is necessary.   */

	/* ViewPort 2 */
	ras_info2.BitMap = &bit_map2; /* Pointer to the BitMap structure.  */
	ras_info2.RxOffset = 0;       /* The top left corner of the Raster */
	ras_info2.RyOffset = 0;       /* should be at the top left corner of the display.                   */
	ras_info2.Next = &ras_info2b;        /* Dual playfield  */

	ras_info2b.BitMap = &bit_map2b; /* Pointer to the BitMap structure.  */
	ras_info2b.RxOffset = 0;       /* The top left corner of the Raster */
	ras_info2b.RyOffset = 0;       /* should be at the top left corner of the display.                   */
	ras_info2b.Next = NULL;        /* Dual playfield  */	

	/* ViewPort 3 */
	ras_info3.BitMap = &bit_map3; /* Pointer to the BitMap structure.  */
	ras_info3.RxOffset = 0;       /* The top left corner of the Raster */
	ras_info3.RyOffset = 0;       /* should be at the top left corner  */
	              /* of the display.                   */
	ras_info3.Next = NULL;        /* Single playfield - only one       */
	              /* RasInfo structure is necessary.   */	


	/* Prepare the RastPort, and give it a pointer to the BitMap. */
	/* ViewPort 1 */
	InitRastPort( &rast_port1 );
	rast_port1.BitMap = &bit_map1;

	/* ViewPort 2 */
	InitRastPort( &rast_port2 );
	rast_port2.BitMap = &bit_map2;
	InitRastPort( &rast_port2b );
	rast_port2b.BitMap = &bit_map2b;	

	/* ViewPort 3 */
	InitRastPort( &rast_port3 );
	rast_port3.BitMap = &bit_map3;	

	/* Create the display */
	MakeVPort(&my_view, &view_port1); /* Prepare ViewPort 1 */
	MakeVPort(&my_view, &view_port2); /* Prepare ViewPort 2 */
	MakeVPort(&my_view, &view_port3); /* Prepare ViewPort 2 */

	/*	Load the assets */
	drawMandarineLogo(&bit_map1, 0);
	WaitBlit();
	free_allocated_bitmap(bitmap_logo);

	/*	Prepare the font */
	/*	Load the bitmap */
	loadTextWriterFont();

	/*	Precalculate the width (in pixels) of each string */
	text_width_cache = AllocMem(sizeof(short) * DEMO_STRINGS_MAX_INDEX, MEMF_CLEAR);
	for(loop = 0; loop < DEMO_STRINGS_MAX_INDEX; loop++)
	{
		do
		{
			text_width = font_get_string_width((const char *)&tiny_font_glyph, (const short *)&tiny_font_x_pos, (UBYTE *)demo_string[loop]);
		}
		while(text_width < 0);

		text_width_cache[loop] = text_width;
	}

	loadBobBitmaps();
	SetAPen(&rast_port2, 0);
	RectFill(&rast_port2, 0, 0, WIDTH2 - 1, HEIGHT2 - 1);
	drawCheckerboard(&bit_map2, &rast_port2);
	WaitBlit();
	free_allocated_bitmap(bitmap_checkerboard);

	initMusic();
	setLogoCopperlist(&view_port1);
	setTextLinerCopperlist(&view_port3);
	setCheckerboardCopperlist(&view_port2);

	WaitTOF();

	MrgCop(&my_view);

	/*	Wait until the DF0: motor stops */
	for(loop = 0; loop < 150; loop++)
		WaitTOF();

	/* 8. Show the new View: */
	LoadView( &my_view );	

	/*	Force the logo screen to the background palette */
	for( loop = 0; loop < COLOURS1; loop++)
		SetRGB4(&view_port1, loop, (COLOUR_PURPLE_RGB4 & 0x0f00) >> 8, (COLOUR_PURPLE_RGB4 & 0x00f0) >> 4, COLOUR_PURPLE_RGB4 & 0x000f);

	/*	Improve the Mandarine's logo palette */
	for( loop = 0; loop < COLOURS1; loop++)
	{
		if(loop == 0)
			tmp_col = RGB8toRGB4(addRGB8Colors(COLOUR_PURPLE, RGB4toRGB8(mandarine_logoPaletteRGB4[loop])));
		else
			tmp_col = RGB8toRGB4(addRGB8Colors(COLOUR_PURPLE_DARK, RGB4toRGB8(mandarine_logoPaletteRGB4[loop])));
		mandarine_logoPaletteRGB4[loop] = tmp_col;
	}

	/*	Improve the Mandarine's logo palette */
	zoetrope_logoPaletteRGB4[0] = tmp_col = RGB8toRGB4(COLOUR_PURPLE);

	for( loop = 0; loop < COLOURS3; loop++)
	{
		tmp_col = RGB8toRGB4(addRGB8Colors(COLOUR_PURPLE, RGB4toRGB8(font_palRGB4[loop])));
		SetRGB4(&view_port3, loop, (tmp_col & 0x0f00) >> 8, (tmp_col & 0x00f0) >> 4, tmp_col & 0x000f);
	}

	/* Precalc the fade in/out */
	/* Free fade in/out palettes */
	mlogo_fade_in_pal = AllocMem(sizeof(UWORD) * 8 * 16, MEMF_CLEAR);
	for(palette_fade = 0; palette_fade < 16; palette_fade++)
		for(palette_idx = 0; palette_idx < 8; palette_idx++)
			mlogo_fade_in_pal[palette_idx + palette_fade * 8] = mixRGB4Colors(COLOUR_PURPLE_RGB4, mandarine_logoPaletteRGB4[palette_idx], palette_fade);

	mlogo_fade_out_pal = AllocMem(sizeof(UWORD) * 8 * 16, MEMF_CLEAR);
	for(palette_fade = 0; palette_fade < 16; palette_fade++)
		for(palette_idx = 0; palette_idx < 8; palette_idx++)
			mlogo_fade_out_pal[palette_idx + palette_fade * 8] = mixRGB4Colors(mandarine_logoPaletteRGB4[palette_idx], COLOUR_PURPLE_RGB4, palette_fade);

	zlogo_fade_in_pal = AllocMem(sizeof(UWORD) * 8 * 16, MEMF_CLEAR);
	for(palette_fade = 0; palette_fade < 16; palette_fade++)
		for(palette_idx = 0; palette_idx < 8; palette_idx++)
			zlogo_fade_in_pal[palette_idx + palette_fade * 8] = mixRGB4Colors(COLOUR_PURPLE_RGB4, zoetrope_logoPaletteRGB4[palette_idx], palette_fade);

	zlogo_fade_out_pal = AllocMem(sizeof(UWORD) * 8 * 16, MEMF_CLEAR);
	for(palette_fade = 0; palette_fade < 16; palette_fade++)
		for(palette_idx = 0; palette_idx < 8; palette_idx++)
			zlogo_fade_out_pal[palette_idx + palette_fade * 8] = mixRGB4Colors(zoetrope_logoPaletteRGB4[palette_idx], COLOUR_PURPLE_RGB4, palette_fade);	

	playMusic();

	WaitTOF();
	WaitTOF();

	Forbid();
	Disable();
	WaitBlit();

	palette_fade = 0; 
	palette_idx = 0;
	ubob_figure = 0;
	demo_string_index = 0;
	mode_switch = DMODE_SW_INTRO;

	text_switch = 0;
	counter_before_next_text = 0;
	text_duration = 0;
	vp3_target_y = 0;
	
	OFF_SPRITE;
	if (!faster_machine)
		OFF_VBLANK;

	while((*(UBYTE *)0xBFE001) & 0x40)
	{
		scrollLogoBackground();
		updateCheckerboard();

		switch(mode_switch)
		{
			case DMODE_SW_INTRO:
				palette_fade = 0;
				palette_idx = 0;
				mode_switch = DMODE_SW_FADE_IN_MLOGO;
				break;

			case DMODE_SW_FADE_IN_MLOGO:
				LoadRGB4(&view_port1, mlogo_fade_in_pal + (palette_fade << 3), 8);

				palette_fade++;
				if (palette_fade > 15)
				{
					palette_fade = 0;
					mode_switch = DMODE_SW_UBOB;
				}
				break;

			case DMODE_SW_FADE_IN_ZLOGO:
				LoadRGB4(&view_port1, zlogo_fade_in_pal + (palette_fade << 3), 8);

				palette_fade++;
				if (palette_fade > 15)
				{
					palette_fade = 0;
					mode_switch = DMODE_SW_UBOB;
				}			
				break;				

			case DMODE_SW_FADE_OUT_LOGO:
				if (logo_switch)
					LoadRGB4(&view_port1, mlogo_fade_out_pal + (palette_fade << 3), 8);
				else
					LoadRGB4(&view_port1, zlogo_fade_out_pal + (palette_fade << 3), 8);

				palette_fade++;
				if (palette_fade > 15)
				{
					
					palette_fade = 0;
					palette_idx = 0;
					if (logo_switch = swapLogoBackgroundOffset())
						mode_switch = DMODE_SW_FADE_IN_MLOGO;
					else
						mode_switch = DMODE_SW_FADE_IN_ZLOGO;
				}
				break;				

			case DMODE_SW_UBOB:
				if (drawUnlimitedBobs(&rast_port2b, &ubob_figure) == 0 && text_switch == TEXTMODE_SW_WAIT)
				{
					if (ubob_figure == (ubob_figure & 0xFE))
						mode_switch = DMODE_SW_CLEAR_FROM_TOP;
					else
						mode_switch = DMODE_SW_CLEAR_FROM_BOTTOM;
				}
				break;

			case DMODE_SW_CLEAR_FROM_TOP:
				if (clearPlayfieldLineByLineFromTop(&rast_port2b) == 0)
					mode_switch = DMODE_SW_NEXT_UBOB;
				break;

			case DMODE_SW_CLEAR_FROM_BOTTOM:
				if (clearPlayfieldLineByLineFromBottom(&rast_port2b) == 0)
					mode_switch = DMODE_SW_NEXT_UBOB;
				break;

			case DMODE_SW_NEXT_UBOB:
				setNextUnlimitedBobs(&ubob_figure);
				palette_idx = 1;
				mode_switch = DMODE_SW_CHANGE_UBOB_PALETTE;
				break;

			case DMODE_SW_CHANGE_UBOB_PALETTE:
				tmp_col = bob_32PaletteRGB4[(ubob_figure << 2) + palette_idx];
				SetRGB4(&view_port2, (COLOURS2 << 1) + palette_idx, (tmp_col & 0x0f00) >> 8, (tmp_col & 0x00f0) >> 4, tmp_col & 0x000f);
				palette_idx++;
				if (palette_idx > 3)
				{
					if ((ubob_figure & 0x1) == 0x1)
						mode_switch = DMODE_SW_FADE_OUT_LOGO; 
					else
						mode_switch = DMODE_SW_UBOB;
				}

				break;			
		}

		switch(text_switch)
		{

			case TEXTMODE_SW_WAIT:
				counter_before_next_text++;
				if(counter_before_next_text > text_duration
					&& mode_switch != DMODE_SW_CLEAR_FROM_TOP
					&& mode_switch != DMODE_SW_CLEAR_FROM_BOTTOM)
				{
					text_switch = TEXTMODE_SW_CLEAR;
					if (vp3_target_y == 0)
						vp3_target_y = DISPL_HEIGHT3;
					else
						vp3_target_y = 0;
				}
				break;

			case TEXTMODE_SW_CLEAR:
				SetAPen(&rast_port3, 0);
				RectFill(&rast_port3, 
	            0, vp3_target_y, WIDTH3 - 1, vp3_target_y + DISPL_HEIGHT3 - 1);

				text_switch = TEXTMODE_SW_PRECALC;
				break;

			case TEXTMODE_SW_PRECALC:
				// text_width = font_get_string_width((const char *)&tiny_font_glyph, (const short *)&tiny_font_x_pos, (UBYTE *)demo_string[demo_string_index]);
				// if (text_width >= 0)
				// {
				// 	text_duration = (UWORD)text_width >> 2; // + 5;
				// 	text_switch = TEXTMODE_SW_DRAW;
				// }
				text_width = text_width_cache[demo_string_index];
				if ((text_width_cache[demo_string_index + 1] >> 2) < text_width)
					text_duration = text_width - (text_width_cache[demo_string_index + 1] >> 2);
				else
					text_duration = text_width;
				text_duration >>= 1;
				text_switch = TEXTMODE_SW_DRAW;
				break;

			case TEXTMODE_SW_DRAW:
				if (mode_switch != DMODE_SW_CLEAR_FROM_TOP && mode_switch != DMODE_SW_CLEAR_FROM_BOTTOM)
					text_switch = TEXTMODE_SW_DRAW_LOOP;
				break;

			case TEXTMODE_SW_DRAW_LOOP:
				if (text_draw_switch)
				{
					if (font_blit_string(bitmap_font, bitmap_font, &bit_map3, (const char *)&tiny_font_glyph, (const short *)&tiny_font_x_pos, (WIDTH3 - (UWORD)text_width) >> 1, vp3_target_y + 1, (UBYTE *)demo_string[demo_string_index]) == 0)
					{
						demo_string_index++;
						if (demo_string_index > DEMO_STRINGS_MAX_INDEX)
							demo_string_index = 0;

						counter_before_next_text = 0;
						text_switch = TEXTMODE_SW_SCROLL;
					}
				}
				text_draw_switch = !text_draw_switch;
				break;

			case TEXTMODE_SW_SCROLL:
				if (scrolltext_switch)
				{
					if (scrollTextViewport(vp3_target_y) == 0)
						text_switch = TEXTMODE_SW_WAIT;
				}
				scrolltext_switch = !scrolltext_switch;
				break;
		}

		if (faster_machine)
			WaitTOF();
	}

	if (!faster_machine)
		ON_VBLANK;

	/*	Wait for mouse up
		to prevent the mouse up event
		to mess with the OS gui
	*/
	while(!((*(UBYTE *)0xBFE001) & 0x40))
		WaitTOF();

	ON_SPRITE;

	Enable();
	Permit();

	close_demo("My friend the end!");
}
