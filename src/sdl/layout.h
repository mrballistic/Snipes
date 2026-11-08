#pragma once

// Chooses the tile size, in pixels, that best fits a renderer output of
// outputWidth×outputHeight pixels showing `columns` columns and, under the
// 1.5-row status bar, `rows` rows. tileWidth/tileHeight hold the current size
// on entry; whichever dimension changed more decides the new size, and tiles
// keep their 3:4 shape.
void FitTilesToOutput(int outputWidth, int outputHeight, int columns, int rows, int &tileWidth, int &tileHeight);
