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
#include <algorithm> //std::clamp
#include "structure.h"
#include "saveMap.h"
#include <string>
// 游戏数据 : 用结构体储存静态变量 且方便重置游戏
struct GameData
{
	Color c1 = {255, 0, 200, 255};
	Color c2 = {72, 61, 139, 255};

	GameMap gameMap;

	Camera2D camera;

	int creativeSelectedBlock = Block::dirt;

	Vector2 selectionStart = {};
	Vector2 selectionEnd = {};
	Structure copySturcture;

	char saveName[100] = {};
} gameData; // 别名
#pragma region 游戏系统设置
AssetManager assetManager;
bool showImgui = false;

std::string hugMessage = "";
float messageTime = 0.0f;	 // 剩余显示时长(秒),倒计时
float messageElapsed = 0.0f; // 已显示时长(秒),用于计算淡入进度
Color messageColor = GREEN;
int messageSize = 30;
float messageAlpha = 0.75f;	 // 提示词常态不透明度(0~1),1.0为不透明
float messageFadeIn = 0.3f;	 // 淡入时长(秒),出现时从全透明渐亮
float messageFadeOut = 0.5f; // 淡出时长(秒),消失前渐隐

// 便捷接口:showMsg("文字") / showMsg("文字", RED) / showMsg("文字", RED, 3.f)
// 调用处只给基础颜色即可
void showMsg(const std::string &text, Color color = GREEN, float duration = 2.f)
{
	hugMessage = text;
	messageColor = color;
	messageTime = duration;
	messageElapsed = 0.f;
}
#pragma endregion
bool initGame()
{
	// 生成世界
	assetManager.loadAll();
	generatorWorld(gameData.gameMap);

#pragma region camera
	gameData.camera.target = {36, 60};
	gameData.camera.rotation = 0.0f;
	gameData.camera.zoom = 80.0f; // 缩放比例
#pragma endregion
	return true;
}

bool updateGame()
{
	// deltaTime[增量时间]:避免帧率差别带来的速度影响
	float deltaTime = GetFrameTime();
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; }
	gameData.camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
#pragma region 相机移动功能
	static float CAMERA_SPEED = 10.f;
	if (IsKeyDown(KEY_A)) { gameData.camera.target.x -= CAMERA_SPEED * deltaTime; }
	if (IsKeyDown(KEY_D)) { gameData.camera.target.x += CAMERA_SPEED * deltaTime; }
	if (IsKeyDown(KEY_W)) { gameData.camera.target.y -= CAMERA_SPEED * deltaTime; }
	if (IsKeyDown(KEY_S)) { gameData.camera.target.y += CAMERA_SPEED * deltaTime; }
#pragma endregion
#pragma region 修改世界
	Vector2 mouseScreenPos = GetMousePosition();
	Vector2 worldPos = GetScreenToWorld2D(mouseScreenPos, gameData.camera);
	int blockX = (int)floor(worldPos.x);
	int blockY = (int)floor(worldPos.y);

	// 确保选择的方块存在
	if (gameData.creativeSelectedBlock < 0) { gameData.creativeSelectedBlock = 0; }
	if (gameData.creativeSelectedBlock >= Block::BLOCKS_COUNT) { gameData.creativeSelectedBlock = Block::BLOCKS_COUNT - 1; }

	// 复制粘贴structure功能
	if (showImgui)
	{
		// 鼠标操作
		if (IsKeyPressed(KEY_ONE)) { gameData.selectionStart = Vector2{(float)blockX, (float)blockY}; }
		if (IsKeyPressed(KEY_TWO)) { gameData.selectionEnd = Vector2{(float)blockX, (float)blockY}; }
		if (IsKeyPressed(KEY_THREE)) { gameData.copySturcture.copyFromMap(gameData.gameMap, gameData.selectionStart, gameData.selectionEnd); }
		if (IsKeyPressed(KEY_FOUR)) { gameData.copySturcture.pasteIntoMap(gameData.gameMap, worldPos); }

		if (gameData.selectionStart.x > gameData.selectionEnd.x)
			std::swap(gameData.selectionStart.x, gameData.selectionEnd.x);
		if (gameData.selectionStart.y > gameData.selectionEnd.y)
			std::swap(gameData.selectionStart.y, gameData.selectionEnd.y);
	}

	// 破坏和放置方块功能
	if (!showImgui) // 仅未显示Imgui时接受输入
	{
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
			if (b) { b->type = gameData.creativeSelectedBlock; }
		}
	}

#pragma endregion
#pragma region 绘制图形

	ClearBackground(gameData.c2);

	if (IsKeyPressed(KEY_TAB)) { showImgui = !showImgui; }

	BeginMode2D(gameData.camera);

	// 渲染优化:只渲染屏幕上的部分   GetScreenToWorld2D(将屏幕的坐标转换为世界的坐标)
	Vector2 topLeftView = GetScreenToWorld2D({0, 0}, gameData.camera);
	Vector2 bottomRightView = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, gameData.camera);
	int startXView = (int)floorf(topLeftView.x - 1);
	int endXView = (int)floorf(bottomRightView.x + 1);
	int startYView = (int)floorf(topLeftView.y - 1);
	int endYView = (int)floorf(bottomRightView.y + 1);
	// clamp将数值限制在限定范围之内
	startXView = std::clamp(startXView, 0, gameData.gameMap.w - 1);
	endXView = std::clamp(endXView, 0, gameData.gameMap.w - 1);
	startYView = std::clamp(startYView, 0, gameData.gameMap.h - 1);
	endYView = std::clamp(endYView, 0, gameData.gameMap.h - 1);

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

	// 复制粘贴功能:选中区域提示
	if (showImgui)
	{
		Rectangle rect;
		rect.x = gameData.selectionStart.x;
		rect.y = gameData.selectionStart.y;
		rect.width = gameData.selectionEnd.x - gameData.selectionStart.x;
		rect.height = gameData.selectionEnd.y - gameData.selectionStart.y;
		// width,height++才能圈中所有方块
		rect.width++;
		rect.height++;
		// 选区描边
		DrawRectangleLinesEx(rect, 0.1, {20, 101, 250, 145});
	}

	EndMode2D();

