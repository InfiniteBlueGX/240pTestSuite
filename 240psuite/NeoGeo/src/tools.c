/* 
 * 240p Test Suite for the Neo Geo
 * by Artemio Urbina and Dustin Dembrosky (Dasutin)
 * Copyright (C)2011-2022 Artemio Urbina
 *
 * This file is part of the 240p Test Suite
 *
 * The 240p Test Suite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The 240p Test Suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 240p Test Suite; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//#include <stddef.h>
#include <stdarg.h>
#include <DATlib.h>
#include <input.h>
#include "types.h"
#include "ng.h"
#include "string_ng.h"
#include "externs.h"
#include "tools.h"
#include "help.h"

u16 randbase;

/*
NTSC AES

    24.167829MHz main clock / 4 = 6.041957MHz pixel clock
    6.041957MHz / 384 pixels per line = 15.734kHz horizontal rate
    15.734kHz / 264 lines = 59.599 frames/second
	16.7788 ms per frame

PAL AES

    24.167829MHz main clock / 4 = 6.041957MHz pixel clock
    6.041957MHz / 384 pixels per line = 15.734kHz horizontal rate
    15.734kHz / 312 lines = 50.429 frames/second
	19.829859 ms per frame

MVS

    24.000000MHz main clock / 4 = 6.000000MHz pixel clock
    6MHz / 384 pixels per line = 15.625kHz horizontal rate
    15.625kHz / 264 lines = 59.1856 frames/second
	16.8960 ms per frame

	Source: https://wiki.neogeodev.org/index.php?title=Framerate
*/

// We can't detect between 2 and 4 slot systems
void check_systype()
{
	BYTE reg = 0;
	WORD regw = 0;

	isMVS = is4S = is6S = isMulti = hwChange = 0, vmode_snk = isPAL = 0, isPALinMVS = 0;
	enable_shadow = 0;
	usePAL256 = 1;
	allowIRE107 = 1;

	if (volMEMBYTE(BIOS_MVS_FLAG) == SYSTEM_MVS)
	{
		isMVS = 1;
		reg = volMEMBYTE(REG_SYSTYPE);
		if (reg & MVS_MULTI)
		{
			isMulti = 1;
			reg = volMEMBYTE(REG_STATUS_A);
			if (reg & MVS_4_OR_6)
				is6S = 1;
			else
				is4S = 1;
		}
	}

	// Detect if MVS/AES IDs are in conflict
	if (MVS_AS_AES || AES_AS_MVS)
		hwChange = 1;

	// Check is 304 mode is enabled, and follow BIOS resolution and rules
	// Also available under options, default is 304 mode to comply
	if (isMVS)
	{
		reg = getSoftDipvalue(SOFT_DIP_4);
		if (reg)
			vmode_snk = 0;
		else
			vmode_snk = 1;
	} else {
		// read from memory card (TODO)
		vmode_snk = 1;
	}
	
	// NTSC at zero, but PAL apparently only works with the newer LPSC2 in AES (I assume all PAL systems have it)
	// we need to test what happens on an AES with LPSC-A0. I checked LSPC2-A2 on a PAL modded system and used
	// that to develop all current tests.
	// LSPC-A0 by NEC is the VDC part of the first generation chipset, see LSPC2-A2 for more details. (AES & MVS)
	// LSPC2-A2 by Fujitsu is the second generation Line SPrite Controller, it is only found in cartridge systems. (AES & MVS)
	regw = volMEMWORD(REG_LSPCMODE);
	if (regw & LPSC2_NTSC_PAL)
	{
		if(isMVS)
			isPALinMVS = 1;
		else
			isPAL = 1;
	}

#ifdef __cd__
	isCDFront = 0;
	isCDZ = 0;
	regw = volMEMWORD(REG_CDCONFIG);
	if (regw & CD_FRONT)
		isCDFront = 1;
	else
	{
		if (!(regw & CD_LID_STATUS))
			isCDZ = 1;
	}
	ngcd_region = ((~regw) & NGCD_REGION) >> 8;
#endif
}

void setRandomSeed(u16 seed)
{
	// xor it with a random value to avoid 0 value
	randbase = seed ^ 0xD94B;
}

