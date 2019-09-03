/* 
 * 240p Test Suite
 * Copyright (C)2011-2014 Artemio Urbina
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

#include "mdfourier.h"

int doLock = 0;

const u16 PITCHES[NUMPITCHES] = {
	277, 293, 311, 329, 349, 369, 
	391, 415, 439, 465, 493, 522
};

struct {
	u8 msb[NUMPITCHES];
	u8 lsb[NUMPITCHES];
} pitchTable;

void Z80Lock()
{
	if(doLock)
		Z80_requestBus(1);
}

void Z80Release()
{
	if(doLock)
		Z80_releaseBus();
}

void ymGrandPianoLoad(u8 channel)
{
	u8 part = 0; //0 or 1
	
	if(channel < 3)
		part = 0;
	else
	{
		part = 1;
		channel = channel - 3;
	}
		
	Z80Lock();
	
	// Register 22H - LFO (General)
	YM2612_writeReg(part,0x22,0); 
	// Register 27H - Timers; Ch3/6 mode
	YM2612_writeReg(part,0x27,0); 

	// Register 30H+ - detune & multiple
	YM2612_writeReg(part,0x30+channel,0x71); // OP1
	YM2612_writeReg(part,0x34+channel,0x0D); // OP3
	YM2612_writeReg(part,0x38+channel,0x33); // OP2
	YM2612_writeReg(part,0x3C+channel,0x01); // OP4

	// Register 40H+ - total level: 0 being the largest and 127 (decimal) the smallest.
	YM2612_writeReg(part,0x40+channel,0x23); // OP1
	YM2612_writeReg(part,0x44+channel,0x2D); // OP3
	YM2612_writeReg(part,0x48+channel,0x26); // OP2
	YM2612_writeReg(part,0x4C+channel,0x00); // OP4

	// Register 50H+ - rate scaling; attack rate
	YM2612_writeReg(part,0x50+channel,0x5F); // OP1
	YM2612_writeReg(part,0x54+channel,0x99); // OP3
	YM2612_writeReg(part,0x58+channel,0x5F); // OP2
	YM2612_writeReg(part,0x5C+channel,0x94); // OP4

	// Register 60H+ - first decay rate; amplitude modulation
	YM2612_writeReg(part,0x60+channel,0x5); // OP1
	YM2612_writeReg(part,0x64+channel,0x5); // OP3
	YM2612_writeReg(part,0x68+channel,0x5); // OP2
	YM2612_writeReg(part,0x6C+channel,0x7); // OP4

	// Register 70H+ - secondary decay rate
	YM2612_writeReg(part,0x70+channel,0x2); // OP1
	YM2612_writeReg(part,0x74+channel,0x2); // OP3
	YM2612_writeReg(part,0x78+channel,0x2); // OP2
	YM2612_writeReg(part,0x7C+channel,0x2); // OP4

	// Register 80H+ - secondary amplitude; release rate
	YM2612_writeReg(part,0x80+channel,0x11); // OP1
	YM2612_writeReg(part,0x84+channel,0x11); // OP3
	YM2612_writeReg(part,0x88+channel,0x11); // OP2
	YM2612_writeReg(part,0x8C+channel,0xA6); // OP4

	// Register 90H+ - proprietary
	YM2612_writeReg(part,0x90+channel,0); // OP1
	YM2612_writeReg(part,0x94+channel,0); // OP3
	YM2612_writeReg(part,0x98+channel,0); // OP2
	YM2612_writeReg(part,0x9C+channel,0); // OP4

	// Register B0H+ - feedback; algorithm
	YM2612_writeReg(part,0xB0+channel,0x32); 
	
	// Register B4H+ - stereo; LFO sensitivity
	YM2612_writeReg(part,0xB4+channel,0xC0); 
	
	// Register 28H - Key off
	YM2612_writeReg(0,0x28,0x00+(part ? channel+4 : channel));
		
	Z80Release();
}

void ymInstrumentLoad(u8 channel)
{
	u8 part = 0; //0 or 1
	
	if(channel < 3)
		part = 0;
	else
	{
		part = 1;
		channel = channel - 3;
	}
		
	Z80Lock();
	
	// Register 22H - LFO (General), we are not using LFO now
	YM2612_writeReg(part,0x22, 0x00); 
	// Register 27H - Timers; Ch3/6 mode, no timmers for this, regular channel 3
	YM2612_writeReg(part,0x27, 0x00); 

	// Register 30H+ - detune & multiple
	YM2612_writeReg(part,0x30+channel,0x06); // OP1
	YM2612_writeReg(part,0x34+channel,0x06); // OP3
	YM2612_writeReg(part,0x38+channel,0x06); // OP2
	YM2612_writeReg(part,0x3C+channel,0x06); // OP4

	// Register 40H+ - total level: 0 being the largest and 127 (decimal) the smallest.
	YM2612_writeReg(part,0x40+channel,0x32); // OP1
	YM2612_writeReg(part,0x44+channel,0x21); // OP3
	YM2612_writeReg(part,0x48+channel,0x73); // OP2
	YM2612_writeReg(part,0x4C+channel,0x00); // OP4

	// Register 50H+ - rate scaling; attack rate
	YM2612_writeReg(part,0x50+channel,0x0F); // OP1
	YM2612_writeReg(part,0x54+channel,0x0F); // OP3
	YM2612_writeReg(part,0x58+channel,0x0F); // OP2
	YM2612_writeReg(part,0x5C+channel,0x0F); // OP4

	// Register 60H+ - first decay rate; amplitude modulation
	YM2612_writeReg(part,0x60+channel,0x0F); // OP1
	YM2612_writeReg(part,0x64+channel,0x0F); // OP3
	YM2612_writeReg(part,0x68+channel,0x0F); // OP2
	YM2612_writeReg(part,0x6C+channel,0x0F); // OP4

	// Register 70H+ - secondary decay rate
	YM2612_writeReg(part,0x70+channel,0x0A); // OP1
	YM2612_writeReg(part,0x74+channel,0x0A); // OP3
	YM2612_writeReg(part,0x78+channel,0x0A); // OP2
	YM2612_writeReg(part,0x7C+channel,0x0A); // OP4

	// Register 80H+ - secondary amplitude; release rate
	YM2612_writeReg(part,0x80+channel,0x08); // OP1
	YM2612_writeReg(part,0x84+channel,0x08); // OP3
	YM2612_writeReg(part,0x88+channel,0x08); // OP2
	YM2612_writeReg(part,0x8C+channel,0x08); // OP4

	// Register 90H+ - proprietary
	YM2612_writeReg(part,0x90+channel,0); // OP1
	YM2612_writeReg(part,0x94+channel,0); // OP3
	YM2612_writeReg(part,0x98+channel,0); // OP2
	YM2612_writeReg(part,0x9C+channel,0); // OP4

	// Register B0H+ - feedback; algorithm
	YM2612_writeReg(part,0xB0+channel,0x1); 
	//YM2612_writeReg(part,0xB0+channel,0x5); 
	//YM2612_writeReg(part,0xB0+channel,0x29); 
	
	// Register B4H+ - stereo; LFO sensitivity
	// R 0x40, L 0x80 or B 0xC0
	YM2612_writeReg(part,0xB4+channel,0xC0); 

	// Register 28H - Key off
	YM2612_writeReg(0,0x28,0x00+(part ? channel+4 : channel));
		
	Z80Release();
}

void ym2612_keyoff(u8 channel)
{
	u8 part = 0; //0 or 1
	
	if(channel < 3)
		part = 0;
	else
	{
		part = 1;
		channel = channel - 3;
	}
	Z80Lock();
	// Register 28H - Key off
	YM2612_writeReg(0,0x28,0x00+(part ? channel+4 : channel));
	Z80Release();
}

void ym2612_keyoffAll()
{
	u8 c = 0;

	for(c = 0; c < 6; c ++)
		ym2612_keyoff(c);
}

void ym2612_disableDAC()
{
	Z80Lock();
	YM2612_disableDAC();
	Z80Release();
}


void yminit()
{
	u8 i,j,f1,f2;
	u16 temp=0;

	Z80Lock();
	YM2612_reset();
	ym2612_disableDAC();		
	Z80Release();
	
	//separate pitches into two chars
	for (i=0; i<NUMPITCHES; i++){
		temp = PITCHES[i];
		for (j=f1=f2=0; j<11; j++){
			//set each bit to remainder (0 or 1) starting from lsb
			if (j<8) f1 += (temp % 2) << j;
			//do the same for second char
			else f2 += (temp % 2) << (j - 8);
			temp /= 2;
		}
		pitchTable.lsb[i] = f1;
		pitchTable.msb[i] = f2;
	}
	
	for(i = 0; i < 3; i++)
		ymInstrumentLoad(i);
	for(i = 3; i < 6; i++)
		ymGrandPianoLoad(i);
}

void ymPlay(u8 channel, u8 note, u8 octave, u8 pan)
{
	u8 part = 0; //0 or 1
	
	if(channel < 3)
		part = 0;
	else
	{
		part = 1;
		channel = channel - 3;
	}
		
	Z80Lock();
	
	// Register B4H+ - stereo; LFO sensitivity
	// R 0x40, L 0x80 or B 0xC0
	YM2612_writeReg(part,0xB4+channel, pan); 
	
	// Register 28H - Key off
	YM2612_writeReg(0,0x28,0x00+(part ? channel+4 : channel));

	/*
	Registers A0H-AFH - frequency
	Channel 1's frequency is in A0 and A4H.
	Channel 2's frequency is in A1 and A5H.
	Channel 3's frequency, if it is in normal mode is in A2 and A6H.
	*/
	
	YM2612_writeReg(part,0xA4+channel,octave + pitchTable.msb[note]);
	YM2612_writeReg(part,0xA0+channel,pitchTable.lsb[note]);
	
	// Register 28H - Key on
	YM2612_writeReg(0,0x28,0xF0+(part ? channel+4 : channel));
	
	Z80Release();
}

