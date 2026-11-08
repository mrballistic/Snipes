#include "layout.h"

void FitTilesToOutput(int outputWidth, int outputHeight, int columns, int rows, int &tileWidth, int &tileHeight)
{
	int contentWidth  = columns * tileWidth;
	int contentHeight = (tileHeight * 3 + 1) / 2 + rows * tileHeight; // status bar + playfield
	double  widthRatio = contentWidth  > 0 ? (double)outputWidth  / contentWidth  : 1;
	double heightRatio = contentHeight > 0 ? (double)outputHeight / contentHeight : 1;
	if ( widthRatio < 1)  widthRatio = 1. /  widthRatio;
	if (heightRatio < 1) heightRatio = 1. / heightRatio;

	if (heightRatio > widthRatio)
	{
		tileHeight = (int)(outputHeight / (rows + 1.5) + 0.5);
		tileWidth  = (tileHeight*3+2)/4;
	}
	else
	{
		tileWidth  = (outputWidth + columns/2) / columns;
		tileHeight = (tileWidth*8+3)/6;
	}
	if (tileWidth  < 1) tileWidth  = 1;
	if (tileHeight < 1) tileHeight = 1;
}
