#pragma once
#include "block.h"
#include <vector>
#include <fstream>
#include <string>
bool saveBlockDataToFile(const std::vector<Block> blocks, int w, int h, const std::string &fileName);

bool loadBlockDataFromFile(std::vector<Block> &blocks, int &w, int &h, const std::string &fileName);