u16 random()
{
	unsigned int fc = DAT_frameCounter;
	randbase ^= (randbase >> 1) ^ fc;
	randbase ^= (randbase << 1);

	return randbase;
}

int bcdToDec(int bcd)
{
	return bcd-(bcd/16)*6;
}

int decToBCD(int dec)
{
	return((dec / 10) << 4) | (dec % 10);
}

inline void readController()
{
	p1  = volMEMBYTE(P1_CURRENT);
	p1e = volMEMBYTE(P1_EDGE);
	ps  = volMEMBYTE(PS_CURRENT);
	pse = volMEMBYTE(PS_EDGE);
}

inline void clearController()
{
	p1 = p2 = ps = pse = p1e = p2e = p1b = p2b = 0;
	waitVBlank();
}

/** pictures
typedef struct pictureInfo {
	ushort		stripSize;		//size of each strip (bytes)
	ushort		tileWidth;
	ushort		tileHeight;
	paletteInfo	*palInfo;
	ushort		*maps[4];		//ptrs to maps (std/flipX/flipY/flipXY)
} pictureInfo;
*/
/*
	Sprites are vertical strips of tiles, with a fixed width of 1 tile (16 pixels), and an adjustable height of up to 32 tiles (512 pixels).
	Each sprite has its own tile and attributes map, defining which tile goes in which place, how is it flipped, and which palette to use.
	Sprites can be stuck together horizontally to make huge moveable objects. 
*/
inline int getPicSprites(pictureInfo *picinfo)
{
	return(picinfo->tileWidth);
}

// placed here temporarily
inline void suiteClearFixLayer()
{
	if (vmode_snk)
		BIOS_FIX_CLEAR;		// Call the SNK BIOS clear that puts the black Mask and reduces it to 304 visible pixels
	else
		clearFixLayer();
}

inline void gfxClear()
{
	disableIRQ();
	clearSprites(1, MAX_SPRITES);
	suiteClearFixLayer();
	backgroundColor(fill_color_bg ? IRE_20 : _BLACK);
	enableIRQ();
}

#define OPTIONS_HORI	1
#define OPTIONS_IRE100	2
#define OPTIONS_DARK	3
#define OPTIONS_VERT	4
#define OPTIONS_BGFILL	5
#define OPTIONS_END		6

