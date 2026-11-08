#pragma once

// Fonts and drawing helpers for the graphical screens (start screen and the
// in-game bar). All sizes are in renderer (pixel) units.

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Opens the game's console font (SnipesConsole.ttf or a fallback); defined in console.cpp.
TTF_Font *OpenGameFont(int size);

// Directory holding bundled resources, with a trailing slash: the app bundle's
// Resources directory, or "" (the working directory) otherwise; defined in console.cpp.
const char *ResourceDirectory();

// IBM Plex Mono from fonts/, falling back to a system sans-serif (Helvetica
// Neue, DejaVu Sans, Segoe UI); cached by size and weight.
TTF_Font *GetUIFont(int pixelSize, bool bold);
// The game's console font at an arbitrary size, with smooth hinting; used for
// the SNIPES art, arrow keycaps and lives faces.
TTF_Font *GetGlyphFont(int pixelSize);
void CloseUIFonts(); // also frees cached text textures

enum TextAlign { Align_Left, Align_Center, Align_Right };

// Width in pixels of UTF-8 text, optionally with extra spacing between characters.
int MeasureUIText(TTF_Font *font, const char *utf8, int tracking = 0);
// Draws UTF-8 text with its top edge at y; x is the left edge, center or right
// edge depending on align. Returns the drawn width.
int DrawUIText(SDL_Renderer *ren, TTF_Font *font, const char *utf8, int x, int y,
               SDL_Color color, TextAlign align = Align_Left, int tracking = 0);

void FillRoundedRect(SDL_Renderer *ren, SDL_Rect r, int radius, SDL_Color color);
// A soft halo around a rounded rect, for the selected tile's glow.
void DrawRoundedGlow(SDL_Renderer *ren, SDL_Rect r, int radius, int spread, SDL_Color color);

inline SDL_Color RGB(Uint32 hex, Uint8 alpha = 0xFF)
{
	SDL_Color c = { (Uint8)(hex >> 16), (Uint8)(hex >> 8), (Uint8)hex, alpha };
	return c;
}
