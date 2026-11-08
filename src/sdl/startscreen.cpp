#include <stdio.h>
#include <string.h>
#include "startscreen.h"
#include "uifont.h"
#include "../skill.h"

// Layout follows the approved mockup (start-screen-a-v2), which was drawn for a
// 720×600 window; every measurement below is in those mockup pixels and is
// scaled by u to the actual window.

namespace {

const Uint32 Background = 0x05070d, Accent = 0x7fe7ff, AccentGlow = 0x2bb8ff, DeepGlow = 0x0a6cff,
             Label = 0x8f99b3, LabelKeys = 0x6f7a94, StripText = 0xa4adc4, CardFill = 0x0d1322,
             CardBorder = 0x1d2740, ChipOff = 0x18213a, ChipOnFill = 0x123a2c, ChipOnText = 0x6ff0b0,
             KeyText = 0xc1c8d8, KeyFill = 0x121a2c, KeyBorder = 0x3a4560, KeyCapText = 0xe8ecf5,
             White = 0xffffff, Credits = 0x8f99b3;

// The original title art (TITLE_SCREEN in Snipes.cpp): ↑ and ☺ in the game font.
const char *const Art[] =
{
	"    ↑↑↑  ↑   ↑ ↑↑↑ ↑↑↑↑  ↑↑↑↑↑  ↑↑↑",
	"   ↑☺☺☺↑ ☺   ☺ ☺☺☺ ☺☺☺☺↑ ☺☺☺☺☺ ↑☺☺☺↑",
	"   ☺   ☺ ↑↑  ↑  ↑  ↑   ☺ ↑     ☺   ☺",
	"   ↑     ☺☺  ☺  ☺  ☺   ↑ ☺     ↑",
	"   ☺↑↑↑  ↑ ↑ ↑  ↑  ↑↑↑↑☺ ↑↑↑↑  ☺↑↑↑",
	"    ☺☺☺↑ ☺ ☺ ☺  ☺  ☺☺☺☺  ☺☺☺☺   ☺☺☺↑",
	"       ☺ ↑  ↑↑  ↑  ↑     ↑         ☺",
	"   ↑   ↑ ☺  ☺☺  ☺  ☺     ☺     ↑   ↑",
	"   ☺↑↑↑☺ ↑   ↑ ↑↑↑ ↑     ↑↑↑↑↑ ☺↑↑↑☺",
	"    ☺☺☺  ☺   ☺ ☺☺☺ ☺     ☺☺☺☺☺  ☺☺☺",
};

struct Scale
{
	double u; int originX, originY; // origin of the 720×600 mockup frame, centered in the window
	int operator()(double mockupPx) const { return (int)(mockupPx * u + 0.5); }
	int X(double mockupX) const { return originX + (*this)(mockupX); }
	int Y(double mockupY) const { return originY + (*this)(mockupY); }
};

void DrawArt(SDL_Renderer *ren, const Scale &s)
{
	TTF_Font *font = GetGlyphFont(s(15));
	int lineHeight = s(15);
	// The art's lines are left-aligned with each other, so center the widest one
	int widest = 0;
	for (size_t i = 0; i < SDL_arraysize(Art); i++)
	{
		int w = MeasureUIText(font, Art[i]);
		if (w > widest) widest = w;
	}
	// The leading spaces are part of the original art; trim the common indent so it centers
	int indent = MeasureUIText(font, "   ");
	int left = s.X(360) - (widest - indent) / 2 - indent;
	int top = s.Y(34);

	// A soft phosphor glow: the art repeated at small offsets in deeper blues, then the art itself
	const int rings = 3;
	for (int ring = rings; ring >= 1; ring--)
	{
		int d = (int)(ring * s.u * 1.6 + 0.5);
		if (d < 1) d = 1;
		SDL_Color c = RGB(ring > 1 ? DeepGlow : AccentGlow, (Uint8)(ring > 1 ? 18 : 34));
		static const int dirs[8][2] = { {1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,-1},{1,-1},{-1,1} };
		for (size_t i = 0; i < SDL_arraysize(Art); i++)
			for (int k = 0; k < 8; k++)
				DrawUIText(ren, font, Art[i], left + dirs[k][0] * d, top + (int)i * lineHeight + dirs[k][1] * d, c);
	}
	for (size_t i = 0; i < SDL_arraysize(Art); i++)
		DrawUIText(ren, font, Art[i], left, top + (int)i * lineHeight, RGB(Accent));
}

// A centered label such as "ENEMY SKILL ← →": letter-spaced caps plus arrow hints.
void DrawStripLabel(SDL_Renderer *ren, const Scale &s, double y, const char *text, const char *arrows)
{
	TTF_Font *labelFont = GetUIFont(s(11), false);
	TTF_Font *arrowFont = GetUIFont(s(11), false);
	int tracking = s(1.8);
	int gap = s(6);
	int labelW = MeasureUIText(labelFont, text, tracking);
	int arrowW = MeasureUIText(arrowFont, arrows);
	int x = s.X(360) - (labelW + gap + arrowW) / 2;
	DrawUIText(ren, labelFont, text, x, s.Y(y), RGB(Label), Align_Left, tracking);
	DrawUIText(ren, arrowFont, arrows, x + labelW + gap, s.Y(y) - s(1), RGB(LabelKeys));
}

void DrawStrip(SDL_Renderer *ren, const Scale &s, double y, Uint count, char first, Uint selected)
{
	const double tileW = 19, tileH = 24, gap = 3;
	double totalW = count * tileW + (count - 1) * gap;
	TTF_Font *regular = GetUIFont(s(13), false), *bold = GetUIFont(s(13), true);
	for (Uint i = 0; i < count; i++)
	{
		double mx = 360 - totalW / 2 + i * (tileW + gap);
		SDL_Rect r = { s.X(mx), s.Y(y), s(tileW), s(tileH) };
		char text[2] = { (char)(first + i), 0 };
		int textH = TTF_FontHeight(regular);
		if (i == selected)
		{
			DrawRoundedGlow(ren, r, s(4), s(8), RGB(AccentGlow, 150));
			FillRoundedRect(ren, r, s(4), RGB(Accent));
			DrawUIText(ren, bold, text, r.x + r.w / 2, r.y + (r.h - textH) / 2, RGB(Background), Align_Center);
		}
		else
			DrawUIText(ren, regular, text, r.x + r.w / 2, r.y + (r.h - textH) / 2, RGB(StripText), Align_Center);
	}
}

void DrawChip(SDL_Renderer *ren, const Scale &s, int x, int y, const char *text, bool on)
{
	TTF_Font *font = GetUIFont(s(11.5), false);
	int w = MeasureUIText(font, text) + s(18), h = s(19);
	SDL_Rect r = { x, y, w, h };
	FillRoundedRect(ren, r, h / 2, RGB(on ? ChipOnFill : ChipOff));
	DrawUIText(ren, font, text, x + w / 2, y + (h - TTF_FontHeight(font)) / 2, RGB(on ? ChipOnText : Label), Align_Center);
}

int ChipWidth(const Scale &s, const char *text) { return MeasureUIText(GetUIFont(s(11.5), false), text) + s(18); }

void DrawLevelCard(SDL_Renderer *ren, const Scale &s, const StartSelection &sel)
{
	SkillLevelInfo info = DescribeSkillLevel(sel.letter, sel.number);

	SDL_Rect card = { s.X(360 - 281), s.Y(328), s(562), s(70) };
	FillRoundedRect(ren, card, s(8), RGB(CardBorder));
	SDL_Rect inner = { card.x + 1, card.y + 1, card.w - 2, card.h - 2 };
	FillRoundedRect(ren, inner, s(8) - 1, RGB(CardFill));

	// "Level M4 · 40 snipes · 4 portals · 5 lives", centered as one line
	char level[16], details[96];
	snprintf(level, sizeof(level), "Level %c%u", (char)('A' + sel.letter), sel.number);
	snprintf(details, sizeof(details), " · %u snipes · %u portals · %u lives", info.snipes, info.portals, info.lives);
	TTF_Font *bold = GetUIFont(s(16), true), *regular = GetUIFont(s(14), false);
	int levelW = MeasureUIText(bold, level), detailsW = MeasureUIText(regular, details);
	int x = s.X(360) - (levelW + detailsW) / 2;
	int baselineY = s.Y(342);
	DrawUIText(ren, bold, level, x, baselineY, RGB(White));
	DrawUIText(ren, regular, details, x + levelW, baselineY + TTF_FontAscent(bold) - TTF_FontAscent(regular), RGB(0xcfd6e6));

	// Hazard tags, lit when that hazard is on at this letter
	const char *names[4] = { "Small snipes", "Bouncing bullets", "Electric walls", "Armored portals" };
	bool on[4] = { info.smallSnipes, info.bouncingBullets, info.electricWalls, info.armoredPortals };
	int gap = s(6), total = 0;
	for (int i = 0; i < 4; i++)
		total += ChipWidth(s, names[i]) + (i < 3 ? gap : 0);
	int cx = s.X(360) - total / 2;
	for (int i = 0; i < 4; i++)
	{
		DrawChip(ren, s, cx, s.Y(369), names[i], on[i]);
		cx += ChipWidth(s, names[i]) + gap;
	}
}

// A keycap: rounded box with a heavier bottom edge.
int KeyCapWidth(const Scale &s, const char *caption)
{
	TTF_Font *font = GetUIFont(s(11.5), false);
	return MeasureUIText(font, caption) + s(10);
}
int DrawKeyCap(SDL_Renderer *ren, const Scale &s, int x, int y, const char *caption)
{
	TTF_Font *font = GetUIFont(s(11.5), false);
	int w = KeyCapWidth(s, caption), h = s(18);
	SDL_Rect outer = { x, y, w, h };
	FillRoundedRect(ren, outer, s(4), RGB(KeyBorder));
	int bottom = s(2) > 1 ? s(2) : 2;
	SDL_Rect inner = { x + 1, y + 1, w - 2, h - 1 - bottom };
	FillRoundedRect(ren, inner, s(4) - 1, RGB(KeyFill));
	DrawUIText(ren, font, caption, x + w / 2, y + (h - bottom - TTF_FontHeight(font)) / 2 + 1, RGB(KeyCapText), Align_Center);
	return w;
}

struct ControlHint { const char *keys[4]; const char *action; };

void DrawControls(SDL_Renderer *ren, const Scale &s, double y)
{
	static const ControlHint hints[] =
	{
		{ { "←", "↑", "↓", "→" }, "move"    },
		{ { "W", "A", "S", "D" }, "shoot"   },
		{ { "Space", 0, 0, 0 },   "run"     },
		{ { "Esc", 0, 0, 0 },     "give up" },
	};
	TTF_Font *font = GetUIFont(s(12.5), false);
	int capGap = s(2), actionGap = s(6), groupGap = s(22);

	int total = 0;
	for (size_t g = 0; g < SDL_arraysize(hints); g++)
	{
		for (int k = 0; k < 4 && hints[g].keys[k]; k++)
			total += KeyCapWidth(s, hints[g].keys[k]) + capGap;
		total += actionGap + MeasureUIText(font, hints[g].action) + (g + 1 < SDL_arraysize(hints) ? groupGap : 0);
	}
	int x = s.X(360) - total / 2, top = s.Y(y);
	for (size_t g = 0; g < SDL_arraysize(hints); g++)
	{
		for (int k = 0; k < 4 && hints[g].keys[k]; k++)
			x += DrawKeyCap(ren, s, x, top, hints[g].keys[k]) + capGap;
		x += actionGap;
		x += DrawUIText(ren, font, hints[g].action, x, top + (s(18) - TTF_FontHeight(font)) / 2, RGB(KeyText));
		x += groupGap;
	}
}

void DrawPressReturn(SDL_Renderer *ren, const Scale &s, double y)
{
	TTF_Font *font = GetUIFont(s(14), false);
	const char *before = "Press", *after = "to start";
	int space = s(6);
	int w = MeasureUIText(font, before) + space + KeyCapWidth(s, "Return") + space + MeasureUIText(font, after);
	int x = s.X(360) - w / 2, top = s.Y(y), textY = top + (s(18) - TTF_FontHeight(font)) / 2;
	x += DrawUIText(ren, font, before, x, textY, RGB(White)) + space;
	x += DrawKeyCap(ren, s, x, top, "Return") + space;
	DrawUIText(ren, font, after, x, textY, RGB(White));
}

} // namespace