void menu_options()
{
	int done = 0, curse = 1, cursemax = OPTIONS_END, redraw = 1, text = 0, index = 0;
	char blank[] = "                                ";

	while (!done)
	{
		int toggle = 0, y = 11;

		if (redraw)
		{
			gfxClear();
			index = draw_background();
			redraw = 0;
		}
		
		SCClose();
		waitVBlank();

		readController();

		if (PRESSED_UP)		curse=curse>1?curse-1:cursemax;
		if (PRESSED_DOWN)	curse=curse<cursemax?curse+1:1;

#ifndef __cd__
		fixPrintf(15, 6, fontColorGreen, 3, "%s Options", isMVS ? "MVS" : "AES");
#else
		fixPrintf(14, 6, fontColorGreen, 3, "NGCD Options");
#endif
		fixPrintf(5, y++, curse == OPTIONS_HORI ? fontColorRed : fontColorWhite, 3, "Horizontal Width:    %s", vmode_snk ? "BIOS 304" : "FULL 320");
		fixPrintf(5, y++, curse == OPTIONS_IRE100 ? fontColorRed : fontColorWhite, 3, "IRE limit:           %s", allowIRE107 ? "107 IRE" : "100 IRE");
		fixPrintf(5, y++, curse == OPTIONS_DARK ? fontColorRed : fontColorWhite, 3, "Video Output:        %s", enable_shadow ? "Darken" : "Normal");
		fixPrintf(5, y++, curse == OPTIONS_VERT ? (isPAL ? fontColorRed : fontColorGrayDark) : (isPAL ? fontColorWhite : fontColorGrayLight), 3, "PAL vertical res:    %03dp", usePAL256 ? 256 : 224);
		fixPrintf(5, y++, curse == OPTIONS_BGFILL ? (isPAL ? fontColorRed : fontColorGrayDark) : (isPAL ? fontColorWhite : fontColorGrayLight), 3, "PAL background fill: %s", fill_color_bg ? "Yes" : "No");
		y++;
		fixPrintf(5, y++, curse == OPTIONS_END ? fontColorRed : fontColorWhite, 3, "Back to Main Menu");

		menu_footer();

		// Extra description
		if (curse == OPTIONS_HORI)
		{
			if(text != OPTIONS_HORI)
			{
				fixPrintf(4, 20, fontColorGreen, 3, "The 304 mode uses the BIOS mask");
				fixPrintf(4, 21, fontColorGreen, 3, "to hide 8 pixels on each side, ");
				fixPrintf(4, 22, fontColorGreen, 3, "as required by SNK in games.");
				if (!vmode_snk)
					fixPrintf(4, 23, fontColorRed, 3, "Some systems trim pixel col. 320");
				text = OPTIONS_HORI;
			}
		} else {
			if (text == OPTIONS_HORI)
			{
				int i = 0;

				for (i = 20; i < 24; i++)
					fixPrintf(4, i, fontColorGreen, 3, blank);
				text = 0;
			}
		}

		if (curse == OPTIONS_DARK)
		{
			if(text != OPTIONS_DARK)
			{
				fixPrintf(4, 20, fontColorGreen, 3, "Hardware option, all patterns");
				fixPrintf(4, 21, fontColorGreen, 3, "will be darker.");
				text = OPTIONS_DARK;
			}
		}
		else {
			if (text == OPTIONS_DARK)
			{
				int i = 0;

				for (i = 20; i < 24; i++)
					fixPrintf(4, i, fontColorGreen, 3, blank);
				text = 0;
			}
		}

		if (curse == OPTIONS_IRE100)
		{
			if(text != OPTIONS_IRE100)
			{
				fixPrintf(4, 20, fontColorGreen, 3, "Neo Geo white level goes up to");
				fixPrintf(4, 21, fontColorGreen, 3, "~106.8 IRE. This option limits");
				fixPrintf(4, 22, fontColorGreen, 3, "patterns to 100 IRE.");
				text = 3;
			}
		}
		else {
			if (text == OPTIONS_IRE100)
			{
				int i = 0;

				for (i = 20; i < 24; i++)
					fixPrintf(4, i, fontColorGreen, 3, blank);
				text = 0;
			}
		}

		if (curse != OPTIONS_END && (PRESSED_LEFT || PRESSED_RIGHT))
			toggle = 1;

		if (BTTN_MAIN || toggle)
		{
			switch (curse)
			{
				case OPTIONS_HORI:
					vmode_snk = !vmode_snk;
					suiteClearFixLayer();
					redraw = 1;
				break;

				case OPTIONS_DARK:
					enable_shadow = !enable_shadow;
					if(enable_shadow)
						volMEMBYTE(REG_SHADOW) = 1;
					else
						volMEMBYTE(REG_NOSHADOW) = 1;
				break;

				case OPTIONS_IRE100:
					allowIRE107 = !allowIRE107;
				break;

				case OPTIONS_VERT:
					if(isPAL)
						usePAL256 = !usePAL256;
				break;

				case OPTIONS_BGFILL:
					if(isPAL)
					{
						fill_color_bg = !fill_color_bg;
						redraw = 1;
					}
				break;

				case OPTIONS_END:
					done = 1;
				break;
			}
		}

		if (BTTN_EXIT)
			done = 1;

		if (checkHelp(HELP_GENERAL))
			redraw = 1;
	}

	if(enable_shadow)
		draw_warning("Dark Mode is enabled.\nColor and brightness patterns\nwill be affected.", index, 20, 1);
	return;
}

int draw_background()
{
	int index = 1, palindex = 16;
	picture background;

	pictureInit(&background, &back, index, palindex, 0, 0,FLIP_NONE);
	palJobPut(palindex,back.palInfo->count,back.palInfo->data);

	index += getPicSprites(background.info);
	return index;
}

