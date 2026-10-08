#include "gamemain.h"
#include <iostream>
#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>
#include "gameMain.h"
#include "AssetManager.h"
#include "gameMap.h"
// 用结构体储存静态变量 且方便重置游戏
struct GameData
{
	GameMap gameMap;
} gameData; // 别名
AssetManager assetManager;
bool initGame()
{
	assetManager.loadAll();

	gameData.gameMap.create(30, 10);
	gameData.gameMap.getBlockUnsafe(0, 0).type = Block::dirt; // dirt是Block里的一个枚举常量
	gameData.gameMap.getBlockUnsafe(1, 1).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(2, 2).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(3, 3).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(4, 4).type = Block::dirt;

	return true;
}
bool updateGame()
{
	// deltaTime[增量时间]:避免帧率差别带来的速度影响
	float deltaTime = GetFrameTime();
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; }

#pragma region draw
	Color c = {255, 0, 200, 255};
	ClearBackground(BLACK);
	// 纹理 (纹理,排序方式{坐标,大小}[显示纹理的截取部分],矩形区域{坐标,大小},原点(旋转中心){0.0},旋转参数0,颜色主题)
	for (int y = 0; y < gameData.gameMap.h; y++)
	{
		for (int x = 0; x < gameData.gameMap.w; x++)
		{
			auto &b = gameData.gameMap.getBlockUnsafe(x, y);
			if (b.type != Block::air)
			{
				float size = 32;
				float posX = x * size;
				float posY = y * size;
				DrawTexturePro(assetManager.dirt,
							   Rectangle{0.f, 0.f, (float)assetManager.dirt.width, (float)assetManager.dirt.height},
							   {posX, posY, size, size},
							   {0, 0},
							   0.0f,
							   WHITE);
			}
		}
	}
	return true;
#pragma endregion
}
bool closeGame()
{
	return true;
}