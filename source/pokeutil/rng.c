#include "rng.h"

u32 lcrng(u32 seed){
	return (seed * 0x41C64E6D + 0x00006073);
}

// TODO: Possible RNG manipulation tools?
// Seems a little difficult, but maybe egg manipulation...
