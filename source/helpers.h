#ifndef HELPERS_H
#define HELPERS_H

#include "pokeutil/pokemon.h"

#define BUTTON_ENABLE BUTTON_ST     // Toggles view between off/on
#define BUTTON_NEXT_TARGET BUTTON_R // Switches target pokemon
#define BUTTON_PREV_TARGET BUTTON_L // Switches between displaying pokemon info and moves

// https://github.com/hartmannaf/PokemonCheatPlugin

extern u32 IoBasePad;

u32 getKey();
void waitKeyUp();
void waitKeyCombinationChanged(u32 buttonCombination);

#endif
