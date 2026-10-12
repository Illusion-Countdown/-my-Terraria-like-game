#pragma once
#include <raylib.h>
#include <rlImGui.h>
#include <FastNoiseSIMD.h>
#include <imgui.h>
#include <string>
Rectangle getTextureAtlas(int x, int y, int cellSizePixelsX, int cellSizePixelsY); // pixel像素,像素点
void drawPromptMid(std::string text, int fontSize = 30, Color color = GREEN);