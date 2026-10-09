#include <iostream>
#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>
#include <gameMain.h>
#include "asserts.h"
int main()
{
// 正式版本 禁用控制台输出
#if PRODUCTION_BUILD == 1
	SetTraceLogLevel(LOG_NONE);
#endif
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1600, 900, "如影流露的冷刃");
	SetExitKey(KEY_NULL); // 禁止Esc关闭窗口
	SetTargetFPS(240);	  // 限制帧率

	// clang-format off
#pragma region imgui
	// clang-format on
	rlImGuiSetup(true);
	ImGuiIO &io = ImGui::GetIO();
	io.FontGlobalScale = 2;
	// 停靠
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// clang-format off
#pragma endregion
#pragma region window
	// clang-format on

	if (!initGame())
		return 0;
	while (!WindowShouldClose())
	{
		BeginDrawing();

		// clang-format off
	#pragma region rlimgui
		// clang-format on
		// 窗口颜色恢复
		rlImGuiBegin();
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::PopStyleColor(2);
		// clang-format off
	#pragma endregion
		// clang-format on

		if (!updateGame())
			CloseWindow();
		rlImGuiEnd();
		EndDrawing();
	}
	rlImGuiShutdown();

	CloseWindow();
	// clang-format off
#pragma endregion
	// clang-format on

	return 0;
}