void ExecutePulseTrain()
{
	u16 f = 0;
	//Sync
	
	PSG_setFrequency(0, PULSE_TRAIN_FREQ);
	for(f = 0; f < 10; f++)
	{
		PSG_setEnvelope(0, PSG_ENVELOPE_MAX);
		VDP_waitVSync();
		PSG_setEnvelope(0, PSG_ENVELOPE_MIN);
		VDP_waitVSync();
	}
}

void ExecuteSilence()
{
	u16 frame = 0;
	
	//Silence
	for(frame = 0; frame < 20; frame++)
		VDP_waitVSync();
}

void ExecuteFM(u16 framelen)
{
	int type, octave, frame;
	
	// FM Test
	for(type = 0; type < 1; type++)
	{
		for(octave = 0; octave <= 56; octave += 8)
		{
			int chann = 0, note;
			
			for(note = 0; note < 12; note++)
			{
				ymPlay(chann, note, octave, STEREO_LEFT);

				ymPlay(chann+3, note, octave, STEREO_RIGHT);
				
				for(frame = 0; frame < framelen; frame++)
				{					
					if(frame == framelen - framelen/5)
					{
						ym2612_keyoff(chann);
						ym2612_keyoff(chann+3);
					}
					VDP_waitVSync();
				}
				chann ++;
				if(chann > 2)
					chann = 0;
			}
		}
	}
	ym2612_keyoffAll();
}



