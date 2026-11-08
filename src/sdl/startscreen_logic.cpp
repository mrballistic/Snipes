#include <ctype.h>
#include "startscreen_logic.h"

static const Uint LastLetter = 'Z'-'A';

StartAction StartScreenKey(StartSelection &sel, StartKey key, char chr, StartModifier mod)
{
	if (mod != StartModifier_None)
		return mod == StartModifier_Control && key == StartKey_Char && (chr == 'c' || chr == 'C') ? StartAction_Quit : StartAction_None;
	switch (key)
	{
	case StartKey_Left  : if (sel.letter > 0         ) sel.letter--; break;
	case StartKey_Right : if (sel.letter < LastLetter) sel.letter++; break;
	case StartKey_Down  : if (sel.number > 1         ) sel.number--; break;
	case StartKey_Up    : if (sel.number < 9         ) sel.number++; break;
	case StartKey_Return: return StartAction_Start;
	case StartKey_Escape: return StartAction_Quit;
	case StartKey_Char:
		if (chr >= 'a' && chr <= 'z') sel.letter = chr - 'a';
		else
		if (chr >= 'A' && chr <= 'Z') sel.letter = chr - 'A';
		else
		if (chr >= '1' && chr <= '9') sel.number = chr - '0';
		break;
	}
	return StartAction_None;
}

bool ParseSavedLevel(const char *text, StartSelection &sel)
{
	if (!text)
		return false;
	char letter = (char)toupper((unsigned char)text[0]);
	if (letter < 'A' || letter > 'Z' || text[1] < '1' || text[1] > '9')
		return false;
	for (const char *p = text + 2; *p; p++)
		if (!isspace((unsigned char)*p))
			return false;
	sel.letter = letter - 'A';
	sel.number = text[1] - '0';
	return true;
}

void FormatSavedLevel(const StartSelection &sel, char out[4])
{
	out[0] = (char)('A' + sel.letter);
	out[1] = (char)('0' + sel.number);
	out[2] = 0;
	out[3] = 0;
}
