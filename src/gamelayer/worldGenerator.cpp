#include "worldGenerator.h"
#include "randomStuff.h"
#include <FastNoiseSIMD.h>
void generatorWorld(GameMap &gameMap, int seed)
{
	const int w = 700;
	const int h = 350;
	gameMap.create(w, h);

	std::ranlux24_base rng(seed++);

	std::unique_ptr<FastNoiseSIMD> dirtNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());
	std::unique_ptr<FastNoiseSIMD> stoneNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());

	dirtNoiseGenerator->SetSeed(seed++);
	stoneNoiseGenerator->SetSeed(seed++);

	dirtNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	dirtNoiseGenerator->SetFractalOctaves(1);
	dirtNoiseGenerator->SetFrequency(0.02);

	stoneNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	stoneNoiseGenerator->SetFractalOctaves(4);
	stoneNoiseGenerator->SetFrequency(0.01);

	float *dirtNoise = FastNoiseSIMD::GetEmptySet(w);
	float *stoneNoise = FastNoiseSIMD::GetEmptySet(w);

	dirtNoiseGenerator->FillNoiseSet(dirtNoise, 0, 0, 0, w, 1, 1);
	stoneNoiseGenerator->FillNoiseSet(stoneNoise, 0, 0, 0, w, 1, 1);
	// 将数据从[-1,1]转换到[0,1]
	for (int i = 0; i < w; i++)
	{
		dirtNoise[i] = (dirtNoise[i] + 1) / 2;
		stoneNoise[i] = (stoneNoise[i] + 1) / 2;
		stoneNoise[i] = std::pow(stoneNoise[i], 2);
	}

	int dirtOffsetStart = -5;
	int dirtOffsetEnd = 35;
	int stoneHeightStart = 80;
	int stoneHeightEnd = 170;
	for (int x = 0; x < w; x++)
	{
		int stoneHeight = stoneHeightStart + (stoneHeightEnd - stoneHeightStart) * stoneNoise[x];
		int dirtHeight = dirtOffsetStart = (dirtOffsetEnd - dirtOffsetStart) * dirtNoise[x];
		dirtHeight = stoneHeight - dirtHeight;
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
	FastNoiseSIMD::FreeNoiseSet(dirtNoise);
	FastNoiseSIMD::FreeNoiseSet(stoneNoise);
}