void ExecuteFMRamp()
{
	int type, octave, wait = 0;
	
	// FM Test
	for(type = 0; type < 1; type++)
	{
		for(octave = 0; octave <= 56; octave += 8)
		{
			int chann = 0, note;
			
			for(note = 0; note < 12; note++)
			{
				ymPlay(chann, note, octave, STEREO_RIGHT);
				ymPlay(chann+3, note, octave, STEREO_LEFT);
				VDP_waitVSync();
			}
		}
	}
	ym2612_keyoffAll();
	// wait 4 frame stop get 100 frames, and have decay
	for(wait = 0; wait < 4; wait++)
		VDP_waitVSync();
}



void ExecutePSG(u16 framelen)
{
	int psg, chann, frame;
	
	// PSG Test
	chann = 0;
	for(psg = 500; psg <= 20000; psg +=500)
	{
		PSG_setFrequency(chann, psg);
		PSG_setEnvelope(chann, PSG_ENVELOPE_MAX);
		for(frame = 0; frame < framelen; frame++)
		{	
			if(frame == framelen - 1)
				StopPSG();
			VDP_waitVSync();
		}	
	}
	StopPSG();
}

void ExecutePSGRamp()
{
	int psg, chann;

	// PSG Test

	chann = 0;
	for(psg = 50; psg <= 20000; psg +=50)
	{
		PSG_setFrequency(chann, psg);	
		PSG_setEnvelope(chann, PSG_ENVELOPE_MAX);

		VDP_waitVSync();        
	}
	StopPSG();
}