static void RenderStartScreen(SDL_Renderer *ren, int w, int h, const StartSelection &sel)
{
	Scale s;
	s.u = SDL_min(w / 720.0, h / 600.0);
	s.originX = (w - s(720)) / 2;
	s.originY = (h - s(600)) / 2;

	SDL_Color bg = RGB(Background);
	SDL_SetRenderDrawColor(ren, bg.r, bg.g, bg.b, 0xFF);
	SDL_RenderClear(ren);

	DrawArt(ren, s);
	DrawStripLabel(ren, s, 216, "ENEMY SKILL", "←→");
	DrawStrip(ren, s, 235, 26, 'A', sel.letter);
	DrawStripLabel(ren, s, 269, "ENEMY NUMBERS", "↑↓");
	DrawStrip(ren, s, 288, 9, '1', sel.number - 1);
	DrawLevelCard(ren, s, sel);
	DrawControls(ren, s, 416);
	DrawPressReturn(ren, s, 448);

	// Credits, anchored to the bottom of the window
	TTF_Font *credits = GetUIFont(s(11), false);
	int lineH = s(17.6), bottom = h - s(18);
	DrawUIText(ren, credits, "© 1982 SuperSet Software Corp. All rights reserved.", w / 2, bottom - 2 * lineH, RGB(Credits), Align_Center);
	DrawUIText(ren, credits, "Ported by David Ellsworth · macOS port by mrBallistic", w / 2, bottom - lineH, RGB(Credits), Align_Center);
}

