#pragma once

#include <SDL2/SDL.h>
#include "../config.h"
#include "../Snipes.h" // console.h needs MazeTile
#include "../console.h"

// Draws the slim status bar across the top barHeight pixels of a w-pixel-wide
// renderer: snipes and portals left, game time and score, with lives and the
// level code at the right.
void DrawHud(SDL_Renderer *ren, int w, int barHeight, const HudValues &v);