void ExecuteNoise(u16 framelen)
{
	int type, clk, envel, frame;
	u8 noisetypes[2] = {PSG_NOISE_TYPE_WHITE, PSG_NOISE_TYPE_PERIODIC};
	u8 clock[5] = {PSG_NOISE_FREQ_CLOCK2, PSG_NOISE_FREQ_CLOCK4, PSG_NOISE_FREQ_CLOCK8, PSG_NOISE_FREQ_TONE3 };
	
	for(envel = 0; envel < 2; envel++)
	{
		for(type = 0; type < 2; type++)
		{
			for(clk = 0; clk < 4; clk ++)			
			{
				if(clock[clk] == PSG_NOISE_FREQ_TONE3)
					PSG_setFrequency(2, 4000);
					
				PSG_setNoise(noisetypes[type], clock[clk]);
				PSG_setEnvelope(3, PSG_ENVELOPE_MAX);
				for(frame = 0; frame < framelen; frame++)
				{				
					if(envel)
						PSG_setEnvelope(3, (int)frame/(framelen/0x0F));
					if(frame == framelen - 1)
						StopPSG();
					VDP_waitVSync();
				}
			}
		}
	}
	
	// These ramps generate random noise
	// so a lot of scatter is produced
	// it doesn't help
	
	/*
	for(noise = 0; noise < 20000; noise += 100)
	{
		PSG_setNoise(PSG_NOISE_TYPE_WHITE, PSG_NOISE_FREQ_TONE3);
		PSG_setFrequency(2, noise);
		PSG_setEnvelope(3, PSG_ENVELOPE_MAX);
		VDP_waitVSync();
	}
	
	for(noise = 0; noise < 20000; noise += 100)
	{
		PSG_setNoise(PSG_NOISE_TYPE_PERIODIC, PSG_NOISE_FREQ_TONE3);
		PSG_setFrequency(2, noise);
		PSG_setEnvelope(3, PSG_ENVELOPE_MAX);
		VDP_waitVSync();
	}
	*/
	
	StopPSG();
}

#ifdef SEGACD

void SendSCDCommand(enum SCD_Command command)
{
	unsigned char *segacd_comm = (void*)0xA1200E;
	
asm("SendSCDCOMM:");
	asm("tst.b	0xA1200F");
	asm("bne	SendSCDCOMM");

	*segacd_comm = 0;
asm("SendSCDWait:");
	asm("tst.b	0xA1200F");
	asm("beq	SendSCDWait");
	
	*segacd_comm = command;
asm("SendSCDWait2:");
	asm("tst.b	0xA1200F");
	asm("bne	SendSCDWait2");
}


void PlayCDTrack()
{
	int i = 0;
	
	PSG_setFrequency(0, 500);
	PSG_setEnvelope(0, PSG_ENVELOPE_MAX);
	VDP_waitVSync();
	PSG_setEnvelope(0, PSG_ENVELOPE_MIN);
		
	SendSCDCommand(Op_PlayCD);
	
	// wait 10012.8 ms 16.688*600 + 20 frames for 300ms sync pulse + 20 frames to capture the whole range
	for(i = 0; i < 640; i++)
		VDP_waitVSync();
		
	SendSCDCommand(Op_StopCD);
		
	PSG_setFrequency(0, 500);
	PSG_setEnvelope(0, PSG_ENVELOPE_MAX);
	VDP_waitVSync();
	PSG_setEnvelope(0, PSG_ENVELOPE_MIN);
}