int draw_background_w_gil(blinker *blinkdata)
{
	int index = 1, palindex = 16;
	picture background;
	picture foreground;

	pictureInit(&background, &back, index, palindex, 0, 0,FLIP_NONE);
	palJobPut(palindex, back.palInfo->count,back.palInfo->data);
	index += getPicSprites(background.info);
	palindex += back.palInfo->count;

	pictureInit(&foreground, &gillian, index, palindex, 216, 70,FLIP_NONE);
	palJobPut(palindex, gillian.palInfo->count, gillian.palInfo->data);
	index += getPicSprites(foreground.info);
	palindex += gillian.palInfo->count;

	if(blinkdata)
		load_blinkdata(blinkdata, &index, &palindex, 216, 70);

	return index;
}

int clear_gillian(int index, blinker *blinkdata)
{
	if(blinkdata)
	{
		clearSprites(index - gblink1.tileWidth, index);
		index -= gblink1.tileWidth;
		clearSprites(index - gblink2.tileWidth, index);
		index -= gblink2.tileWidth;
		memset(blinkdata, 0, sizeof(blinker));
	}
	clearSprites(index - gillian.tileWidth, index);
	index -= gillian.tileWidth;
	return index;
}

void load_blinkdata(blinker* blinkdata, int *index, int *palindex, int x, int y)
{
	blinkdata->blink_counter = 0;
	blinkdata->is_blinking = 0;

	pictureInit(&blinkdata->blink1, &gblink1, *index, *palindex, x+16, y+32, FLIP_NONE);
	palJobPut(*palindex, gblink1.palInfo->count, gblink1.palInfo->data);
	*index += getPicSprites(blinkdata->blink1.info);
	*palindex += gblink1.palInfo->count;

	pictureInit(&blinkdata->blink2, &gblink2, *index, *palindex, x+16, y+32, FLIP_NONE);
	palJobPut(*palindex, gblink2.palInfo->count, gblink2.palInfo->data);
	*index += getPicSprites(blinkdata->blink2.info);
	*palindex += gblink2.palInfo->count;

	pictureHide(&blinkdata->blink1);
	pictureHide(&blinkdata->blink2);
}

int SD_blink_cycle(blinker *blinkdata)
{	
	if(!blinkdata)
		return 0;

	blinkdata->blink_counter++;	
	if(blinkdata->blink_counter > 230)
	{
		if(!blinkdata->is_blinking)
		{
			if(rand() % 100 < 98)	// 2% chance every frame after 240
			{
				blinkdata->is_blinking = 1;
				blinkdata->blink_counter = 230;
				pictureShow(&blinkdata->blink1);
				return 1;
			}
		}
		else
		{
			if(blinkdata->blink_counter == 232)
			{
				pictureHide(&blinkdata->blink1);
				pictureShow(&blinkdata->blink2);
				return 1;
			}
				
			if(blinkdata->blink_counter == 234)
			{
				pictureHide(&blinkdata->blink2);
				pictureShow(&blinkdata->blink1);
				return 1;
			}
	
			if(blinkdata->blink_counter == 236)
			{
				pictureHide(&blinkdata->blink1);
				blinkdata->blink_counter = 0;
				blinkdata->is_blinking = 0;
				return 1;
			}
		}
	}
	return 0;
}