// The screen only changes when the selection or the window size does, and
// drawing it (hundreds of glyphs for the glowing art) is far too costly to
// repeat every frame. So draw it into a texture once and reuse that.
static SDL_Texture *Cache;
static StartScreenFrame CachedFrame;

void DrawStartScreen(SDL_Renderer *ren, int w, int h, const StartSelection &sel)
{
	StartScreenFrame frame = { w, h, sel };
	if (!SDL_RenderTargetSupported(ren))
	{
		RenderStartScreen(ren, w, h, sel);
		return;
	}
	if (Cache && (CachedFrame.width != w || CachedFrame.height != h))
		CloseStartScreen();
	if (!Cache)
	{
		Cache = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
		if (!Cache)
		{
			RenderStartScreen(ren, w, h, sel);
			return;
		}
	}
	else
	if (SameStartScreenFrame(frame, CachedFrame))
	{
		SDL_RenderCopy(ren, Cache, NULL, NULL);
		return;
	}
	SDL_SetRenderTarget(ren, Cache);
	RenderStartScreen(ren, w, h, sel);
	SDL_SetRenderTarget(ren, NULL);
	CachedFrame = frame;
	SDL_RenderCopy(ren, Cache, NULL, NULL);
}

void CloseStartScreen()
{
	if (Cache)
		SDL_DestroyTexture(Cache);
	Cache = NULL;
}
