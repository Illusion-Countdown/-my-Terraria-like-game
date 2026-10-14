#include "worldGenerator.h"
#include "randomStuff.h"
#include <FastNoiseSIMD.h>
#include <structure.h>
#include <saveMap.h>
#include <algorithm>

// 噪声类型   Simplex:一个频率的平滑连续值    SimplexFractal:多层Simplex噪声,层次更丰富,更自然
void generatorWorld(GameMap &gameMap, int seed)
{
	const int w = 1000;
	const int h = 400;
	gameMap.create(w, h);
	int stoneType = Block::stone;
	int dirtType = Block::dirt;
	int grassType = Block::grassBlock;
	int airType = Block::air;
	// 世界生成顺序:岩石基底 → 高山隆起 → 洞穴侵蚀
	// → 土层/草皮覆盖 → 沙漠/雪山生物群系
	// → 湖泊 → 矿石 → 结构
	int base_surface = 110;
	std::vector<int> surface(w);
	std::vector<int> stoneLayerStart(w);

	std::ranlux24_base rng(seed++);

	auto createStoneLayer = [&]()
	{
		int keepDirectionTimeStone = getRandomInt(rng, 5, 50);
		int directionStone = getRandomInt(rng, -2, 2);
		for (int x = 0; x < w; x++)
		{
			keepDirectionTimeStone--;
			if (keepDirectionTimeStone <= 0)
			{
				keepDirectionTimeStone = getRandomInt(rng, 5, 50);
				directionStone = getRandomInt(rng, -2, 2);
			}
			if (directionStone == -2)
			{
				if (getRandomChance(rng, 0.2))
					base_surface--;
				if (getRandomChance(rng, 0.2))
					base_surface--;
			}
			if (directionStone == -1)
				if (getRandomChance(rng, 0.2))
					base_surface--;
			if (directionStone == 1)
				if (getRandomChance(rng, 0.2))
					base_surface++;
			if (directionStone == 2)
			{
				if (getRandomChance(rng, 0.2))
					base_surface++;
				if (getRandomChance(rng, 0.2))
					base_surface++;
			}
			if (base_surface > 150) { base_surface = 150; }
			if (base_surface < 80) { base_surface = 80; }
			surface[x] = base_surface;
			for (int y = 0; y < h; y++)
			{
				Block b;
				if (y >= base_surface)
					b.type = Block::stone;
				gameMap.getBlockUnsafe(x, y) = b;
			}
		}
	};
	auto addExtraMountain = [&]()
	{
		// 控制山体形状(高频,剧烈抖动)
		auto noise = FastNoiseSIMD::NewFastNoiseSIMD(seed++);
		noise->SetNoiseType(FastNoiseSIMD::SimplexFractal);
		noise->SetFractalOctaves(4);	   // 细节度[3~6最好](细节丰富,山脉,山包,碎石坑)
		noise->SetFrequency(0.005f);	   // 山脉间距(越小越宽,越大越陡)
		noise->SetFractalLacunarity(2.0f); // 频率乘数(越高越破碎)
		noise->SetFractalGain(0.5);		   // 越大越崎岖

		// 控制山的生成位置(低频)
		// mask只负责大轮廓的实现,所以不用像山脉一样需要具体的细节填写
		// 默认:   Octaves:1 ,Lacunarity:2.f ,FractalGain: 0.5f
		auto maskNoise = FastNoiseSIMD::NewFastNoiseSIMD(seed++);
		maskNoise->SetNoiseType(FastNoiseSIMD::Simplex);
		maskNoise->SetFrequency(0.002f); // 频率(越低,山区越宽)

		// 得到两张噪声长图
		float *heightMap = noise->GetNoiseSet(0, 0, 0, w, 1, 1, 1.0f);
		float *maskMap = maskNoise->GetNoiseSet(0, 0, 0, w, 1, 1, 1.0f);
		for (int x = 0; x < w; x++)
		{
			int baseY = surface[x];

			// mountFactor>0时,形成山区;否则为0,extraheight=0
			// maskMap[x]取值范围为[-1,1],经处理后在[-0.67,1]之内,略微提高标记为山区概率
			float mountFactor = std::max(0.0f, (maskMap[x] + 0.2f) / 1.2f); // std::max(a,b),返回a,b中较大值
			if (mountFactor > 0.01f)
			{
				int extraHeight = static_cast<int>(std::abs(heightMap[x]) * mountFactor * 180.f); // ×heightMap[x]使得到的山起伏较大
				int peakY = baseY - extraHeight;
				if (peakY < 10)
					peakY = 10;
				for (int y = peakY; y < baseY; y++)
					gameMap.getBlockUnsafe(x, y).type = stoneType;
				surface[x] = peakY;
			}
		}
		FastNoiseSIMD::FreeNoiseSet(heightMap);
		FastNoiseSIMD::FreeNoiseSet(maskMap);
	};
	auto addBaseCaves = [&]()
	{
		auto noise = FastNoiseSIMD::NewFastNoiseSIMD(seed++);
		noise->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
		noise->SetFractalOctaves(4);
		noise->SetFrequency(0.02f);

		// 保证xSize*ySize=w*h,但噪声尺寸需要转置为h,w,以获得适合的洞穴形状
		float *caveNoiseMap = noise->GetNoiseSet(0, 0, 0, h, w, 1, 1.0f);
		for (int i = 0; i < w * h; i++)
			caveNoiseMap[i] = (caveNoiseMap[i] + 1) / 2;

		auto getCaveNoise = [&](int x, int y)
		{ return caveNoiseMap[x + y * w]; };
		for (int x = 0; x < w; x++)
			for (int y = surface[x] + 9; y < h; y++)
				if (getCaveNoise(x, y) < 0.33 || getCaveNoise(x, y) > 0.77)
					gameMap.getBlockUnsafe(x, y).type = airType;
		FastNoiseSIMD::FreeNoiseSet(caveNoiseMap);
	};
	auto addPerlinWormChannels = [&]()
	{
		auto PerlinWorm = [&](int startX, int startY, int wormLength, int radius)
		{
			int x = startX;
			int y = startY;

			int len = wormLength;
			float r = radius;
			for (int i = 0; i < len; i++)
			{
				if (x < 0 || x > w || y < surface[x] + 8 || y > h) { break; }

				int keepDirectionTime = getRandomInt(rng, 5, 25);
				int dirX = getRandomInt(rng, -2, 2);
				int dirY = getRandomInt(rng, -2, 2);
				int intR = std::ceil(radius);
				for (int ox = -intR; ox < intR; ox++)
					for (int oy = -intR; oy < intR; oy++)
						if (ox * ox + oy * oy < r * r)
						{
							int digX = x + ox;
							int digY = y + oy;
							auto b = gameMap.getBlockSafe(digX, digY);
							if (b) { b->type = airType; }
						}
				keepDirectionTime--;
				if (keepDirectionTime <= 0)
				{
					keepDirectionTime = getRandomInt(rng, 5, 25);
					float keepFactor = 0.5;
					if (getRandomChance(rng, 0.69))
						keepFactor = 0.8;
					else
						keepFactor = 0.2;
					dirX = dirX * keepFactor + getRandomInt(rng, -2, 2) * (1 - keepFactor);
					dirY = dirY * keepFactor + getRandomInt(rng, -2, 2) * (1 - keepFactor);
					r += getRandomFloat(rng, -0.2f, 0.2f);
					r = std::clamp(r, 1.f, 4.f);
				}
				x += dirX * 1.2f;
				y += dirY * 1.2f;
			}
		};

		for (int i = 0; i < 8; i++)
		{
			int startX = getRandomInt(rng, 40, w - 50);
			int startY = getRandomInt(rng, surface[startX] + 50, h - 60);
			int wormLength = getNormalInt(rng, 50, 800, 5);
			float radius = 2.f;
			PerlinWorm(startX, startY, wormLength, radius);
		}
	};

	auto addDirtLayer = [&]()
	{
		int baseThickness = 18;
		int minThickness = 6;

		auto noise = FastNoiseSIMD::NewFastNoiseSIMD(seed++);
		noise->SetNoiseType(FastNoiseSIMD::Simplex);
		noise->SetFrequency(0.03f);
		float *dirtThicknessMap = noise->GetNoiseSet(0, 0, 0, w, 1, 1, 1.0f);

		for (int x = 0; x < w; x++)
		{
			int surfaceY = surface[x];
			if (surfaceY < 0 || surfaceY >= h)
				continue;
			float slope = 0;

			if (x > 1 && x < w - 2)
				slope = std::abs((surface[x - 2] - surface[x + 2]) * 0.5f + (surface[x - 1] - surface[x + 1])) * 0.1f;

			float nosieFactor = 0.7f + 0.3f * dirtThicknessMap[x];
			float slopeFactor = std::max(0.3f, 1.0f - slope);
			int adaptiveThickness = std::clamp(std::ceilf(baseThickness * nosieFactor * slopeFactor), (float)minThickness, (float)h);
			stoneLayerStart[x] = surfaceY + adaptiveThickness;
			for (int dy = 0; dy < adaptiveThickness; dy++)
			{
				Block &b_now = gameMap.getBlockUnsafe(x, dy + surfaceY),
					  b_next = gameMap.getBlockUnsafe(x, dy + 1 + surfaceY);
				if (b_now.type == stoneType && b_next.type != airType)
					if (dy == 0)
						b_now.type = grassType;
					else
						b_now.type = dirtType;
			}
		}
		FastNoiseSIMD::FreeNoiseSet(dirtThicknessMap);
	};

	// base world outline
	createStoneLayer();
	addExtraMountain();
	// diversve caves
	addBaseCaves();
	addPerlinWormChannels();

	addDirtLayer();
}