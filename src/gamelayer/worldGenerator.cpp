#include "worldGenerator.h"
#include "randomStuff.h"
#include <FastNoiseSIMD.h>
void generatorWorld(GameMap &gameMap, int seed)
{
	const int w = 700;
	const int h = 350;
	gameMap.create(w, h);

	int stoneHeight = 90;
	int dirtHeight = 50;
	std::ranlux24_base rng(seed);
	int keepDirectionTimeDirt = getRandomInt(rng, 5, 40);
	int directionDirt = getRandomInt(rng, -2, 2);

	int keepDirectionTimeStone = getRandomInt(rng, 5, 40);
	int directionStone = getRandomInt(rng, -2, 2);
	for (int x = 0; x < w; x++)
	{
#pragma region random_terrain
		// 土层
		keepDirectionTimeDirt--;
		if (keepDirectionTimeDirt <= 0)
			keepDirectionTimeDirt = getRandomInt(rng, 5, 40);
		directionDirt = getRandomInt(rng, -2, 2);
		if (directionDirt == -2)
		{
			if (getRandomChance(rng, 0.2))
				dirtHeight--;
			if (getRandomChance(rng, 0.2))
				dirtHeight--;
		}
		if (directionDirt == -1)
		{
			if (getRandomChance(rng, 0.2))
				dirtHeight--;
		}

		if (directionDirt == 1)
		{
			if (getRandomChance(rng, 0.2))
				dirtHeight++;
		}
		if (directionDirt == 2)
		{
			if (getRandomChance(rng, 0.2))
				dirtHeight++;
			if (getRandomChance(rng, 0.2))
				dirtHeight++;
		}
		// 岩石层
		keepDirectionTimeStone--;
		if (keepDirectionTimeStone <= 0)
			keepDirectionTimeStone = getRandomInt(rng, 5, 40);
		int directionStone = getRandomInt(rng, -2, 2);
		if (directionDirt == -2)
		{
			if (getRandomChance(rng, 0.2))
				stoneHeight--;
			if (getRandomChance(rng, 0.2))
				stoneHeight--;
		}
		if (directionStone == -1)
		{
			if (getRandomChance(rng, 0.2))
				stoneHeight--;
		}

		if (directionDirt == 1)
		{
			if (getRandomChance(rng, 0.2))
				stoneHeight++;
		}
		if (directionStone == 2)
		{
			if (getRandomChance(rng, 0.2))
				stoneHeight++;
			if (getRandomChance(rng, 0.2))
				stoneHeight++;
		}
		if (stoneHeight < 60) { stoneHeight = 60; }
		if (stoneHeight > 120) { stoneHeight = 120; }
#pragma endregion
		for (int y = 0; y < h; y++)
		{
			Block b;
			if (y == dirtHeight)
				b.type = Block::grassBlock;
			else if (y > dirtHeight)
				b.type = Block::dirt;
			if (y > stoneHeight)
			{
				b.type = Block::stone;
				if (getRandomChance(rng, 0.01)) { b.type = Block::gold; }
			}

			gameMap.getBlockUnsafe(x, y) = b;
		}
	}
}