void menu_footer()
{
	fixPrintf(23, 26, fontColorWhite, 3, "%s %03dx%03dp", isPAL ? "PAL " : "NTSC", vmode_snk ? 304 : 320, isPAL && usePAL256 ? 256 : 224);
#ifndef __cd__
	if (isMVS)
	{
		fixPrint(23, 28, fontColorWhite, 3, "MVS");
		if (is4S || is6S)
			fixPrint(26, 28, fontColorWhite, 3, is4S ? "2/4S" : "6S");
		else
			fixPrint(26, 28, fontColorWhite, 3, "1S");
		if (hwChange)
			fixPrint(19, 28, fontColorWhite, 3, "AES>");
	} else {

		fixPrint(27, 28, fontColorWhite, 3, "AES");
		if (hwChange)
			fixPrint(23, 28, fontColorWhite, 3, "MVS>");
	}
#else
	fixPrintf(isCDFront ? 21 : 22, 28, fontColorWhite, 3, "NGCD %s", isCDFront ? "Front" : isCDZ ? "CDZ" : "Top", isCDFront, isCDZ);
#endif

	if ((volMEMBYTE(BIOS_COUNTRY_CODE) == SYSTEM_JAPAN))
	{
		fixPrint(32, 28, fontColorWhite, 3, "Japan");
	}
	else if ((volMEMBYTE(BIOS_COUNTRY_CODE) == SYSTEM_USA))
	{
		fixPrint(34, 28, fontColorWhite, 3, "USA");
	}
	else if ((volMEMBYTE(BIOS_COUNTRY_CODE) == SYSTEM_EUROPE))
	{
		fixPrint(31, 28, fontColorWhite, 3, "Europe");
	}

	if (getSoftDipvalue(SOFT_DIP_2))
	{
		int credits;
		
		credits = getCreditCount();
		fixPrintf(4, 26, fontColorWhite, 3, "CREDIT%c %02d", credits <= 1 ? ' ' : 'S', credits);  // credit counter
	}

	fixPrintf(32, 4, fontColorRed, 3, enable_shadow ? "Dark" : "    ");
	fixPrintf(30, 5, fontColorGreen, 3, !allowIRE107 ? "100 IRE" : "       ");
}

void draw_warning(char* msg, int index, int palindex, int clearback)
{
#ifndef __cd__
	if (!isMVS || getSoftDipvalue(SOFT_DIP_5))
#endif
		draw_message("WARNING", msg, index, palindex, clearback);
}

void draw_message(char *title, char *msg, int index, int palindex, int clearback)
{
	int len = 0, x = 0, done = 0, sublen = 0, rows = 0, i = 0;
	char buffer[40];
	picture mbox;

	if(clearback)
		gfxClear();
	else
		suiteClearFixLayer();

	pictureInit(&mbox, &messagebox, index, palindex, 0, 0, FLIP_NONE);
	palJobPut(palindex,messagebox.palInfo->count,messagebox.palInfo->data);

	len = strlen(title);
	x = (40 - len)/2;
	fixPrint(x, 11, fontColorGreen, 3, title);
	fixPrint(8, 23, fontColorGreen, 3, "Press B or START to close");

	len = strlen(msg);
	for(i = 0; i < len+1; i++)
	{
		if(msg[i] == '\n' || msg[i] == '\0')
		{
			buffer[sublen] = '\0';
			x = (40 - sublen)/2;
			fixPrint(x, 14+rows, fontColorWhite, 3, buffer);

			rows ++;
			sublen = 0;
		}
		else
		{
			buffer[sublen] = msg[i];
			sublen ++;
		}
	}

	if(!rows)
	{
		x = (40 - len)/2;
		fixPrint(x, 14, fontColorWhite, 3, msg);
	}

	while(!done)
	{
		SCClose();
		waitVBlank();

		readController();

		if (BTTN_EXIT || PRESSED_START)
			done = 1;
	}
}

inline int getHorScroll()
{
	int x = NTSC_304;

	if(isPAL && usePAL256) {
		if (vmode_snk)
			x = PAL_304;
		else
			x = PAL_320;
	}
	else {
		if (vmode_snk)
			x = NTSC_304;
		else
			x = NTSC_320;
	}
	return x;
}

inline int getHorScrollAspect()
{
	int x = NTSC_304;

	if(isPAL) {
		if (vmode_snk)
			x = PAL_304;
		else
			x = PAL_320;
	}
	else {
		if (vmode_snk)
			x = NTSC_304;
		else
			x = NTSC_320;
	}
	return x;
}

void getScreenLimits(int* x, int* y)
{
	if(vmode_snk)
		*x = 304;
	else
		*x = 320;
	if(isPAL && usePAL256)
		*y = 256;
	else
		*y = 224;
}

/*
A color word is composed in the following manner:
Bit 	15 	14 	13 	12 	11 	10 	9 	8 	7 	6 	5 	4 	3 	2 	1 	0
Def 	Dk  R0	G0	B0	R4	R3	R2	R1	G4	G3	G2	G1	B4	B3	B2	B1
*/

