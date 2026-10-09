#include "gamemain.h"
#include <iostream>
#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>
#include "gameMain.h"
#include "AssetManager.h"
#include "gameMap.h"
#include "helpers.h"
#include <raymath.h> //Clamp
#include <worldGenerator.h>
// 用结构体储存静态变量 且方便重置游戏
struct GameData
{
	Color c1 = {255, 0, 200, 255};
	Color c2 = {72, 61, 139, 255};
	GameMap gameMap;
	Camera2D camera;
} gameData; // 别名
AssetManager assetManager;
bool initGame()
{
	// 生成世界
	assetManager.loadAll();
	generatorWorld(gameData.gameMap);

#pragma region camera
	gameData.camera.target = {0, 0};
	gameData.camera.rotation = 0.0f;
	gameData.camera.zoom = 100.0f; // 缩放比例
#pragma endregion
	return true;
}
bool updateGame()
{
	// deltaTime[增量时间]:避免帧率差别带来的速度影响
	float deltaTime = GetFrameTime();
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; }
	gameData.camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
#pragma region camera_movement
	static float CAMERA_SPEED = 10.f;
	if (IsKeyDown(KEY_A)) { gameData.camera.target.x -= CAMERA_SPEED * deltaTime; }
	if (IsKeyDown(KEY_D)) { gameData.camera.target.x += CAMERA_SPEED * deltaTime; }
	if (IsKeyDown(KEY_W)) { gameData.camera.target.y -= CAMERA_SPEED * deltaTime; }
	if (IsKeyDown(KEY_S)) { gameData.camera.target.y += CAMERA_SPEED * deltaTime; }
#pragma endregion
#pragma region test place & break
	Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), gameData.camera);
	int blockX = (int)floor(worldPos.x);
	int blockY = (int)floor(worldPos.y);
	// 破坏方块
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		auto b = gameData.gameMap.getBlockSafe(blockX, blockY);
		if (b) { *b = {}; }
	}
	// 放置方块
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		auto b = gameData.gameMap.getBlockSafe(blockX, blockY);
		if (b) { b->type = Block::ice; }
	}
#pragma endregion
#pragma region draw

	ClearBackground(gameData.c2);
	BeginMode2D(gameData.camera);
	// 渲染优化:只渲染屏幕上的部分   GetScreenToWorld2D(将屏幕的坐标转换为世界的坐标)
	Vector2 topLeftView = GetScreenToWorld2D({0, 0}, gameData.camera);
	Vector2 bottomRightView = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, gameData.camera);
	int startXView = (int)floorf(topLeftView.x - 1);
	int endXView = (int)floorf(bottomRightView.x + 1);
	int startYView = (int)floorf(topLeftView.y - 1);
	int endYView = (int)floorf(bottomRightView.y + 1);
	// Clamp将数值限制在限定范围之内
	startXView = Clamp(startXView, 0, gameData.gameMap.w - 1);
	endXView = Clamp(endXView, 0, gameData.gameMap.w - 1);
	startYView = Clamp(startYView, 0, gameData.gameMap.h - 1);
	endYView = Clamp(endYView, 0, gameData.gameMap.h - 1);

	// 纹理 (纹理,排序方式{坐标,大小}[显示纹理的截取部分],矩形区域{坐标,大小},原点(旋转中心){0.0},旋转参数0,颜色主题)
	for (int y = startYView; y < endYView; y++)
	{
		for (int x = startXView; x < endXView; x++)
		{
			auto &b = gameData.gameMap.getBlockUnsafe(x, y);
			if (b.type != Block::air)
			{
				DrawTexturePro(
					assetManager.textures,				// texture纹理
					getTextureAtlas(b.type, 0, 32, 32), // source截取的矩形区域
					{(float)x, (float)y, 1, 1},			// dest显示在的矩形区域
					{0, 0},								// origin旋转中心
					0.0f,								// rotation旋转参数
					WHITE);								// tint颜色主题
			}
		}
	}
	// 选中方块提示
	DrawTexturePro(
		assetManager.frame,
		{0, 0, (float)assetManager.frame.width, (float)assetManager.frame.height},
		{(float)blockX, (float)blockY, 1, 1},
		{0, 0},
		0.0f,
		WHITE);
	EndMode2D();

#pragma region imgui
	// 第一个窗口
	// usually:同窗口部件不可以重名(但可以使用 名称+##num 实现 或者 PushID() + PopID())
	// 不同窗口的部件可以重名
	// 若窗口同名,则会合并为一个
	ImGui::Begin("test");
	ImGui::Text("Oh watchin'it burn,ain't got no hesition.");
	if (ImGui::Button("Elemental Skill"))
		std::cout << "Enigmatic Feint\n";
	ImGui::SameLine(); // 在同一行显示
	if (ImGui::Button("Elemental Burst"))
		std::cout << "Magic Trick: Astonishing Shift\n";
	ImGui::End();
	// 控制camera窗口
	ImGui::Begin("Game Control");
	ImGui::SliderFloat("Camera zoom", &gameData.camera.zoom, 5, 150); // 滑块
	ImGui::SliderFloat("Camera speed", &CAMERA_SPEED, 5, 60);
	ImGui::End();
#pragma endregion

#pragma endregion
	DrawFPS(10, 10);
	return true;
}
bool closeGame()
{
	return true;
}