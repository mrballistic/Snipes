#include <stdio.h>
#include <string.h>
#include "hud.h"
#include "hudformat.h"
#include "uifont.h"

// Layout follows the approved mockup (hud-final-v4): a 34px bar across a 720px
// window. Measurements are in mockup pixels, scaled by u to the actual bar.

namespace {

const Uint32 BarFill = 0x0b1020, Divider = 0x1d2740, Label = 0x8f99b3, Value = 0xffffff,
             PipOn = 0x7fe7ff, PipOff = 0x22304d, Lives = 0xffd36b;

const Uint MaxPips = 10;  // the most portals any level starts with
const Uint MaxLives = 5;

struct Bar
{
	SDL_Renderer *ren;
	double u;
	int height, x;
	TTF_Font *label, *value;
	int px(double mockupPx) const { return (int)(mockupPx * u + 0.5); }
	int TextTop(TTF_Font *f) const { return (height - TTF_FontHeight(f)) / 2; }

	// A label followed by a value right-aligned in a slot wide enough for slotSample
	void Stat(const char *labelText, const char *valueText, const char *slotSample)
	{
		int tracking = px(0.9);
		x += DrawUIText(ren, label, labelText, x, TextTop(label) + px(1.5), RGB(Label), Align_Left, tracking) + px(7);
		int slot = MeasureUIText(value, slotSample);
		DrawUIText(ren, value, valueText, x + slot, TextTop(value), RGB(Value), Align_Right);
		x += slot;
	}
	void Gap() { x += px(24); }
};

} // namespace

void DrawHud(SDL_Renderer *ren, int w, int barHeight, const HudValues &v)
{
	Bar bar;
	bar.ren = ren;
	bar.u = barHeight / 34.0;
	bar.height = barHeight;
	bar.x = bar.px(18);
	bar.label = GetUIFont(bar.px(11), false);
	bar.value = GetUIFont(bar.px(17), true);

	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
	SDL_Color fill = RGB(BarFill), line = RGB(Divider);
	SDL_SetRenderDrawColor(ren, fill.r, fill.g, fill.b, 0xFF);
	SDL_Rect r = { 0, 0, w, barHeight };
	SDL_RenderFillRect(ren, &r);
	SDL_SetRenderDrawColor(ren, line.r, line.g, line.b, 0xFF);
	SDL_Rect divider = { 0, barHeight - SDL_max(1, bar.px(1)), w, SDL_max(1, bar.px(1)) };
	SDL_RenderFillRect(ren, &divider);

	char text[16];
	snprintf(text, sizeof(text), "%u", v.snipes);
	bar.Stat("SNIPES", text, "888");
	bar.Gap();

	snprintf(text, sizeof(text), "%u", v.portals);
	bar.Stat("PORTALS", text, "88");
	{
		// One pip per starting portal, lit for those remaining; the slot always fits ten
		int pip = SDL_max(2, bar.px(6)), gap = SDL_max(1, bar.px(2));
		int x = bar.x + bar.px(7), y = (barHeight - pip) / 2 + bar.px(1);
		for (Uint i = 0; i < v.portalsAtStart && i < MaxPips; i++)
		{
			SDL_Rect p = { x + (int)i * (pip + gap), y, pip, pip };
			FillRoundedRect(ren, p, SDL_max(1, pip / 3), RGB(i < v.portals ? PipOn : PipOff));
		}
		bar.x += bar.px(7) + (int)MaxPips * (pip + gap) - gap;
	}
	bar.Gap();

	FormatGameTime(v.frame, text);
	bar.Stat("TIME", text, "88:88");
	bar.Gap();

	snprintf(text, sizeof(text), "%06u", v.score);
	bar.Stat("SCORE", text, "888888");

	// Right side: lives as yellow faces in the game font, then the level code
	int right = w - bar.px(18);
	snprintf(text, sizeof(text), "%c%u", (char)('A' + v.letter), v.number);
	right -= DrawUIText(ren, bar.label, text, right, bar.TextTop(bar.label) + bar.px(1.5), RGB(Label), Align_Right, bar.px(1.1));
	right -= bar.px(8);
	TTF_Font *faces = GetGlyphFont(bar.px(14));
	char lives[MaxLives * 3 + 1] = {}; // "☺" is 3 bytes of UTF-8
	for (Uint i = 0; i < v.lives && i < MaxLives; i++)
		strcat(lives, "☺");
	DrawUIText(ren, faces, lives, right, bar.TextTop(faces), RGB(Lives), Align_Right);
}
