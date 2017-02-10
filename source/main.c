#include "global.h"
#include "ov.h"
#include "pokemon.h"
#include "lookup.h"
#include "helpers.h"

FS_archive sdmcArchive = { 0x9, (FS_path){ PATH_EMPTY, 1, (u8*)"" } };
Handle fsUserHandle = 0;

#define CALLBACK_OVERLAY (101)

#define TICKS_PER_MSEC (268123.480)

const int POKE_SIZE = 232;

u32 IoBasePad = 0xFFFD4000;
vu32 lastPid = 0;
vu8 displayMoves = 0;
vu8 displayMovesChanged = 0;
vu8 enabled = 0;
vu8 isNewNtr = 0;
Opponent selectedOpponent = PRIMARY;

u8 buttonAck = 0;

void drawPokemonID(int isBottom, u32 addr, u32 stride, u32 format, u32 colOffset) {
	u8 pkm[232];
	
	decryptPokemon(OPPONENT_POINTERS[selectedOpponent], pkm);
	Pokemon* poke = (Pokemon*)pkm;
	
	u32 height = 400;
	char buf[100];
	char nick[13];
	// Bounding box
	ovDrawTranspartBlackRect(addr, stride, format, 9, colOffset, 77 + 4, 170 + 4, 1);
	
	// Pokemon name
	asciiNick(poke, nick);
	xsprintf(buf, "#%d: %s", selectedOpponent + 1, nick);
	ovDrawString(addr, stride, format, height, 11, colOffset + 4, 255, 255, 255, buf);
	if(displayMoves) {
		// TODO: implement move display
	} else {
		
		// Nature
		if(poke->nature < NATURE_COUNT) {
			xsprintf(buf, "%s", NATURE_LOOKUP[poke->nature]);
			ovDrawString(addr, stride, format, height, 22, colOffset + 14, 255, 255, 255, buf);
		}
		// Ability
		if(poke->ability < ABILITY_COUNT) {
			xsprintf(buf, "%s", ABILITY_LOOKUP[poke->ability]);
			ovDrawString(addr, stride, format, height, 33, colOffset + 14, 255, 255, 255, buf);
		}
		
		// Held item
		if(poke->heldItem < ITEM_COUNT) {
			xsprintf(buf, "%s", ITEM_LOOKUP[poke->heldItem]);
			ovDrawString(addr, stride, format, height, 44, colOffset + 14, 255, 255, 255, buf);
		}
		
		// IVs
		for(u8 i = 0; i < 6; i++) {
			u8 iv = getIV(poke, i);
			xsprintf(buf, "%3s: %2d", STAT_NAME[i], iv);
			u8 r = iv > 29 ? 0 : 255;
			u8 g = iv < 2 ? 0 : 255;
			u8 b = (iv > 29 || iv < 2) ? 0 : 255;
			ovDrawString(addr, stride, format, height,  11 * ((i % 3) + 5), colOffset + 24 + (80 * (i / 3)), r, g, b, buf);
		}
	}
	
	//ovDrawString(addr, stride, format, height, 11, colOffset + 4, 255, 255, 255, buf);
}

void handleKey() {
	u32 curkey = getKey();
	if(buttonAck) {
		if(curkey == 0) buttonAck = 0;
	} else {
	switch(curkey) {
		case BUTTON_ENABLE:
			buttonAck = 1;
			enabled = ~enabled;
			selectedOpponent = PRIMARY;
			break;
		case BUTTON_SWAP_TARGET:
			buttonAck = 1;
			selectedOpponent = (selectedOpponent + 1) % NUM_OPPONENTS;
			break;
		case BUTTON_TOGGLE_VIEW:
		
			break;
		}
	}
}

/*
Overlay Callback.
isBottom: 1 for bottom screen, 0 for top screen.
addr: writable cached framebuffer virtual address, should flush data cache after modifying.
addrB: right-eye framebuffer for top screen, undefined for bottom screen.
stride: framebuffer stride(pitch) in bytes, at least 240*bytes_per_pixel.
format: framebuffer format, see https://www.3dbrew.org/wiki/GPU/External_Registers for details.

return 0 on success. return 1 when nothing in framebuffer was modified.
*/

u32 overlayCallback(u32 isBottom, u32 addr, u32 addrB, u32 stride, u32 format) {
	handleKey();
	if(enabled && isBottom == 0) {
		drawPokemonID(isBottom, addr, stride, format, 14);
		if ((isBottom == 0) && (addrB) && (addrB != addr))  {
			drawPokemonID(isBottom, addrB, stride, format, 10);
		}
		return 0;
	}
	return 1;
}

int main() {
	initSharedFunc();
	
	if (((NS_CONFIG*)(NS_CONFIGURE_ADDR))->sharedFunc[8]) {
		isNewNtr = 1;
	} else {
		isNewNtr = 0;
	}
	
	if (isNewNtr) {
		IoBasePad = plgGetIoBase(IO_BASE_PAD);
	}
	
	plgRegisterCallback(CALLBACK_OVERLAY, (void*) overlayCallback, 0);
	
	return 0;
}

