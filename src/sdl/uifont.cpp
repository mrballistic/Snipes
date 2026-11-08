#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include "uifont.h"

namespace {

struct FontEntry { int size; int kind; TTF_Font *font; };
enum { Kind_Regular, Kind_Bold, Kind_Glyph };
std::vector<FontEntry> Fonts;

struct FontFile { const char *path; int index; };

// System fallbacks if the bundled font is missing. First entry that opens
// wins, so list the preferred font for each platform first.
const FontFile RegularFonts[] =
{
	{ "/System/Library/Fonts/HelveticaNeue.ttc", 0 },
	{ "/System/Library/Fonts/Helvetica.ttc", 0 },
	{ "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0 },
	{ "/usr/share/fonts/TTF/DejaVuSans.ttf", 0 },
	{ "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf", 0 },
	{ "C:\\Windows\\Fonts\\segoeui.ttf", 0 },
};
const FontFile BoldFonts[] =
{
	{ "/System/Library/Fonts/HelveticaNeue.ttc", 1 },
	{ "/System/Library/Fonts/Helvetica.ttc", 1 },
	{ "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 0 },
	{ "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf", 0 },
	{ "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans-Bold.ttf", 0 },
	{ "C:\\Windows\\Fonts\\segoeuib.ttf", 0 },
};

TTF_Font *OpenFirst(const FontFile *files, size_t count, int size)
{
	for (size_t i = 0; i < count; i++)
		if (TTF_Font *f = TTF_OpenFontIndex(files[i].path, size, files[i].index))
			return f;
	return NULL;
}

// IBM Plex Mono ships in fonts/ (and in the app bundle's Resources/fonts/).
TTF_Font *OpenBundled(const char *file, int size)
{
	char path[1200];
	snprintf(path, sizeof(path), "%sfonts/%s", ResourceDirectory(), file);
	return TTF_OpenFont(path, size);
}

TTF_Font *GetFont(int size, int kind)
{
	if (size < 1)
		size = 1;
	for (size_t i = 0; i < Fonts.size(); i++)
		if (Fonts[i].size == size && Fonts[i].kind == kind)
			return Fonts[i].font;

	TTF_Font *font = NULL;
	if (kind == Kind_Glyph)
		font = OpenGameFont(size);
	else
	{
		font = OpenBundled(kind == Kind_Bold ? "IBMPlexMono-Bold.ttf" : "IBMPlexMono-Regular.ttf", size);
		if (!font && kind == Kind_Bold)
			font = OpenFirst(BoldFonts, SDL_arraysize(BoldFonts), size);
		if (!font)
		{
			font = OpenFirst(RegularFonts, SDL_arraysize(RegularFonts), size);
			if (font && kind == Kind_Bold) // no bold face found: let SDL_ttf embolden the regular one
				TTF_SetFontStyle(font, TTF_STYLE_BOLD);
		}
	}
	if (!font) // last resort: the game font, so text still appears
		font = OpenGameFont(size);
	if (font)
		TTF_SetFontHinting(font, TTF_HINTING_LIGHT);

	FontEntry e = { size, kind, font };
	Fonts.push_back(e);
	return font;
}

// Rendered text, keyed by font and string. Text is rendered white and tinted
// at draw time, so one texture serves every color.
std::map<std::pair<TTF_Font*, std::string>, SDL_Texture*> TextCache;

SDL_Texture *GetTextTexture(SDL_Renderer *ren, TTF_Font *font, const std::string &text)
{
	std::pair<TTF_Font*, std::string> key(font, text);
	auto it = TextCache.find(key);
	if (it != TextCache.end())
		return it->second;
	if (TextCache.size() > 2048) // e.g. after many resizes; just start over
	{
		for (auto &entry : TextCache)
			SDL_DestroyTexture(entry.second);
		TextCache.clear();
	}
	SDL_Color white = { 0xFF, 0xFF, 0xFF, 0xFF };
	SDL_Texture *t = NULL;
	if (SDL_Surface *s = TTF_RenderUTF8_Blended(font, text.c_str(), white))
	{
		t = SDL_CreateTextureFromSurface(ren, s);
		SDL_FreeSurface(s);
	}
	TextCache[key] = t;
	return t;
}

// Splits UTF-8 into one string per code point (for letter-spaced text).
std::vector<std::string> SplitCodePoints(const char *utf8)
{
	std::vector<std::string> out;
	for (const char *p = utf8; *p; )
	{
		const char *start = p++;
		while ((*p & 0xC0) == 0x80)
			p++;
		out.push_back(std::string(start, p));
	}
	return out;
}

} // namespace

TTF_Font *GetUIFont(int pixelSize, bool bold) { return GetFont(pixelSize, bold ? Kind_Bold : Kind_Regular); }
TTF_Font *GetGlyphFont(int pixelSize)         { return GetFont(pixelSize, Kind_Glyph); }

void CloseUIFonts()
{
	for (auto &entry : TextCache)
		SDL_DestroyTexture(entry.second);
	TextCache.clear();
	for (size_t i = 0; i < Fonts.size(); i++)
		if (Fonts[i].font)
			TTF_CloseFont(Fonts[i].font);
	Fonts.clear();
}

int MeasureUIText(TTF_Font *font, const char *utf8, int tracking)
{
	if (!font || !*utf8)
		return 0;
	if (!tracking)
	{
		int w = 0;
		TTF_SizeUTF8(font, utf8, &w, NULL);
		return w;
	}
	std::vector<std::string> chars = SplitCodePoints(utf8);
	int total = 0;
	for (size_t i = 0; i < chars.size(); i++)
	{
		int w = 0;
		TTF_SizeUTF8(font, chars[i].c_str(), &w, NULL);
		total += w + (i + 1 < chars.size() ? tracking : 0);
	}
	return total;
}

int DrawUIText(SDL_Renderer *ren, TTF_Font *font, const char *utf8, int x, int y,
               SDL_Color color, TextAlign align, int tracking)
{
	if (!font || !*utf8)
		return 0;
	int width = MeasureUIText(font, utf8, tracking);
	if (align == Align_Center) x -= width / 2;
	if (align == Align_Right ) x -= width;

	std::vector<std::string> pieces;
	if (tracking)
		pieces = SplitCodePoints(utf8);
	else
		pieces.push_back(utf8);

	for (size_t i = 0; i < pieces.size(); i++)
	{
		SDL_Texture *t = GetTextTexture(ren, font, pieces[i]);
		if (!t)
			continue;
		int w, h;
		SDL_QueryTexture(t, NULL, NULL, &w, &h);
		SDL_SetTextureColorMod(t, color.r, color.g, color.b);
		SDL_SetTextureAlphaMod(t, color.a);
		SDL_Rect dst = { x, y, w, h };
		SDL_RenderCopy(ren, t, NULL, &dst);
		x += w + tracking;
	}
	return width;
}

void FillRoundedRect(SDL_Renderer *ren, SDL_Rect r, int radius, SDL_Color color)
{
	if (radius * 2 > r.w) radius = r.w / 2;
	if (radius * 2 > r.h) radius = r.h / 2;
	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
	SDL_Rect middle = { r.x, r.y + radius, r.w, r.h - 2 * radius };
	SDL_RenderFillRect(ren, &middle);
	// Top and bottom bands, one scanline at a time, inset to follow the corner curve
	for (int i = 0; i < radius; i++)
	{
		double dy = radius - i - 0.5;
		int inset = (int)(radius - SDL_sqrt((double)radius * radius - dy * dy) + 0.5);
		SDL_Rect top    = { r.x + inset, r.y + i,           r.w - 2 * inset, 1 };
		SDL_Rect bottom = { r.x + inset, r.y + r.h - 1 - i, r.w - 2 * inset, 1 };
		SDL_RenderFillRect(ren, &top);
		SDL_RenderFillRect(ren, &bottom);
	}
}

void DrawRoundedGlow(SDL_Renderer *ren, SDL_Rect r, int radius, int spread, SDL_Color color)
{
	for (int i = spread; i > 0; i--)
	{
		SDL_Color c = color;
		c.a = (Uint8)(color.a * (spread - i + 1) / (spread + 1) / spread);
		SDL_Rect g = { r.x - i, r.y - i, r.w + 2 * i, r.h + 2 * i };
		FillRoundedRect(ren, g, radius + i, c);
	}
}
