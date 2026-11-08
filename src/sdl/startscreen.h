#pragma once

#include <SDL2/SDL.h>
#include "startscreen_logic.h"

// Draws the graphical start screen filling a w×h pixel renderer: the SNIPES
// art, the skill and number strips, the level card, controls and credits.
void DrawStartScreen(SDL_Renderer *ren, int w, int h, const StartSelection &sel);
