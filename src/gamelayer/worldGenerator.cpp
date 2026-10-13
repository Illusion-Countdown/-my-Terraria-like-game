#include "worldGenerator.h"
#include "randomStuff.h"
#include <FastNoiseSIMD.h>
#include <algorithm> //std::clamp
#include <cmath>	 //std::ceil
#include <cstdlib>	 //std::abs
#include <structure.h>
#include <saveMap.h>
void generatorWorld(GameMap &gameMap, int seed)
{
	const int w = 700;
	const int h = 350;
	gameMap.create(w, h);

	std::ranlux24_base rng(seed++);
#pragma region world_map
	// 土层
	std::unique_ptr<FastNoiseSIMD> dirtNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());
	std::unique_ptr<FastNoiseSIMD> cavesNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());

	dirtNoiseGenerator->SetSeed(seed++);
	cavesNoiseGenerator->SetSeed(seed++);

	dirtNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	dirtNoiseGenerator->SetFractalOctaves(1);
	dirtNoiseGenerator->SetFrequency(0.01);
	dirtNoiseGenerator->SetFractalGain(0.45f);

	cavesNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	cavesNoiseGenerator->SetFractalOctaves(3);
	cavesNoiseGenerator->SetFrequency(0.02f);

	float *dirtNoise = FastNoiseSIMD::GetEmptySet(w);

	dirtNoiseGenerator->FillNoiseSet(dirtNoise, 0, 0, 0, w, 1, 1);
	// 将数据从[-1,1]转换到[0,1]
	for (int i = 0; i < w; i++)
		dirtNoise[i] = (dirtNoise[i] + 1) / 2;

	float *cavesNoise = FastNoiseSIMD::GetEmptySet(w * h);
	cavesNoiseGenerator->FillNoiseSet(cavesNoise, 0, 0, 0, h, w, 1); // 先h再w
	for (int i = 0; i < w * h; i++)
		cavesNoise[i] = (cavesNoise[i] + 1) / 2;
	auto getCaveNoise = [&](int x, int y)
	{ return cavesNoise[x + y * w]; }; // lambda表达式

	int dirtOffsetStart = -5;
	int dirtOffsetEnd = 35;
	// 岩石层
	int stoneHeight = 80;
	int keepDirectionTimeStone = getRandomInt(rng, 5, 50);
	int directionStone = getRandomInt(rng, -2, 2);
	// 沙漠群落
	int desertStart = getRandomInt(rng, 100, w - 200);
	int desertEnd = desertStart + getRandomInt(rng, 0, 100);
	if (desertEnd >= w) { desertEnd = w - 100; }
	// 树
	Structure treeStructure;
	loadBlockDataFromFile(treeStructure.mapData, treeStructure.w, treeStructure.h, RESOURCES_PATH "sturcture/tree2.bin");

	for (int x = 0; x < w; x++)
	{
		bool inDesert = (x >= desertStart && x <= desertEnd);
#pragma region stone // 岩石层
		keepDirectionTimeStone--;
		if (keepDirectionTimeStone <= 0)
		{
			keepDirectionTimeStone = getRandomInt(rng, 5, 50);
			directionStone = getRandomInt(rng, -2, 2);
		}
		if (directionStone == -2)
		{
			if (getRandomChance(rng, 0.2))
				stoneHeight--;
			if (getRandomChance(rng, 0.2))
				stoneHeight--;
		}
		if (directionStone == -1)
			if (getRandomChance(rng, 0.2))
				stoneHeight--;
		if (directionStone == 1)
			if (getRandomChance(rng, 0.2))
				stoneHeight++;
		if (directionStone == 2)
		{
			if (getRandomChance(rng, 0.2))
				stoneHeight++;
			if (getRandomChance(rng, 0.2))
				stoneHeight++;
		}
		if (stoneHeight > 120) { stoneHeight = 120; }
		if (stoneHeight < 50) { stoneHeight = 50; }
#pragma endregion

		int dirtThickness = 3 + (int)(dirtOffsetStart + (dirtOffsetEnd - dirtOffsetStart) * dirtNoise[x]);
		int dirtHeight = stoneHeight - dirtThickness;

		int grassType = Block::grassBlock;
		int dirtType = Block::dirt;
		int stoneType = Block::stone;
		if (inDesert)
		{
			grassType = Block::sand;
			dirtType = Block::sand;
			stoneType = Block::sandStone;
		}
		for (int y = 0; y < h; y++)
		{
			Block b;
			if (y == dirtHeight)
				b.type = grassType;
			else if (y > dirtHeight)
				b.type = dirtType;
			if (y > stoneHeight)
			{
				b.type = stoneType;
				if (getRandomChance(rng, 0.005)) { b.type = Block::gold; }
			}
			// 沙漠地貌
			if (inDesert)
			{
				int desertMid = (desertStart + desertEnd) / 2;
				int desertHalfWidth = (desertEnd - desertStart) / 2;
				int distanceFromDesertMid = std::abs(x - desertMid);

				float desertRelativeDepth = 1 - (float)distanceFromDesertMid / (float)desertHalfWidth;
				float desertStoneStart = 10 + stoneHeight;
				float desertDepth = 50;
				int triangleStoneY = desertStoneStart + desertRelativeDepth * desertDepth;
				if (y > triangleStoneY)
					b.type = Block::stone;
			}
			// 洞穴
			if (getCaveNoise(x, y) <= 0.33 || getCaveNoise(x, y) >= 0.75)
				b.type = Block::air;

			gameMap.getBlockUnsafe(x, y) = b;
		}
	}
	FastNoiseSIMD::FreeNoiseSet(dirtNoise);
	FastNoiseSIMD::FreeNoiseSet(cavesNoise);
