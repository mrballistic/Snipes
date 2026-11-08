#pragma once

#include "../types.h"

// Formats the game's frame counter (about 18.2 frames per second, the PC
// timer rate) as minutes and seconds, e.g. "1:24" or "59:59".
void FormatGameTime(WORD frame, char out[8]);
