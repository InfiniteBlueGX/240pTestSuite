/* 
 * 240p Test Suite
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

#include "genesis.h"
#include "res.h"
#include "patterns.h"
#include "help.h"
#include "myvdp.h"
#include "main.h"

void DrawPluge()
{
	u16 size, tiles;
	u16 exit = 0, loadvram = 1;
	u16 buttons, oldButtons = 0xffff, pressedButtons;

	while(!exit)
	{
		if(loadvram)
		{
			VDP_Start();
			VDP_setHilightShadow(1);

			tiles = TILE_USERINDEX;
			VDP_setPalette(PAL0, palette_grey);
			size = sizeof(color_tiles) / 32;
			VDP_loadTileData(color_tiles, tiles, size, USE_DMA);

			// 6% & 13% black
			VDP_fillTileMapRect(APLAN, TILE_ATTR_FULL(PAL0, 1, 0, 0, 0) + tiles, 5, 4, 2, 20);
			VDP_fillTileMapRect(APLAN, TILE_ATTR_FULL(PAL0, 0, 0, 0, 0) + tiles, 9, 4, 2, 20);
			VDP_fillTileMapRect(APLAN, TILE_ATTR_FULL(PAL0, 0, 0, 0, 0) + tiles, 29, 4, 2, 20);
			VDP_fillTileMapRect(APLAN, TILE_ATTR_FULL(PAL0, 1, 0, 0, 0) + tiles, 33, 4, 2, 20);

			VDP_fillTileMapRect(APLAN, TILE_ATTR_FULL(PAL0, 1, 0, 0, 6) + tiles, 15, 4, 10, 5);
			VDP_fillTileMapRect(APLAN, TILE_ATTR_FULL(PAL0, 1, 0, 0, 4) + tiles, 15, 9, 10, 5);
			VDP_fillTileMapRect(APLAN, TILE_ATTR_FULL(PAL0, 1, 0, 0, 1) + tiles, 15, 14, 10, 5);
			VDP_fillTileMapRect(APLAN, TILE_ATTR_FULL(PAL0, 0, 0, 0, 2) + tiles, 15, 19, 10, 5);

			VDP_End();
			loadvram = 0;
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_PLUGE))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;
		
		if(pressedButtons & BUTTON_C)
		{
			if(DrawContrast() == 1)
				exit = 1;
			else
				loadvram = 1;
		}

		VDP_waitVSync();
	}
}

void DrawGrayRamp()
{
	u16 size, loadvram = 1;
	u16 exit = 0;
	u16 buttons, oldButtons = 0xffff, pressedButtons;

	while(!exit)
	{
		if(loadvram)
		{
			VDP_Start();
			VDP_clearTileMapRect(APLAN, 0, 0, 320 / 8, 224 / 8);
			VDP_setHilightShadow(1);

			VDP_setPalette(PAL0, palette_grey);
			size = sizeof(color_tiles) / 32;
			VDP_loadTileData(color_tiles, TILE_USERINDEX, size, USE_DMA);
			DrawColorTilesAt(APLAN, PAL0, 0, 0, TILE_USERINDEX, 3, 14);
			DrawColorTilesAtInv(APLAN, PAL0, 0, 14, TILE_USERINDEX, 3, 14);

			//Center it, we are missing a few tiles, so scroll 16 pixels to center the pattern
			VDP_setHorizontalScroll(PLAN_A, 16);
			VDP_End();
			loadvram = 0;
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_GRAY))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;

		VDP_waitVSync();
	}
}

void DrawWhiteScreen()
{
	u16 size, loadvram = 1;
	u16 exit = 0;
	u16 buttons, oldButtons = 0xffff, pressedButtons, redraw = 0;
	int color = 0, sel = 0;
	u16 custom_pal[16];
	u8 r, g, b, custom = 0;
	char str[20], num[4];

	memcpy(custom_pal, palette_grey, sizeof(palette_grey));

	b = custom_pal[0xf] & 0x0f00 >> 8;
	g = custom_pal[0xf] & 0x00f0 >> 4;
	r = custom_pal[0xf] & 0x000f;

	while(!exit)
	{
		if(loadvram)
		{
			size = sizeof(solid_tiles) / 32;

			VDP_Start();
			VDP_setPalette(PAL3, palette_black);
			VDP_loadTileData(solid_tiles, TILE_USERINDEX, size, USE_DMA);
			VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + TILE_USERINDEX, 0, 0, 320 / 8, getVerticalRes() / 8);
			VDP_End();
			loadvram = 0;
		}

		if(redraw)
		{
			custom_pal[0xf] = b << 8 | g << 4 | r;

			VDP_Start();
			switch (color)
			{
			case 0:
				VDP_setPalette(PAL0, custom_pal);
				break;
			case 1:
				VDP_setPalette(PAL0, palette_black);
				break;
			case 2:
				VDP_setPalette(PAL0, palette_red);
				break;
			case 3:
				VDP_setPalette(PAL0, palette_green);
				break;
			case 4:
				VDP_setPalette(PAL0, palette_blue);
				break;
			}

			if(custom && color == 0)
			{
				VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL3, 0, 0, 0) + TILE_USERINDEX, 22, 1, 14, 1);

				strcpy(str, "R:");
				intToHex(r, num, 2);
				strcat(str, num);

				VDP_drawTextBG(APLAN, str, TILE_ATTR(sel == 0 ? PAL1 : PAL2, 0, 0, 0), 22, 1);

				strcpy(str, "G:");
				intToHex(g, num, 2);
				strcat(str, num);

				VDP_drawTextBG(APLAN, str, TILE_ATTR(sel == 1 ? PAL1 : PAL2, 0, 0, 0), 27, 1);

				strcpy(str, "B:");
				intToHex(b, num, 2);
				strcat(str, num);

				VDP_drawTextBG(APLAN, str, TILE_ATTR(sel == 2 ? PAL1 : PAL2, 0, 0, 0), 32, 1);
			}
			else
				VDP_fillTileMapRect(APLAN, TILE_ATTR(PAL0, 0, 0, 0) + TILE_USERINDEX, 22, 1, 14, 1);

			VDP_End();
			redraw = 0;
		}
		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_WHITE))
		{
			loadvram = 1;
			redraw = 1;
		}

		if(pressedButtons & BUTTON_START)
			exit = 1;

		if(custom)
		{
			if(pressedButtons & BUTTON_LEFT)
			{
				sel--;
				redraw = 1;
			}

			if(pressedButtons & BUTTON_RIGHT)
			{
				sel++;
				redraw = 1;
			}

			if(pressedButtons & BUTTON_UP)
			{
				u8 *edit = NULL;

				switch (sel)
				{
				case 0:
					edit = &r;
					break;
				case 1:
					edit = &g;
					break;
				case 2:
					edit = &b;
					break;
				}

				if(edit)
				{
					if(*edit != 0xe)
						(*edit) += 2;

					redraw = 1;
				}
			}

			if(pressedButtons & BUTTON_DOWN)
			{
				u8 *edit = NULL;

				switch (sel)
				{
				case 0:
					edit = &r;
					break;
				case 1:
					edit = &g;
					break;
				case 2:
					edit = &b;
					break;
				}

				if(edit)
				{
					if(*edit != 0)
						(*edit) -= 2;

					redraw = 1;
				}
			}

			if(sel < 0)
				sel = 2;
			if(sel > 2)
				sel = 0;
		}

		if(pressedButtons & BUTTON_A)
		{
			color++;
			redraw = 1;
		}

		if(pressedButtons & BUTTON_B)
		{
			color--;
			redraw = 1;
		}

		if(pressedButtons & BUTTON_C)
		{
			custom = !custom;
			redraw = 1;
		}

		if(color > 4)
			color = 0;

		if(color < 0)
			color = 4;

		VDP_waitVSync();
	}
}

void DrawSMPTEorEBU(int EBU)
{
	u16 size, Is75 = 0, text = 0;
	u16 exit = 0, loadvram = 1;
	u16 buttons, oldButtons = 0xffff, pressedButtons;

	while(!exit)
	{
		if(loadvram)
		{
			VDP_Start();
			if(!Is75)
				VDP_setPalette(PAL2, SMPTECB100_pal);
			else
				VDP_setPalette(PAL2, SMPTECB75_pal);

			if(EBU)
			{
				size = sizeof(EBUColorBars75_tiles) / 32;
				VDP_loadTileData(EBUColorBars75_tiles, TILE_USERINDEX, size, USE_DMA);
				VDP_setMyTileMapRect(BPLAN, EBUColorBars75_map, TILE_ATTR(PAL2, 0, 0, 0) + TILE_USERINDEX, 0, 0, 320 / 8, 224 / 8);
			}
			else
			{
				size = sizeof(SMPTECB75_tiles) / 32;
				VDP_loadTileData(SMPTECB75_tiles, TILE_USERINDEX, size, USE_DMA);
				VDP_setMyTileMapRect(BPLAN, SMPTECB75_map, TILE_ATTR(PAL2, 0, 0, 0) + TILE_USERINDEX, 0, 0, 320 / 8, 224 / 8);
			}
			VDP_End();
			loadvram = 0;
		}

		if(text)
		{
			text--;
			if(!text)
			{
				VDP_Start();
				VDP_drawText("    ", 32, 1);
				VDP_End();
			}
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_SMPTE))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;

		if(pressedButtons & BUTTON_A)
		{
			Is75 = !Is75;

			VDP_Start();
			if(!Is75)
			{
				VDP_setPalette(PAL2, SMPTECB100_pal);
				VDP_drawText("100%", 32, 1);
			}
			else
			{
				VDP_setPalette(PAL2, SMPTECB75_pal);
				VDP_drawText(" 75%", 32, 1);
			}
			VDP_End();
			text = 30;
		}

		VDP_waitVSync();
	}
}

void Draw601ColorBars()
{
	u16 size;
	u16 exit = 0, loadvram = 1;
	u16 buttons, oldButtons = 0xffff, pressedButtons;

	while(!exit)
	{
		if(loadvram)
		{
			size = sizeof(cb601_tiles) / 32;
			VDP_Start();
			VDP_setPalette(PAL0, cb601_pal);
			VDP_loadTileData(cb601_tiles, TILE_USERINDEX, size, USE_DMA);
			VDP_setMyTileMapRect(BPLAN, cb601_map, TILE_USERINDEX, 0, 0, 320 / 8, 224 / 8);
			VDP_End();
			loadvram = 0;
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_601CB))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;

		VDP_waitVSync();
	}
}

void DrawSharpness()
{
	u16 size;
	u16 exit = 0, loadvram = 1, type = FLOAT_OPTION, ntype = 0;
	u16 buttons, oldButtons = 0xffff, pressedButtons;
	
	while(!exit)
	{
		if(loadvram)
		{
			VDP_Start();
			if(type == FLOAT_OPTION)
			{
				VDP_setScreenWidth320();
				
				size = sizeof(sharpness_tiles) / 32;
				VDP_setPalette(PAL0, sharpness_pal);
				VDP_loadTileData(sharpness_tiles, TILE_USERINDEX, size, USE_DMA);
				VDP_setMyTileMapRect(BPLAN, sharpness_map, TILE_USERINDEX, 0, 0, 320 / 8, 224 / 8);
			}
			else
			{	
				if(type == RES_256)
					VDP_setScreenWidth256();
				else
					VDP_setScreenWidth320();
				
				size = sizeof(bricks_tiles) / 32;	
				VDP_setPalette(PAL0, bricks_pal);
				VDP_loadTileData(bricks_tiles, TILE_USERINDEX, size, USE_DMA);
				VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + TILE_USERINDEX, 0, 0, 320 / 8, getVerticalRes() / 8);
			}
			VDP_End();
			loadvram = 0;
		}
		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_SHARPNESS))
			loadvram = 1;

		if(pressedButtons & BUTTON_C)
		{
			
			ntype = DrawFloatMenuResExtra(type, "Sharpness");
			if(ntype != FLOAT_CANCEL)
				type = ntype;
			
			resetController(&oldButtons);
			loadvram = 1;
		}
		
		if(pressedButtons & BUTTON_START)
			exit = 1;

		VDP_waitVSync();
	}
}

void DrawGrid()
{
	u16 size, drawBorder = 0;
	u16 exit = 0, loadvram = 1, type = 0, ntype = 0;
	u16 buttons, oldButtons = 0xffff, pressedButtons;
	u16 first_pal[16], oldColor = 0, redraw = 1;

	type = DrawFloatMenuRes(RES_320);
	if(type == FLOAT_CANCEL)
		return;
		
	while(!exit)
	{
		if(VDPChanged)
		{
			loadvram = 1;
			VDPChanged = 0;
		}

		if(loadvram)
		{
			VDP_Start();
			
			VDP_clearTileMapRect(APLAN, 0, 0, 320 / 8, 240 / 8);
			if(type == RES_256)
				VDP_setScreenWidth256();
			else
				VDP_setScreenWidth320();

			VDP_setPalette(PAL0, grid_pal);
			size = sizeof(grid_tiles) / 32;
			VDP_loadTileData(grid_tiles, TILE_USERINDEX, size, USE_DMA);

			if(type == RES_256)
			{
				if(isVertical240())
					VDP_setMyTileMapRect(APLAN, grid_PAL_256_map, TILE_USERINDEX, 0, 0, 256 / 8, 240 / 8);
				else
					VDP_setMyTileMapRect(APLAN, grid_256_map, TILE_USERINDEX, 0, 0, 256 / 8, 224 / 8);
			}
			else
			{
				if(isVertical240())
					VDP_setMyTileMapRect(APLAN, grid_PAL_map, TILE_USERINDEX, 0, 0, 320 / 8, 240 / 8);
				else
					VDP_setMyTileMapRect(APLAN, grid_map, TILE_USERINDEX, 0, 0, 320 / 8, 224 / 8);
			}
			VDP_End();

			oldColor = first_pal[0];
			loadvram = 0;
			redraw = 1;
		}
		if(redraw)
		{
			VDP_Start();
			if(drawBorder)
			{
				VDP_getPalette(PAL0, first_pal);
				oldColor = first_pal[0];
				first_pal[0] = 0x0666;
				VDP_setPalette(PAL0, first_pal);
				
			}
			else
			{
				VDP_getPalette(PAL0, first_pal);
				first_pal[0] = oldColor;
				VDP_setPalette(PAL0, first_pal);
			}
			VDP_End();
			redraw = 0;
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_GRID))
			loadvram = 1;

		if(pressedButtons & BUTTON_A)
		{
			drawBorder = !drawBorder;
			redraw = 1;
		}

		if(pressedButtons & BUTTON_START)
			exit = 1;
		
		if(pressedButtons & BUTTON_C)
		{
			ntype = DrawFloatMenuRes(type);
			if(ntype != FLOAT_CANCEL)
				type = ntype;
			resetController(&oldButtons);
			loadvram = 1;
		}

		VDP_waitVSync();
	}
}

void DrawMonoscope()
{
	u16 ind;
	u16 size;
	u16 exit, loadvram = 1, type = 0, ntype = 0, redraw = 0, drawBorder = 0, oldColor = 0;
	u16 buttons, oldButtons = 0xffff, pressedButtons;
	u16 custom_pal[16];
	int bright = 1;

	exit = 0;
	
	if(!IsPALVDP)
	{
		type = DrawFloatMenuRes(RES_320);
		if(type == FLOAT_CANCEL)
			return;
	}

	memcpy(custom_pal, grid_pal, sizeof(grid_pal));
	while(!exit)
	{
		if(VDPChanged)
		{
			loadvram = 1;
			VDPChanged = 0;
		}

		if(loadvram)
		{
			VDP_Start();
			
			VDP_clearTileMapRect(APLAN, 0, 0, 320 / 8, 240 / 8);
			VDP_setVerticalScroll(PLAN_A, 0);
			
			if(IsPALVDP)
				type = RES_320;
				
			if(type == RES_256)
				VDP_setScreenWidth256();
			else
				VDP_setScreenWidth320();
			
			VDP_setPalette(PAL0, custom_pal);

			ind = TILE_USERINDEX;
			
			if(!IsPALVDP)
			{
				if(type == RES_256)
				{
					size = sizeof(monoscope_tiles) / 32;
					VDP_loadTileData(monoscope_tiles, ind, size, USE_DMA);
					VDP_setMyTileMapRect(APLAN, monoscope_map, TILE_USERINDEX, 0, 0, 256 / 8, 224 / 8);
				}
				else
				{
					size = sizeof(monoscope320_tiles) / 32;
					VDP_loadTileData(monoscope320_tiles, ind, size, USE_DMA);
					VDP_setMyTileMapRect(APLAN, monoscope320_map, TILE_USERINDEX, 0, 0, 320 / 8, 224 / 8);
				}
			}
			else
			{
				//Scroll to center the pattern
				if(!isVertical240())
					VDP_setVerticalScroll(PLAN_A, 8);
				
				size = sizeof(monoscopePAL_tiles) / 32;
				VDP_loadTileData(monoscopePAL_tiles, ind, size, USE_DMA);
				VDP_setMyTileMapRect(APLAN, monoscopePAL_map, TILE_USERINDEX, 0, 0, 320 / 8, 240 / 8);
			}

			VDP_End();

			loadvram = 0;
		}
		
		if(redraw)
		{
			VDP_Start();
			if(drawBorder)
			{
				VDP_getPalette(PAL0, custom_pal);
				oldColor = custom_pal[0];
				custom_pal[0] = 0x0666;
				VDP_setPalette(PAL0, custom_pal);
				
			}
			else
			{
				VDP_getPalette(PAL0, custom_pal);
				custom_pal[0] = oldColor;
				VDP_setPalette(PAL0, custom_pal);
			}
			VDP_End();
			redraw = 0;
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_MONOSCOPE))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;
			
		if(pressedButtons & BUTTON_A)
		{
			bright++;
			if(bright > 7)
				bright = 1;
			custom_pal[2] = btw_pal[bright];
			VDP_setPalette(PAL0, custom_pal);
		}
		
		if(pressedButtons & BUTTON_B)
		{
			drawBorder = !drawBorder;
			redraw = 1;
		}
		
		if(pressedButtons & BUTTON_C)
		{
			ntype = DrawFloatMenuRes(type);
			if(ntype != FLOAT_CANCEL)
				type = ntype;
			resetController(&oldButtons);
			loadvram = 1;
		}

		VDP_waitVSync();
	}
}

void DrawColorBleed()
{
	u16 ind = 0, Drawtype = 0;
	u16 size = 0, loadvram = 1;
	u16 exit = 0, type = 0, ntype = 0, redraw = 1;
	u16 buttons, oldButtons = 0xffff, pressedButtons;

	type = DrawFloatMenuRes(RES_320);
	if(type == FLOAT_CANCEL)
		return;
		
	while(!exit)
	{
		if(loadvram)
		{
			if(type == RES_256)
				VDP_setScreenWidth256();
			else
				VDP_setScreenWidth320();
			
			VDP_Start();
			VDP_setPalette(PAL0, palette_red);
			VDP_setPalette(PAL1, palette_green);
			VDP_setPalette(PAL2, palette_blue);
			VDP_setPalette(PAL3, palette_grey);

			ind = TILE_USERINDEX;
			size = sizeof(vstripes_tiles) / 32;

			if(Drawtype)
				VDP_loadTileData(checkbleed_tile, ind, size, USE_DMA);
			else
				VDP_loadTileData(vstripes_tiles, ind, size, USE_DMA);

			VDP_End();
			loadvram = 0;
			redraw = 1;
		}
		if(redraw)
		{
			u16 width = 0;

			VDP_Start();			
			if(Drawtype)
				VDP_loadTileData(checkbleed_tile, ind, size, USE_DMA);
			else
				VDP_loadTileData(vstripes_tiles, ind, size, USE_DMA);
				
			if(type == RES_256)
				width = 224;
			else
				width = 288;

			VDP_fillTileMapRect(APLAN, TILE_ATTR(PAL0, 0, 0, 0) + ind, 16 / 8, 40 / 8, width / 8, 32 / 8);
			VDP_fillTileMapRect(APLAN, TILE_ATTR(PAL1, 0, 0, 0) + ind, 16 / 8, 80 / 8, width / 8, 32 / 8);
			VDP_fillTileMapRect(APLAN, TILE_ATTR(PAL2, 0, 0, 0) + ind, 16 / 8, 120 / 8, width / 8, 32 / 8);
			VDP_fillTileMapRect(APLAN, TILE_ATTR(PAL3, 0, 0, 0) + ind, 16 / 8, 160 / 8, width / 8, 32 / 8);
			VDP_End();
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_BLEED))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;

		if(pressedButtons & BUTTON_A)
		{
			Drawtype = !Drawtype;
			redraw = 1;
		}
		
		if(pressedButtons & BUTTON_C)
		{
			ntype = DrawFloatMenuRes(type);
			if(ntype != FLOAT_CANCEL)
				type = ntype;
			resetController(&oldButtons);
			loadvram = 1;
		}

		VDP_waitVSync();
	}
}

void DrawColorBars()
{
	u16 ind;
	u16 size;
	u16 exit, loadvram = 1;
	u16 buttons, oldButtons = 0xffff, pressedButtons;

	exit = 0;

	while(!exit)
	{
		if(loadvram)
		{
			VDP_Start();
			
			VDP_setPalette(PAL0, palette_red);
			VDP_setPalette(PAL1, palette_green);
			VDP_setPalette(PAL2, palette_blue);
			VDP_setPalette(PAL3, palette_grey);

			VDP_setHilightShadow(1);
			ind = TILE_USERINDEX;
			size = sizeof(color_tiles) / 32;
			VDP_loadTileData(color_tiles, ind, size, USE_DMA);

			VDP_drawTextBG(APLAN, "  0  1  2  3  4  5  6  7  8  A  C  E", TILE_ATTR(PAL0, 1, 0, 0), 2, 4);
			VDP_drawTextBG(APLAN, "RED", TILE_ATTR(PAL0, 1, 0, 0), 2, 9);
			VDP_drawTextBG(APLAN, "GREEN", TILE_ATTR(PAL1, 1, 0, 0), 2, 14);
			VDP_drawTextBG(APLAN, "BLUE", TILE_ATTR(PAL2, 1, 0, 0), 2, 19);
			VDP_drawTextBG(APLAN, "WHITE", TILE_ATTR(PAL3, 1, 0, 0), 2, 24);
			DrawColorTilesAt(APLAN, PAL0, 2, 5, ind, 3, 4);
			DrawColorTilesAt(APLAN, PAL1, 2, 10, ind, 3, 4);
			DrawColorTilesAt(APLAN, PAL2, 2, 15, ind, 3, 4);
			DrawColorTilesAt(APLAN, PAL3, 2, 20, ind, 3, 4);
			VDP_End();

			loadvram = 0;
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_COLORS))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;

		VDP_waitVSync();
	}
}

void Draw100IRE()
{
	char str[10];
	u16 size, irevals[] = { 13, 25, 41, 53, 66, 82, 94 };
	u16 exit = 0, ire = 6, text = 0, loadvram = 1;
	u16 buttons, oldButtons = 0xffff, pressedButtons;

	size = sizeof(color_tiles) / 32;

	while(!exit)
	{
		if(loadvram)
		{
			VDP_Start();
			VDP_loadTileData(color_tiles, TILE_USERINDEX, size, USE_DMA);
			VDP_fillTileMapRect(APLAN, TILE_ATTR(PAL0, 0, 0, 0) + TILE_USERINDEX + ire, 80 / 8, 56 / 8, 160 / 8, 112 / 8);
			VDP_setPalette(PAL0, palette_grey);
			VDP_End();
			loadvram = 0;
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_IRE))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;

		// 13, 25, 41, 53, 66, 82, 94 IRE
		if(pressedButtons & BUTTON_A)
		{
			if(ire != 0)
				ire--;
			VDP_Start();
			VDP_fillTileMapRect(APLAN, TILE_ATTR(PAL0, 0, 0, 0) + TILE_USERINDEX + ire, 80 / 8, 56 / 8, 160 / 8, 112 / 8);
			intToStr(irevals[ire], str, 2);
			VDP_drawText(str, 32, 26);
			VDP_drawText("IRE", 35, 26);
			VDP_End();
			text = 60;
		}
		if(pressedButtons & BUTTON_B)
		{
			if(ire != 6)
				ire++;
			VDP_Start();
			VDP_fillTileMapRect(APLAN, TILE_ATTR(PAL0, 0, 0, 0) + TILE_USERINDEX + ire, 80 / 8, 56 / 8, 160 / 8, 112 / 8);
			intToStr(irevals[ire], str, 2);
			VDP_drawText(str, 32, 26);
			VDP_drawText("IRE", 35, 26);
			VDP_End();
			text = 60;
		}
		if(text)
		{
			text--;
			if(!text)
			{
				VDP_Start();
				VDP_drawText("        ", 32, 26);
				VDP_End();
			}
		}
		VDP_waitVSync();
	}
}

void DrawColorTilesAt(u16 plan, u16 pal, u16 x, u16 y, u16 tiles, u16 w, u16 h)
{
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 0, 0, 0, 0) + tiles, x + w * 1, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 0) + tiles, x + w * 2, y, w, h);

	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 0, 0, 0, 2) + tiles, x + w * 3, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 1) + tiles, x + w * 4, y, w, h);

	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 0, 0, 0, 4) + tiles, x + w * 5, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 2) + tiles, x + w * 6, y, w, h);

	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 0, 0, 0, 6) + tiles, x + w * 7, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 3) + tiles, x + w * 8, y, w, h);

	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 4) + tiles, x + w * 9, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 5) + tiles, x + w * 10, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 6) + tiles, x + w * 11, y, w, h);
}

void DrawColorTilesAtInv(u16 plan, u16 pal, u16 x, u16 y, u16 tiles, u16 w, u16 h)
{
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 0, 0, 0, 0) + tiles, x + w * 10, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 0) + tiles, x + w * 9, y, w, h);

	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 0, 0, 0, 2) + tiles, x + w * 8, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 1) + tiles, x + w * 7, y, w, h);

	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 0, 0, 0, 4) + tiles, x + w * 6, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 2) + tiles, x + w * 5, y, w, h);

	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 0, 0, 0, 6) + tiles, x + w * 4, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 3) + tiles, x + w * 3, y, w, h);

	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 4) + tiles, x + w * 2, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 5) + tiles, x + w * 1, y, w, h);
	VDP_fillTileMapRect(plan, TILE_ATTR_FULL(pal, 1, 0, 0, 6) + tiles, x + w * 0, y, w, h);
}

u32 *tile_l, *tile_r, *tile_t, *tile_b;
u32 *tile_lb, *tile_lt, *tile_rt, *tile_rb;

void CleanTile(u32 * array)
{
	int i = 0;

	for(i = 0; i < 8; i++)
		array[i] = 0x44444444;
}

void FillTiles(u16 vram, int left, int right, int top, int bottom)
{
	u32 mask = 0;
	int l, r, t, b, i;

	l = left % 8;
	r = right % 8;
	t = top % 8;
	b = bottom % 8;

	CleanTile(tile_l);
	CleanTile(tile_r);
	CleanTile(tile_t);
	CleanTile(tile_b);
	CleanTile(tile_lt);
	CleanTile(tile_lb);
	CleanTile(tile_rt);
	CleanTile(tile_rb);

	// top
	for(i = 0; i < t; i++)
	{
		tile_t[i] = 0x77777777;
		tile_rt[i] = 0x77777777;
		tile_lt[i] = 0x77777777;
	}

	// bottom       
	for(i = 0; i < b; i++)
	{
		tile_b[7 - i] = 0x77777777;
		tile_rb[7 - i] = 0x77777777;
		tile_lb[7 - i] = 0x77777777;
	}

	// left
	mask = 0x44444444;
	for(i = 0; i < l; i++)
		mask |= 0x7 << 4 * (7 - i);

	for(i = 0; i < 8; i++)
	{
		tile_l[i] = mask;
		tile_lt[i] |= mask;
		tile_lb[i] |= mask;
	}

	// right
	mask = 0x44444444;
	for(i = 0; i < r; i++)
		mask |= 0x7 << 4 * i;

	for(i = 0; i < 8; i++)
	{
		tile_r[i] = mask;
		tile_rt[i] |= mask;
		tile_rb[i] |= mask;
	}

	VDP_loadTileData(tile_l, vram + 1, 1, USE_DMA);
	VDP_loadTileData(tile_r, vram + 2, 1, USE_DMA);
	VDP_loadTileData(tile_t, vram + 3, 1, USE_DMA);
	VDP_loadTileData(tile_b, vram + 4, 1, USE_DMA);
	VDP_loadTileData(tile_lt, vram + 5, 1, USE_DMA);
	VDP_loadTileData(tile_lb, vram + 6, 1, USE_DMA);
	VDP_loadTileData(tile_rt, vram + 7, 1, USE_DMA);
	VDP_loadTileData(tile_rb, vram + 8, 1, USE_DMA);
}

void DrawOverscan()
{
	u32 _tile_l[8], _tile_r[8], _tile_t[8], _tile_b[8];
	u32 _tile_lb[8], _tile_lt[8], _tile_rt[8], _tile_rb[8];
	u16 vram = TILE_USERINDEX, type = 0, ntype = 0;
	int left = 0, right = 0, top = 0, bottom = 0, exit = 0;
	u16 buttons, oldButtons = 0xffff, pressedButtons, redraw = 1;
	int sel = 0, maxTileVert = 0, maxTileHor = 0, loadvram = 1;
	const u32 back[8] = { 0x44444444, 0x44444444, 0x44444444, 0x44444444,
		0x44444444, 0x44444444, 0x44444444, 0x44444444
	};
	const u32 white[8] = { 0x77777777, 0x77777777, 0x77777777, 0x77777777,
		0x77777777, 0x77777777, 0x77777777, 0x77777777
	};

	tile_l = _tile_l;
	tile_r = _tile_r;
	tile_t = _tile_t;
	tile_b = _tile_b;
	tile_lb = _tile_lb;
	tile_lt = _tile_lt;
	tile_rt = _tile_rt;
	tile_rb = _tile_rb;

	type = DrawFloatMenuRes(RES_320);
	if(type == FLOAT_CANCEL)
		return;

	while(!exit)
	{
		if(VDPChanged)
		{
			loadvram = 1;
			VDPChanged = 0;
		}
		
		if(loadvram)
		{
			VDP_Start();
			if(type == RES_256)
				VDP_setScreenWidth256();
			else
				VDP_setScreenWidth320();
					
			VDP_loadTileData(back, vram, 1, USE_DMA);
			VDP_loadTileData(white, vram + 9, 1, USE_DMA);

			VDP_setPalette(PAL3, palette_grey);
			VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL3, 0, 0, 0) + vram, 0, 0, (type == RES_256 ? 256 : 320) / 8, getVerticalRes() / 8);
			VDP_End();
			loadvram = 0;
			redraw = 1;
		}

		if(redraw)
		{
			char data[10];
			int l, r, t, b;

			l = left / 8;
			r = right / 8;
			t = top / 8;
			b = bottom / 8;

			maxTileVert = getVerticalRes() / 8;
			maxTileHor = (type == RES_256 ? 256 : 320) / 8;

			VDP_Start();
			// Clean center
			VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram, l + 1, t + 1, maxTileHor - (r + l) - 2, maxTileVert - (t + b) - 2);

			FillTiles(vram, left, right, top, bottom);

			// Left
			VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 1, l, t + 1, 1, (maxTileVert - 2) - b - t);
			
			// Right
			VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 2, (maxTileHor - 1) - r, t + 1, 1, (maxTileVert - 2) - b - t);
			// Top
			VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 3, l + 1, t, (maxTileHor - 2) - r - l, 1);
			// Bottom
			VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 4, l + 1, (maxTileVert - 1) - b, (maxTileHor - 2) - r - l, 1);

			// Corners

			// left top
			VDP_setTileMapXY(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 5, l, t);
			// left bottom
			VDP_setTileMapXY(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 6, l, (maxTileVert - 1) - b);
			// right top
			VDP_setTileMapXY(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 7, (maxTileHor - 1) - r, t);
			// right bottom
			VDP_setTileMapXY(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 8, (maxTileHor - 1) - r, (maxTileVert - 1) - b);

			// Whites
			if(l)
				VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 9, 0, t, l, maxTileVert - b - t);
			// Right
			if(r)
				VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 9, maxTileHor - r, t, r, maxTileVert - b - t);
			// Top
			if(t)
				VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 9, 0, 0, maxTileHor, t);
			// Bottom
			if(b)
				VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 9, 0, maxTileVert - b, maxTileHor, b);

			// text
			intToStr(top, data, 1);
			VDP_drawTextBG(APLAN, "Top:", TILE_ATTR(sel == 0 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 8 : 12, 12);
			VDP_drawTextBG(APLAN, "   pixels", TILE_ATTR(sel == 0 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 16 : 20, 12);
			VDP_drawTextBG(APLAN, data, TILE_ATTR(sel == 0 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 16 : 20, 12);

			intToStr(bottom, data, 1);
			VDP_drawTextBG(APLAN, "Bottom:", TILE_ATTR(sel == 1 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 8 : 12, 13);
			VDP_drawTextBG(APLAN, "   pixels", TILE_ATTR(sel == 1 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 16 : 20, 13);
			VDP_drawTextBG(APLAN, data, TILE_ATTR(sel == 1 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 16 : 20, 13);

			intToStr(left, data, 1);
			VDP_drawTextBG(APLAN, "Left:", TILE_ATTR(sel == 2 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 8 : 12, 14);
			VDP_drawTextBG(APLAN, "   pixels", TILE_ATTR(sel == 2 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 16 : 20, 14);
			VDP_drawTextBG(APLAN, data, TILE_ATTR(sel == 2 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 16 : 20, 14);

			intToStr(right, data, 1);
			VDP_drawTextBG(APLAN, "Right:", TILE_ATTR(sel == 3 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 8 : 12, 15);
			VDP_drawTextBG(APLAN, "   pixels", TILE_ATTR(sel == 3 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 16 : 20, 15);
			VDP_drawTextBG(APLAN, data, TILE_ATTR(sel == 3 ? PAL1 : PAL0, 0, 0, 0), type == RES_256 ? 16 : 20, 15);
			VDP_End();

			redraw = 0;
		}
		
		VDP_waitVSync();
		
		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;


		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_OVERSCAN))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;

		if(pressedButtons & BUTTON_UP)
		{
			sel--;
			redraw = 1;
		}

		if(pressedButtons & BUTTON_DOWN)
		{
			sel++;
			redraw = 1;
		}

		if(sel < 0)
			sel = 3;
		if(sel > 3)
			sel = 0;

		if(pressedButtons & BUTTON_LEFT)
		{
			int *data = NULL;

			switch (sel)
			{
			case 0:
				data = &top;
				break;
			case 1:
				data = &bottom;
				break;
			case 2:
				data = &left;
				break;
			case 3:
				data = &right;
				break;
			}

			if(data)
			{
				(*data)--;
				if(*data < 0)
					*data = 0;
			}
			redraw = 1;
		}

		if(pressedButtons & BUTTON_RIGHT)
		{
			int *data = NULL;

			switch (sel)
			{
			case 0:
				data = &top;
				break;
			case 1:
				data = &bottom;
				break;
			case 2:
				data = &left;
				break;
			case 3:
				data = &right;
				break;
			}

			if(data)
			{
				(*data)++;
				if(*data > 99)
					*data = 99;
			}
			redraw = 1;
		}

		if(pressedButtons & BUTTON_A)
		{
			left = right = bottom = top = 0;
			redraw = 1;
		}
		
		if(pressedButtons & BUTTON_C)
		{
			ntype = DrawFloatMenuRes(type);
			if(ntype != FLOAT_CANCEL)
				type = ntype;
			resetController(&oldButtons);
			loadvram = 1;
		}
	}
}

u8 DrawContrast()
{
	u16 size, tiles;
	u16 exit = 0, loadvram = 1;
	u16 buttons, oldButtons = 0xffff, pressedButtons;
	u16 *selpal[4], selected = 0;

	selpal[0] = FireShark_pal;
	selpal[1] = FireSharkBL_pal;
	selpal[2] = FireSharkBL_pal;
	selpal[3] = FireSharkWH_pal;
	
	while(!exit)
	{
		if(loadvram)
		{
			int i = 0, j = 0;
			
			VDP_Start();
			if(selected == 1)
				VDP_setHilightShadow(1);
			else
				VDP_setHilightShadow(0);
			tiles = TILE_USERINDEX;
			VDP_setPalette(PAL0, selpal[selected]);
			size = sizeof(FireShark_tiles) / 32;
			VDP_loadTileData(FireShark_tiles, tiles, size, USE_DMA);

			for(j = 0; j < 28; j += 4)
			{
				for(i = 0; i < 40; i += 4)
				{
					VDP_fillTileMapInc(APLAN, TILE_ATTR_FULL(PAL0, 0, 0, 0, 0) + tiles   , i    +64*j, 4);
					VDP_fillTileMapInc(APLAN, TILE_ATTR_FULL(PAL0, 0, 0, 0, 0) + tiles+4 , i+64 +64*j, 4);
					VDP_fillTileMapInc(APLAN, TILE_ATTR_FULL(PAL0, 0, 0, 0, 0) + tiles+8 , i+128+64*j, 4);
					VDP_fillTileMapInc(APLAN, TILE_ATTR_FULL(PAL0, 0, 0, 0, 0) + tiles+12, i+192+64*j, 4);
				}
			}

			VDP_End();
			loadvram = 0;
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_CONTRAST))
			loadvram = 1;

		if(pressedButtons & BUTTON_START)
			exit = 1;
			
		if(pressedButtons & BUTTON_A)
		{
			VDP_Start();
			selected ++;
			if(selected > 3)
				selected = 0;
			VDP_setPalette(PAL0, selpal[selected]);
			if(selected == 1)
				VDP_setHilightShadow(1);
			else
				VDP_setHilightShadow(0);
			VDP_End();
		}
		if(pressedButtons & BUTTON_C)
		{
			CleanUp();
			return(0);
		}

		VDP_waitVSync();
	}
	return(1);
}

void DrawConvergence()
{
	int convType = 4, horRes = RES_320;
	int exit = 0, nhorRes = horRes, redraw = 1;
	u16 ind = 0, size = 0, loadvram = 1; 
	u16 buttons, oldButtons = 0xffff, pressedButtons;

	horRes = DrawFloatMenuRes(RES_320);
	if(horRes == FLOAT_CANCEL)
		return;
	
	while(!exit)
	{
		if(loadvram)
		{
			VDP_Start();	
			if(convType < 3)
			{
				VDP_setPalette(PAL0, bw_pal);

				ind = TILE_USERINDEX;
				size = sizeof(convgrid_tiles) / 32;
				VDP_loadTileData(convgrid_tiles, ind, size, USE_DMA);
				ind += size;
				size = sizeof(convdot_tiles) / 32;
				VDP_loadTileData(convdot_tiles, ind, size, USE_DMA);
				ind += size;
				size = sizeof(convcross_tiles) / 32;
				VDP_loadTileData(convcross_tiles, ind, size, USE_DMA);
			}
			else
			{	
				size = sizeof(convcolor_tiles) / 32;
				VDP_setPalette(PAL0, convcolor_pal);
				VDP_loadTileData(convcolor_tiles, TILE_USERINDEX, size, USE_DMA);
				VDP_setMyTileMapRect(APLAN, convcolor_map, TILE_USERINDEX, 0, 0, 320 / 8, 224 / 8);
			}
			VDP_End();
			loadvram = 0;
			redraw = 1;
		}
		
		if(redraw)
		{
			VDP_Start();
			if(horRes == RES_256)
				VDP_setScreenWidth256();
			else
				VDP_setScreenWidth320();
			
			if(convType < 3)
				VDP_fillTileMapRect(APLAN, TILE_ATTR(PAL0, 0, 0, 0) + TILE_USERINDEX+size*convType, 
							0, 0, (horRes == RES_320 ? 320 : 256)/ 8, getVerticalRes() / 8);
			else
			{
				if(convType == 4)
					VDP_setPalette(PAL0, convcolor_lines_pal);
				else
					VDP_setPalette(PAL0, convcolor_pal);
			}
			VDP_End();
			if(isVertical240() && convType >= 3)
				VDP_setVerticalScroll(PLAN_A, -8);
			else
				VDP_setVerticalScroll(PLAN_A, 0);
			redraw = 0;
		}

		buttons = JOY_readJoypad(JOY_ALL);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_CONVERGENCE))
			loadvram = 1;
			
		if(pressedButtons & BUTTON_A)
		{
			convType--;
			if(convType < 0)
				convType = 4;
			loadvram = 1;
		}

		if(pressedButtons & BUTTON_B)
		{
			convType++;
			if(convType > 4)
				convType = 0;
			loadvram = 1;
		}

		if(pressedButtons & BUTTON_START)
			exit = 1;
			
		if(pressedButtons & BUTTON_C)
		{
			nhorRes = DrawFloatMenuRes(horRes);
			resetController(&oldButtons);
			
			if(nhorRes != FLOAT_CANCEL)
				horRes = nhorRes;
			
			loadvram = 1;
		}

		VDP_waitVSync();
	}
}