WORD PackColor(short r, short g, short b, BYTE dark)
{
	WORD color = 0;
	BYTE r_lsb, r_msb, g_lsb, g_msb, b_lsb, b_msb;

	r_lsb = r & (short)0x01;
	r_msb = (r & (short)0x1E) >> 1;

	g_lsb = g & (short)0x01;
	g_msb = (g & (short)0x1E) >> 1;

	b_lsb = b & (short)0x01;
	b_msb = (b & (short)0x1E) >> 1;

	color = (dark & 0x01) << 15 | (r_lsb << 14) | (g_lsb << 13) | (b_lsb << 12) | r_msb << 8 | g_msb << 4 | b_msb;
	return color;
}

void UnPackColor(WORD color, short *r, short *g, short *b, short *dark)
{
	BYTE r_lsb, r_msb, g_lsb, g_msb, b_lsb, b_msb;
	
	*dark = (color & 0x8000) >> 15;

	r_lsb = (color & 0x4000) >> 14;
	r_msb = (color & 0x0F00) >> 8;
	*r = (r_msb << 1) | r_lsb;

	g_lsb = (color & 0x2000) >> 13;
	g_msb = (color & 0x00F0) >> 4;
	*g = (g_msb << 1) | g_lsb;

	b_lsb = (color & 0x1000) >> 12;
	b_msb = (color & 0x000F);
	*b = (b_msb << 1) | b_lsb;
}

void darken_palette(short *pal, short factor)
{
	int p = 0;

	for(p = 0; p < 16; p++)
	{
		short r, g, b, d;

		UnPackColor(pal[p], &r, &g, &b, &d);
		r /= factor;
		g /= factor;
		b /= factor;
		d = 1;
		pal[p] = PackColor(r, g, b, d);
	}
}

void displayRegByte(u16 x, u16 y, char *dispname, u32 regAddr)
{
	BYTE regb = 0;
	char buffer[10];

	regb = volMEMBYTE(regAddr);
	intToHex(regb, buffer, 2);
	fixPrintf(x, y, fontColorGreen, 3, "%s:", dispname);
	fixPrintf(x+15, y, fontColorWhite, 3, "0x%s", buffer);
	byteToBin(regb, buffer);
	fixPrintf(x+22, y, fontColorWhite, 3, "%s", buffer);
}

void displayRegWord(u16 x, u16 y, char *dispname, u32 regAddr)
{
	WORD regw = 0;
	char buffer[10];

	regw = volMEMWORD(regAddr);
	intToHex(regw, buffer, 4);
	fixPrintf(x, y, fontColorGreen, 3, "%s:", dispname);
	fixPrintf(x+15, y, fontColorWhite, 3, "0x%s", buffer);
	byteToBin((regw & 0xFF00) >> 8, buffer);
	fixPrintf(x+22, y, fontColorWhite, 3, "%s", buffer);
	byteToBin(regw & 0x00FF, buffer);
	fixPrintf(x+22, y+1, fontColorWhite, 3, "%s", buffer);
}

void displayValue(u16 x, u16 y, char *dispname, int value)
{
	fixPrintf(x, y, fontColorGreen, 3, "%s:", dispname);
	fixPrintf(x+13, y, fontColorWhite, 3, "%d", value);
}

int getCreditCount()
{
#ifndef __cd__
	if(volMEMBYTE(BIOS_DEV_MODE))
		return(bcdToDec(volMEMBYTE(BIOS_CREDIT_DB)));
	return(bcdToDec(volMEMBYTE(BIOS_NM_CREDIT)));
#else
	return 0;
#endif
}

BYTE getHardDipValue(BYTE harddip)
{
#ifndef __cd__
	if(!isMVS || AES_AS_MVS)
		return 0;
	return(!(volMEMBYTE(REG_DIPSW) & harddip));
#else
	return 0;
#endif
}

void fixPrintC(int y, int color, int font, char *str)
{
	int len = 0, x  =0;

	len = strlen(str);
	x = (40 - len)/2;
	fixPrintf(x, y, color, font, str);
}

int getSoftDipvalue(u32 softdip)
{
#ifndef __cd__
	if(!isMVS)
		return 0;
	return(volMEMBYTE(softdip));	
#else
	return 0;
#endif
}

