#pragma once
#include <random>
float getRandomFloat(std::ranlux24_base &rng, float min, float max);
int getRandomInt(std::ranlux24_base &rng, int min, int max);
bool getRandomChance(std::ranlux24_base &rng, float chance);
int getNormalInt(std::ranlux24_base &rng, int min, int max, float deviation = 1);
float getNormalFloat(std::ranlux24_base &rng, float min, float max, float deviation = 1);