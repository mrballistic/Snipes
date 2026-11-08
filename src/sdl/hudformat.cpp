#include <stdio.h>
#include "hudformat.h"

void FormatGameTime(WORD frame, char out[8])
{
	// The game advances one frame per PC timer tick, 1193182/65536 Hz
	Uint seconds = (Uint)((QWORD)frame * 65536 / 1193182);
	snprintf(out, 8, "%u:%02u", seconds / 60, seconds % 60);
}
