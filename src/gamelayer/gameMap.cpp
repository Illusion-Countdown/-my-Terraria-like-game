#include "gameMap.h"
#include <asserts.h>
void GameMap::create(int w, int h)
{
	*this = {}; // 重置结构体
	mapData.resize(w * h);

	this->w = w;
	this->h = h;

	for (auto &e : mapData)
	{
		e = {};
	} // 清空所有方块
}

Block &GameMap::getBlockUnsafe(int x, int y)
{
	// 若没有调用create()函数,则会报错
	permaAssertCommentDevelopement(mapData.size() == w * h, "Map dat not initialized");
	// 仅限开发阶段,若超出地图边界依旧会报错
	permaAssertCommentDevelopement(x >= 0 && y >= 0 && x < w && y < h, "getBlockUnsafe out of bounds error");

	return mapData[x + y * w];
}
Block *GameMap::getBlockSafe(int x, int y)
{
	permaAssertCommentDevelopement(mapData.size() == w * h, "Map dat not initialized");
	// 超出地图边界返回空指针
	if (x < 0 || y < 0 || x > w || y > h) { return nullptr; }

	return &mapData[x + y * w];
}