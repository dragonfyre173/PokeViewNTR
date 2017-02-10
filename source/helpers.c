#include "helpers.h"
#include "global.h"

u32 getKey() {
	return (*(vu32*)(IoBasePad) ^ 0xFFF) & 0xFFF;
}

void waitKeyUp() {
	while (getKey() != 0) {
		svc_sleepThread(100000000);
	}
}
void waitKeyCombinationChanged(u32 buttonCombination) {
	while (getKey() == buttonCombination) {
		svc_sleepThread(100000000);
	}
}

