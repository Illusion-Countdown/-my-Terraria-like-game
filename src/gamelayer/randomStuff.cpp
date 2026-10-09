#include "randomStuff.h"
float getRandomFloat(std::ranlux24_base &rng, float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max); // 限制随机数范围
	return dist(rng);									  // 实现随机数生成
}
int getRandomInt(std::ranlux24_base &rng, int min, int max)
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}
bool getRandomChance(std::ranlux24_base &rng, float chance)
{
	float dice = getRandomFloat(rng, 0.0f, 1.0f);
	return dice <= chance;
}