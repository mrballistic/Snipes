#pragma once

#include "../types.h"

// The level highlighted on the start screen.
struct StartSelection { Uint letter; Uint number; }; // letter 0-25 (A-Z), number 1-9

enum StartKey { StartKey_Left, StartKey_Right, StartKey_Up, StartKey_Down, StartKey_Return, StartKey_Escape, StartKey_Char };
enum StartAction { StartAction_None, StartAction_Start, StartAction_Quit };
enum StartModifier { StartModifier_None, StartModifier_Control, StartModifier_Command }; // Command = Cmd/Win/Alt

// Applies one key press to the selection. Left/Right change the letter and
// Up/Down the number, stopping at the ends; StartKey_Char with a letter or a
// digit 1-9 jumps straight to it. Return starts the game, Escape quits.
// With a modifier held the key is a shortcut, not a choice: it is ignored,
// except Ctrl+C, which quits.
StartAction StartScreenKey(StartSelection &sel, StartKey key, char chr = 0, StartModifier mod = StartModifier_None);

// The last level played is saved as two characters, e.g. "M4".
bool ParseSavedLevel(const char *text, StartSelection &sel); // leaves sel unchanged on failure
void FormatSavedLevel(const StartSelection &sel, char out[4]);