void PlayPCM()
{	
	int i = 0;
	
	PSG_setFrequency(0, 500);
	PSG_setEnvelope(0, PSG_ENVELOPE_MAX);
	VDP_waitVSync();
	PSG_setEnvelope(0, PSG_ENVELOPE_MIN);
	
	SendSCDCommand(Op_PlayPCM);
	
	// wait 2048 ms/16.688 = 122.7229 frames.
	// we wait 140
	
	for(i = 0; i < 140; i++)
		VDP_waitVSync();
		
	SendSCDCommand(Op_StopPCM);
		
	PSG_setFrequency(0, 500);
	PSG_setEnvelope(0, PSG_ENVELOPE_MAX);
	VDP_waitVSync();
	PSG_setEnvelope(0, PSG_ENVELOPE_MIN);
	
	ExecuteSilence();
}

void TestPCM()
{	
	int i = 0;
	
	PSG_setFrequency(0, 500);
	PSG_setEnvelope(0, PSG_ENVELOPE_MAX);
	VDP_waitVSync();
	PSG_setEnvelope(0, PSG_ENVELOPE_MIN);
	
	SendSCDCommand(Op_TestPCM);
	
	// wait 2048 ms/16.688 = 122.7229 frames.
	// we wait 140
	
	for(i = 0; i < 140; i++)
		VDP_waitVSync();
		
	SendSCDCommand(Op_StopPCM);
		
	PSG_setFrequency(0, 500);
	PSG_setEnvelope(0, PSG_ENVELOPE_MAX);
	VDP_waitVSync();
	PSG_setEnvelope(0, PSG_ENVELOPE_MIN);
	
	ExecuteSilence();
}

void ChangePCM(int *type)
{	
	int command[] = { Op_SetSamplesSweep, Op_SetSampSin32000, Op_SetSampSin32552, Op_SetSampSin32604 };
	
	if(*type < 0)
		*type = 3;
		
	if(*type > 3)
		*type = 0;
	
	SendSCDCommand(command[*type]);
	
	// Test it
	//PlayPCM();
}

#endif

void ExececuteMDF(u16 framelen)
{	
#ifdef SEGACD
	int i = 0;
#endif

	yminit();
	PSG_init();
	VDP_waitVSync();
	
	ExecutePulseTrain();
	ExecuteSilence();
	
	ExecuteFM(framelen);
	ExecutePSG(framelen);
	ExecutePSGRamp();
	ExecuteNoise(framelen);
	
	ExecuteSilence();
	ExecutePulseTrain();
	
#ifdef SEGACD
	// Wait PAUSE
	for(i = 0; i < 4; i++)	
		VDP_waitVSync();
		
	PlayPCM();
	PlayCDTrack();
#endif
}

