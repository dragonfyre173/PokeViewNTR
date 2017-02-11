#include "global.h"
#include "ov.h"
#include "pokeutil/pokemon.h"
#include "pokeutil/lookup.h"
#include "helpers.h"

FS_archive sdmcArchive = { 0x9, (FS_path){ PATH_EMPTY, 1, (u8*)"" } };
Handle fsUserHandle = 0;

#define CALLBACK_OVERLAY (101)

#define TICKS_PER_MSEC (268123.480)
#define BLANK 255,255,255

const int POKE_SIZE = 232;

u32 IoBasePad = 0xFFFD4000;
vu32 lastPid = 0;
vu8 displayMoves = 0;
vu8 displayMovesChanged = 0;
vu8 enabled = 1;
vu8 isNewNtr = 0;
Opponent selectedOpponent = PRIMARY;

u8 buttonAck = 0;

u32* battleOff = (u32*)0x6747D8;
u32 inBattle = 0x40400000;

u32 isInBattle() {
	for(int i = 0; i < 0x10000; i++) {
		if(*battleOff == inBattle + i) {
			return 1;
		}
	}
	return 0;
}

void drawPokemonID() 
{

	u8 pkbytes[232];
	Pokemon* pkm = (Pokemon*)pkbytes;

	if(pkm != 0)
	{
		decryptPokemon(selectedOpponent, pkm);

		char buf[100];
		char nick[13];
		int posX = 14;
		int posY = 11;
		// Bounding box
		OvDrawTranspartBlackRect(posX, 9, 125, 87, 1);

		// See if the pokemon's actually valid first
		if(isValid(pkm)) 
		{
			// This value will be updated when the battle is over
			// Now we're good to display it
			// Pokemon name
			asciiNick(pkm, nick);
			xsprintf(buf, "[%d] %s", selectedOpponent + 1, nick);
			posY = OvDrawString(buf, posX, posY, BLANK); 

			posX += 10;

			//Nature
		  	if(pkm->nature < NATURE_COUNT)
		  	{
				xsprintf(buf, "%s", NATURE_LOOKUP[pkm->nature]);
				posY = OvDrawString(buf, posX, posY, BLANK);
			}

			
			// Ability
			if(pkm->ability < ABILITY_COUNT)
			{
				xsprintf(buf, "%s", ABILITY_LOOKUP[pkm->ability]);
				posY = OvDrawString(buf, posX, posY, BLANK);
			}

			// Held item
			if(pkm->heldItem < ITEM_COUNT)
			{
				xsprintf(buf, "%s", ITEM_LOOKUP[pkm->heldItem]);
				posY = OvDrawString(buf, posX, posY, BLANK);
			}

			posX += 6;
			// IVs
			for(u8 i = 0; i < 6; i++)
			{
				u8 iv = getIV(pkm, i);
				xsprintf(buf, "%3s: %2d", STAT_NAME[i], iv);
				u8 r = iv > 29 ? 0 : 255;
				u8 g = iv < 2 ? 0 : 255;
				u8 b = (iv > 29 || iv < 2) ? 0 : 255;
				OvDrawString(buf, posX + (60 * (i / 3)),  posY + (12 * (i % 3)), r, g, b);
			}
		} 
		else 
		{
			xsprintf(buf, "[%d] Invalid Pokemon", selectedOpponent + 1);
			OvDrawString(buf, posX, posY, BLANK);
		}
	}
}

void handleKey() {
	u32 curkey = getKey();
	if(buttonAck) {
		if(curkey == 0) buttonAck = 0;
	} else {
	switch(curkey) {
		case BUTTON_ENABLE:
			buttonAck = 1;
			enabled = !enabled;
			break;
		case BUTTON_NEXT_TARGET:
			buttonAck = 1;
			selectedOpponent = (selectedOpponent + 1) % NUM_OPPONENTS;
			break;
		case BUTTON_PREV_TARGET:
			buttonAck = 1;
			selectedOpponent = (selectedOpponent + NUM_OPPONENTS - 1) % NUM_OPPONENTS;
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

u32 overlayCallback(u32 isBottom, u32 addr, u32 addrB, u32 stride, u32 format)
{
	// Set draw settings
	OvSettings(addr, addrB, stride, format, !isBottom);

	handleKey();
	if(enabled && isBottom == 0) 
	{
		u8 battle = isInBattle();
		if(battle) 
		{
			drawPokemonID();
			return 0;
		}
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
