#pragma once
#include <vector>
#include "block.h"
struct GameMap
{
	int w = 0;
	int h = 0;
	std::vector<Block> mapData;
	void create(int w, int h);
	Block &getBlockUnsafe(int x, int y); // 确定不会越界时效率更高
	Block *getBlockSafe(int x, int y);	 // 会检查是否超出地图外,但是会消耗额外时间
};