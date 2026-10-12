#include "structure.h"
#include <asserts.h>

void Structure::create(int w, int h)
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

Block &Structure::getBlockUnsafe(int x, int y)
{
	// 若没有调用create()函数,则会报错
	permaAssertCommentDevelopement(mapData.size() == w * h, "Map dat not initialized");
	// 仅限开发阶段,若超出地图边界依旧会报错
	permaAssertCommentDevelopement(x >= 0 && y >= 0 && x < w && y < h, "getBlockUnsafe out of bounds error");

	return mapData[x + y * w];
}
Block *Structure::getBlockSafe(int x, int y)
{
	permaAssertCommentDevelopement(mapData.size() == w * h, "Map dat not initialized");
	// 超出地图边界返回空指针
	if (x < 0 || y < 0 || x >= w || y >= h) { return nullptr; }

	return &mapData[x + y * w];
}
void Structure::copyFromMap(GameMap &map, Vector2 start, Vector2 end)
{
	// 检查
	if (start.x > map.w - 1) { start.x = map.w - 1; }
	if (end.x > map.w - 1) { end.x = map.w - 1; }

	if (start.y > map.h - 1) { start.y = map.h - 1; }
	if (end.y > map.h - 1) { end.y = map.h - 1; }

	if (start.x < 0) { start.x = 0; }
	if (end.x < 0) { end.x = 0; }

	if (start.y < 0) { start.y = 0; }
	if (end.y < 0) { end.y = 0; }

	if (start.x > end.x)
		std::swap(start.x, end.x);
	if (start.y > end.y)
		std::swap(start.y, end.y);

	// attention:+1
	Vector2 size = Vector2{end.x - start.x + 1, end.y - start.y + 1};

	if (size.x > map.w) { return; }
	if (size.y > map.h) { return; }

	create(size.x, size.y);
	for (int x = 0; x < size.x; x++)
		for (int y = 0; y < size.y; y++)
			getBlockUnsafe(x, y) = map.getBlockUnsafe(x + start.x, y + start.y);
}
void Structure::pasteIntoMap(GameMap &map, Vector2 start)
{
	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
		{
			auto b = map.getBlockSafe(x + start.x, y + start.y);
			if (b) { *b = getBlockUnsafe(x, y); }
		}
}