inline WORD getVideoline()
{
	return(volMEMWORD(REG_LSPCMODE) >> 7);
}

inline void waitVLine(WORD line)
{
	while((volMEMWORD(REG_LSPCMODE) >> 7) != line);	
}

// regular commands AES max 180
// regular commands MAME max 258
// Max Timeout from z80 RAM test in NTSC AES is 66264
// Max Timeout from z80 RAM test in PAL AES is 66508
// Max Timeout from z80 RAM test in MAME is 108733
// we'll use 0x25000

#define Z80_TIMEOUT 0x25000
inline int checkZ80Response(BYTE command)
{
	int timeout = 0;
	BYTE response = 0;

	// wait for the Z80 to enter NMI
	do
	{
		response = volMEMBYTE(REG_SOUND);
		timeout++;
	}while(timeout < Z80_TIMEOUT && response != 0xff);
	// Z80 got command, and is inside the NMI (or timed out)

	if(timeout >= Z80_TIMEOUT)
		return 0;

	do
	{
		response = volMEMBYTE(REG_SOUND);
		timeout++;
	}while(timeout < Z80_TIMEOUT && response == 0xff);
	// Z80 has finished NMI (or timed out)

	if(timeout > max_z80_timout)
		max_z80_timout = timeout;
	if(timeout < min_z80_timout)
		min_z80_timout = timeout;
	if(timeout >= Z80_TIMEOUT)
		return 0;

	return response;
}

/*
 * send command and wait for reponse
 */
int sendZ80command(u8 command)
{
	volMEMBYTE(REG_SOUND) = command;

	return(checkZ80Response(command));
}

/* 
 * Play sound aligned with first line of the visible frame 
 */
void sendZ80commandAtVideoStart(u8 command)
{
	/*
		Get upper 9 bits, Raster line counter
		$0F8~$0FF : Vertical sync (8px)
		$100~$10F : Top border (16px) (active in PAL, blanked in NTSC)
		$110~$1EF : Active display (224px)
		$1F0~$1FF : Bottom border (16px)
	*/

	waitVLine(isPAL ? 0x100 : 0x010C);
	volMEMBYTE(REG_SOUND) = command;

	// we don't wait for Z80 response here
}

/*
 * Play sound aligned with last line of the visible frame 
 */
void sendZ80commandAtVideoEnd(u8 command)
{
	waitVLine(isPAL ? 0x1FD : 0x1ED);
	volMEMBYTE(REG_SOUND) = command;

	// we don't wait for Z80 response here
}

void sendZ80commandAtLine(WORD line, u8 command)
{
	waitVLine(line);
	volMEMBYTE(REG_SOUND) = command;

	// we don't wait for Z80 response here
}

/*
 * Play sound immediately with no delay or response check
 */
void sendZ80commandnoWait(u8 command)
{
	volMEMBYTE(REG_SOUND) = command;
}

#ifdef __cd__

void sendCDDAcommand(BYTE command, BYTE track, BYTE wait)
{
	WORD word_out = 0;
	
	track = decToBCD(track);
	word_out = (command << 8) | track;
	// bit 10 for sync
	if(wait)
		word_out |= (1 << 10);

	disableIRQ();
	// Send command via register d0 to BIOSF_CDDACMD
	asm (
			"move.w		%0,%%d0\n\t"
		".loop_cdda_%=_chk:\n\t"
			"move.b		#5,0x300001\n\t"			// the watchdog
			"tst.b		0x10F6D9\n\t"				// documented wait...
			"beq.s		.loop_cdda_%=_chk\n\t"
			"jsr		0xC0056A\n\t"				// BIOSF_CDDACMD
		: /* No outputs. */
		: "r" (word_out));
	 enableIRQ();
}

void playCDDA(BYTE track, BYTE wait)
{
	sendCDDAcommand(BIOS_CDDA_PLAY_NLP, track, wait);
}

void pauseCDDA(BYTE wait)
{
	sendCDDAcommand(BIOS_CDDA_PAUSE, 0, wait);
}

void unPauseCDDA(BYTE wait)
{
	sendCDDAcommand(BIOS_CDDA_UNPAUSE, 0, wait);
}

#endif