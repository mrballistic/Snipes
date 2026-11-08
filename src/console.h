#pragma once

#include "types.h"
#include "macros.h"

void WriteTextMem(Uint count, WORD row, WORD column, MazeTile *src);

void outputText(BYTE color, WORD count, WORD row, WORD column, const char *src);
void outputNumber(BYTE color, bool zeroPadding, WORD count, WORD row, WORD column, Uint number);
void EraseBottomTwoLines();

DWORD ReadTextFromConsole(char buffer[], DWORD bufsize);

void SetConsoleOutputTextColor(WORD wAttributes);

void WriteTextToConsole(char const *text, size_t length);
template <size_t LENGTH> void WriteTextToConsole(char const (&text)[LENGTH]) { WriteTextToConsole(text, strlength(text)); }

void OpenDirectConsole();
void CloseDirectConsole(Uint lineNum);

void ClearConsole();

int OpenConsole();

// SDL build: shows the graphical start screen, starting from the given level,
// and blocks until the player starts a game (returning true with the chosen
// level) or quits (returning false).
bool ChooseSkillLevel(Uint &letter, Uint &number);
void CloseConsole();
