#include "helpers.h"
Rectangle getTextureAtlas(int x, int y, int cellSizePixelsX, int cellSizePixelsY)
{
	return Rectangle{(float)x * cellSizePixelsX, (float)y * cellSizePixelsY,
					 (float)cellSizePixelsX, (float)cellSizePixelsY};
}
void drawPromptMid(std::string text, int fontSize, Color color)
{
	int textWidth = MeasureText(text.c_str(), fontSize);
	int posX = (GetScreenWidth() - textWidth) / 2;
	int posY = GetScreenHeight() / 2;
	DrawText(text.c_str(), posX, posY, fontSize, color);
}