void MDFourier()
{
	int loadvram = 1, debug = 0;
#ifdef SEGACD	
	int sampleLoaded = 0, frequency = 0x0800;
#endif
	u16 ind = 0, size = 0, exit = 0;
	u16 buttons, oldButtons = 0xffff, pressedButtons;
	u16 len = 0, redraw = 0, framelen = 20;

#ifdef SEGACD	
	SendSCDCommand(Op_InitCD);
#endif
	
	while(!exit)
	{
		if(loadvram)
		{
			len = sizeof(beep);
			VDP_setPalette(PAL0, palette_grey);
			VDP_setPalette(PAL1, palette_green);
			VDP_setPalette(PAL2, back_pal);
			VDP_setPalette(PAL3, palette_red);

			ind = TILE_USERINDEX;
			size = sizeof(back_tiles) / 32;
			VDP_loadTileData(back_tiles, ind, size, USE_DMA);

			VDP_setMyTileMapRect(BPLAN, back_map, TILE_USERINDEX, 0, 0, 320 / 8, 224 / 8);
			loadvram = 0;
			redraw = 1;
		}
		
		if(redraw)
		{	
			VDP_clearTileMapRect(APLAN, 0, 0, 320 / 8, 224 / 8);
			
			if(debug)
				VDP_drawTextBG(APLAN, "MDFourier 0.947 (Test mode)", TILE_ATTR(PAL1, 0, 0, 0), 5, 6);
			else
				VDP_drawTextBG(APLAN, "MDFourier 0.947", TILE_ATTR(PAL1, 0, 0, 0), 10, 6);
			
			VDP_drawTextBG(APLAN, "Auto sequence", TILE_ATTR(PAL1, 0, 0, 0), 12, 9);
			VDP_drawTextBG(APLAN, "Start recording and press A", TILE_ATTR(PAL2, 0, 0, 0), 6, 10);
			if(debug)
			{
#ifdef SEGACD	
				char buffer[6];
#endif
				
				VDP_drawTextBG(APLAN, doLock ? "Z80 BUS Request [B] ON " : "Z80 BUS Request [B] OFF", TILE_ATTR(PAL1, 0, 0, 0), 8, 15);

#ifdef SEGACD	
				VDP_drawTextBG(APLAN, "PCM tone [C] 0x0800", TILE_ATTR(PAL0, 0, 0, 0), 8, 17);
				VDP_drawTextBG(APLAN, "PCM Sample used, change w/[X]", TILE_ATTR(PAL0, 0, 0, 0), 5, 18);
				switch(sampleLoaded)
				{
					case 0:
						VDP_drawTextBG(APLAN, "SWEEP 32552hz", TILE_ATTR(PAL1, 0, 0, 0), 12, 19);
						break;
					case 1:
						VDP_drawTextBG(APLAN, "SINE 1Khz@32000hz", TILE_ATTR(PAL1, 0, 0, 0), 12, 19);
						break;
					case 2:
						VDP_drawTextBG(APLAN, "SINE 1Khz@32552hz", TILE_ATTR(PAL1, 0, 0, 0), 12, 19);
						break;
					case 3:
						VDP_drawTextBG(APLAN, "SINE 1Khz@32604hz", TILE_ATTR(PAL1, 0, 0, 0), 12, 19);
						break;
				}
				
				VDP_drawTextBG(APLAN, "PCM ratio Test [Y]", TILE_ATTR(PAL0, 0, 0, 0), 9, 20);
				VDP_drawTextBG(APLAN, "change w/[L or R]: 0x", TILE_ATTR(PAL0, 0, 0, 0), 7, 21);
				intToHex(frequency, buffer, 4);
				VDP_drawTextBG(APLAN, buffer, TILE_ATTR(PAL0, 0, 0, 0), 28, 21);
#endif
			}
			redraw = 0;
		}

		buttons = JOY_readJoypad(JOY_1);
		pressedButtons = buttons & ~oldButtons;
		oldButtons = buttons;

		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_SOUND))
			loadvram = 1;

		if(pressedButtons & BUTTON_A)
		{
			VDP_drawTextBG(APLAN, "Please wait while recording", TILE_ATTR(PAL3, 0, 0, 0), 6, 10);
			VDP_waitVSync();
			
			ExececuteMDF(framelen);
			
			redraw = 1;
			
			VDP_clearTileMapRect(APLAN, 0, 0, 320 / 8, 224 / 8);
			//loadvram = 1;

		}
		
		if(CheckHelpAndVO(&buttons, &pressedButtons, HELP_SOUND))
			loadvram = 1;
		
		if(debug)
		{
			if(pressedButtons & BUTTON_B)
			{
				doLock = !doLock;
				redraw = 1;
			}
		}
		
		if(!debug && (pressedButtons & BUTTON_X
		|| (buttons & BUTTON_UP && buttons & BUTTON_B)))
		{
			debug = 1;
			redraw = 1;
		}

#ifdef SEGACD			
		if(debug)
		{
			if(pressedButtons & BUTTON_C)
				PlayPCM();
			
			if(pressedButtons & BUTTON_X)
			{
				//SendSCDCommand(Op_SeekCD);
				//PlayCDTrack();
				
				sampleLoaded ++;
				ChangePCM(&sampleLoaded);
				redraw = 1;
			}
			
			if(pressedButtons & BUTTON_Y)
				TestPCM();

			if(pressedButtons & BUTTON_LEFT)
			{
				SendSCDCommand(Op_IncremetPCMFreq); 	//Increment the internal value by 1
				frequency ++;
				redraw = 1;
			}
			
			if(pressedButtons & BUTTON_RIGHT)
			{
				SendSCDCommand(Op_DecrementPCMFreq); 	//Decrement the internal value by 1
				frequency --;
				redraw = 1;
			}
		}
#endif

		if(!debug && pressedButtons & BUTTON_START)
			exit = 1;
			
		if(debug && pressedButtons & BUTTON_START)
		{
			redraw = 1;
			debug = 0;
		}
	}
	
	ym2612_keyoffAll();
	SND_stopPlay_PCM();
	StopPSG();
	VDP_waitVSync();
}
