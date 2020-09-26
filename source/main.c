//SPDX-License-Identifier: BSD-3-Clause
//SPDX-FileCopyrightText: 2020 Lorenzo Cauli (lorecast162)

//libnds include
#include <nds.h>

//sprites common palette data
#include <sprites.h>

//sprite tilesets
#include <rotSphere.h>
#include <blauSphere.h>
#include <gruenSphere.h>

//sprite size defines
#define SPRITE_WIDTH  64
#define SPRITE_HEIGHT 64

int main() {
	//sprite coordinates
	u16 x = 96, y = 160;
	u16 yinc = 2;

	//bools to decide which version of the sprite gets hidden
	//used to hide wraparound when going too far on one screen
	bool topHidden = false;
	bool bottomHidden = false;

	//palette selectors
	u8 curPalette = 0;
	u8 prevPalette = curPalette;

	//initialize all 2D engines
	powerOn(POWER_ALL_2D);

	//set VRAM banks
	vramSetBankA(VRAM_A_MAIN_SPRITE);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	//set top screen video mode
	videoSetMode(MODE_0_2D);

	//set bottom screen video mode
	videoSetModeSub(MODE_0_2D);

	//initialize OAM for top and bottom screens
	oamInit(&oamMain, SpriteMapping_1D_32, false);
	oamInit(&oamSub, SpriteMapping_1D_32, false);

	//load sprites for both screens' OAMs
	u16* spriteMainOAM = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_256Color);
	u16* spriteSubOAM  = oamAllocateGfx(&oamSub,  SpriteSize_64x64, SpriteColorFormat_256Color);

	//copy palette and data to OAMs
	dmaCopyHalfWords(3, spritesPal, SPRITE_PALETTE, spritesPalLen);
	dmaCopyHalfWords(3, spritesPal, SPRITE_PALETTE_SUB, spritesPalLen);
	dmaCopyHalfWords(3, rotSphereTiles, spriteMainOAM, rotSphereTilesLen);
	dmaCopyHalfWords(3, rotSphereTiles, spriteSubOAM, rotSphereTilesLen);

	//frame counter var. unused for now
	u16 frames = 0;

	//stop var
	u8 stop = 0;
	while (!stop) {
		//wait for vblank
		swiWaitForVBlank();

		//get current input state
		scanKeys();

		//get currently held keys
		u32 held = keysHeld();

		//kill game
		if (held & KEY_START) stop = 1;
		
		//movement
		if (held & KEY_UP && ((y - yinc) > 0)) y -= yinc;
		else if (held & KEY_DOWN && ((y + yinc) < (384 - SPRITE_HEIGHT))) y += yinc;

		//palette switching
		if (held & KEY_RIGHT) curPalette = 2;
		else if (held & KEY_LEFT) curPalette = 1;
		else curPalette = 0;

		//palette selector handling with check to avoid redundant DMA
		if (curPalette != prevPalette) {
			switch (curPalette) {
				case 0:
					dmaCopyHalfWords(3, rotSphereTiles, spriteMainOAM, rotSphereTilesLen);
					dmaCopyHalfWords(3, rotSphereTiles, spriteSubOAM, rotSphereTilesLen);
					break;
				case 1:
					dmaCopyHalfWords(3, blauSphereTiles, spriteMainOAM, blauSphereTilesLen);
					dmaCopyHalfWords(3, blauSphereTiles, spriteSubOAM, blauSphereTilesLen);
					break;
				case 2:
					dmaCopyHalfWords(3, gruenSphereTiles, spriteMainOAM, gruenSphereTilesLen);
					dmaCopyHalfWords(3, gruenSphereTiles, spriteSubOAM, gruenSphereTilesLen);
					break;
			}
		}

		//checks to select which version of the sprite to hide
		if (y < 192 - SPRITE_HEIGHT) {topHidden = false; bottomHidden = true;}
		else if (y < 192 && y >= 192 - SPRITE_HEIGHT) { topHidden = false; bottomHidden = false; }
		else if (y >= 192 && y < 384) { topHidden = true; bottomHidden = false; }

		//update oam sprite attributes for top screen
		oamSet(
				&oamMain,						//context (in this case main engine)
				0,								//oam index
				x, y,							//sprite coords
				0,								//priority
				0,								//palette index
				SpriteSize_64x64,				//sprite size
				SpriteColorFormat_256Color,		//sprite color depth
				spriteMainOAM,					//pointer to loaded graphics
				-1,								//sprite rotation
				false,							//double the size when rotating?
				topHidden,						//hide sprite?
				false, false,					//vflip, hflip
				false							//apply mosaic filter
		);

		//update oam sprite attributes for bottom screen
		oamSet(
				&oamSub,						//context (in this case sub engine)
				0,								//oam index
				x, y-192,						//sprite coords
				0,								//priority
				0,								//palette index
				SpriteSize_64x64,				//sprite size
				SpriteColorFormat_256Color,		//sprite color depth
				spriteSubOAM,					//pointer to loaded graphics
				-1,								//sprite rotation
				false,							//double the size when rotating?
				bottomHidden,					//hide sprite?
				false, false,					//vflip, hflip
				false							//apply mosaic filter
		);

		//update OAM
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);

		//increment frame counter
		frames++;

		//assign current palette to previous palette
		prevPalette = curPalette;

	}
	return 0;
}