#pragma endregion

#pragma region ImGui窗口
	// usually:同窗口部件不可以重名(但可以使用 名称+##num 实现 或者 PushID() + PopID())
	// 不同窗口的部件可以重名
	// 若窗口同名,则会合并为一个
	if (showImgui)
	{
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

		// 复制粘贴功能
		if (ImGui::Button("Copy: 3"))
			gameData.copySturcture.copyFromMap(gameData.gameMap, gameData.selectionStart, gameData.selectionEnd);
		ImGui::SameLine();
		ImGui::Text(" Paste: 4 ,Start: 1,End: 2");

		ImGui::InputText("File Name", gameData.saveName, sizeof(gameData.saveName));

		// 路径必须基于RESOURCES_PATH:相对路径会跟着进程的工作目录(exe所在目录)跑,而不是resources/
		std::string structureFileName = "structures/";
		structureFileName += gameData.saveName;
		structureFileName += ".bin";
		std::string structurePath = RESOURCES_PATH + structureFileName;
		std::string structureRelativePath = "resources/" + structureFileName;
		// 保存资源
		if (ImGui::Button("Save"))
		{
			if (gameData.saveName[0] == 0)
				showMsg("Save failed: file name is empty", RED);
			else if (saveBlockDataToFile(gameData.copySturcture.mapData, gameData.copySturcture.w, gameData.copySturcture.h, structurePath))
				showMsg("Structure saved to " + structureRelativePath);
			else
				showMsg("Save failed: " + structureRelativePath + "\ndid you copy a structure first?", RED);
		}
		ImGui::SameLine();
		// 导入资源
		if (ImGui::Button("Load"))
		{
			if (loadBlockDataFromFile(gameData.copySturcture.mapData, gameData.copySturcture.w, gameData.copySturcture.h, structurePath))
				showMsg("Structure loaded from " + structureRelativePath);
			else
				showMsg("Load failed: " + structureRelativePath, RED);
		}

		ImGui::Separator();

		for (int i = 0; i < Block::BLOCKS_COUNT; i++)
		{
			auto atlas = getTextureAtlas(i, 0, 32, 32);
			atlas.x /= assetManager.textures.width;
			atlas.width /= assetManager.textures.width;
			atlas.y /= assetManager.textures.height;
			atlas.height /= assetManager.textures.height;

			ImGui::PushID(i);

			ImTextureID tex = (ImTextureID)(intptr_t)assetManager.textures.id;
			if (ImGui::ImageButton(tex,
								   {35, 35},
								   {atlas.x, atlas.y},
								   {atlas.x + atlas.width, atlas.y + atlas.height}))
				gameData.creativeSelectedBlock = i;

			ImGui::PopID();

			if (i % 10 != 0)
				ImGui::SameLine(); // 保证每10个图标一行,同时air单独一行
		}

		ImGui::End();
	}

#pragma endregion

#pragma region 中央提示词显示 [ImGui窗口]
	// 正中心提示词显示(HUD):通过ImGui前景绘制列表输出,保证显示在所有ImGui窗口之上
	if (messageTime > 0)
	{
		messageTime -= deltaTime;
		messageElapsed += deltaTime;
		// alpha = 常态透明度 × min(淡入进度, 淡出进度);绘制时统一套用,不修改 messageColor
		float fadeIn = std::clamp(messageElapsed / messageFadeIn, 0.f, 1.f);
		float fadeOut = std::clamp(messageTime / messageFadeOut, 0.f, 1.f);
		float alpha = messageAlpha * std::min(fadeIn, fadeOut);

		ImFont *font = ImGui::GetFont();
		float fontSize = (float)messageSize;
		ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, hugMessage.c_str());
		ImVec2 textPos = {(ImGui::GetIO().DisplaySize.x - textSize.x) * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f};
		ImU32 textColor = IM_COL32(messageColor.r, messageColor.g, messageColor.b, (int)(alpha * 255.f));
		ImGui::GetForegroundDrawList()->AddText(font, fontSize, textPos, textColor, hugMessage.c_str());
	}
	else
	{
		hugMessage = "";
		messageTime = 0;
		messageElapsed = 0;
		messageSize = 30;
		messageColor = GREEN;
	}
#pragma endregion

	DrawFPS(10, 10);
	DrawText(TextFormat("Mouse Screen: (%.1f, %.1f)", mouseScreenPos.x, mouseScreenPos.y), 100, 10, 20, gameData.c1);
	DrawText(TextFormat("Mouse World: (%.1f, %.1f)", worldPos.x, worldPos.y), 100, 40, 20, RED);

	return true;
}

bool closeGame()
{
	return true;
}