#pragma endregion
#pragma region perlin_worms
	auto spawnWorm = [&](float startX, float startY, float maxLength, float maxR)
	{
		float x = startX;
		float y = startY;
		float dirX = getRandomFloat(rng, -2, 2);
		float dirY = getRandomFloat(rng, -2, 2);

		int wormLength = maxLength;
		float radius = maxR;
		int changeDirectionTime = getRandomInt(rng, 5, 20);
		for (int i = 0; i < wormLength; i++)
		{
			int intRadius = std::ceil(radius);
			for (int ox = -intRadius; ox < intRadius; ox++)
			{
				for (int oy = -intRadius; oy < intRadius; oy++)
				{
					if (ox * ox + oy * oy <= radius * radius)
					{

						int digX = x + ox;
						int digY = y + oy;
						auto b = gameMap.getBlockSafe(digX, digY);
						if (b) { b->type = Block::air; }
					}
				}
			}
			changeDirectionTime--;
			if (changeDirectionTime <= 0)
			{
				changeDirectionTime = getRandomInt(rng, 5, 20);
				float keepfactor = 0.5;
				if (getRandomChance(rng, 0.7))
					keepfactor = 0.8;
				else
					keepfactor = 0.2;
				dirX = dirX * keepfactor + getRandomFloat(rng, -1, 1) * (1.f - keepfactor);
				dirY = dirY * keepfactor + getRandomFloat(rng, -1, 1) * (1.f - keepfactor);
			}
			x += dirX * 1.5f;
			y += dirY * 1.5f;
			radius += getRandomFloat(rng, -0.2f, 0.2f);
			radius = std::clamp(radius, 2.2f, 5.5f);
		}
	};
	for (int i = 0; i < 10; i++)
	{
		spawnWorm(getRandomFloat(rng, 30, w - 30),
				  getRandomFloat(rng, 50, h - 10),
				  getRandomInt(rng, 200, 500),
				  2.5f);
	}
#pragma endregion
#pragma region fill_tree
	for (int x = 0; x < w; x++)
		if (getRandomChance(rng, 0.04))
			for (int y = 0; y < h; y++)
			{
				auto type = gameMap.getBlockUnsafe(x, y).type;
				if (type == Block::air)
					continue;
				if (type == Block::grassBlock)
				{
					Vector2 spawnPos{(float)x, (float)y};
					spawnPos.x -= treeStructure.w / 2;
					spawnPos.y -= treeStructure.h;
					treeStructure.pasteIntoMap(gameMap, spawnPos);
					x += 3;
					break;
				}
				else
					break;
			}

#pragma endregion
}