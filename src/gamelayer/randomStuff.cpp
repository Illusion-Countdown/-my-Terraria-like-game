#include "randomStuff.h"
#include <algorithm>
#include <cmath>
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
int getNormalInt(std::ranlux24_base &rng, int min, int max, float deviation)
{
	int average = (max + min) / 2;
	// normal_distribution 的模板参数必须是浮点类型,不能是 int
	std::normal_distribution<float> dist((float)average, deviation);
	int num = static_cast<int>(std::round(std::clamp(dist(rng), (float)min, (float)max)));
	return num;
}
float getNormalFloat(std::ranlux24_base &rng, float min, float max, float deviation)
{
	float average = (max + min) / 2;
	std::normal_distribution<float> dist(average, deviation);
	return std::clamp(dist(rng), min, max);
}