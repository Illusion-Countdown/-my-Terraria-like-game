#include "saveMap.h"
#include <asserts.h>
#include <filesystem>

struct BlockSaveRepresentation01
{
	std::uint16_t type = 0;

	Block toBlock()
	{
		Block b;
		b.type = type;
		b.durability = 1;
		return b;
	}
};
struct BlockSaveRepresentation02 // 新增方块耐久
{
	std::uint16_t type = 0;
	std::uint8_t durability = 0;

	Block toBlock()
	{
		Block b;
		b.type = type;
		b.durability = durability;
		return b;
	}
};
const int VERSION = 02;

BlockSaveRepresentation02 toBlockRepresentation(Block b)
{
	BlockSaveRepresentation02 rez;
	rez.type = b.type;
	rez.durability = b.durability;
	return rez;
}

bool saveBlockDataToFile(const std::vector<Block> blocks, int w, int h, const std::string &fileName)
{
	// 先确保父目录存在,否则新克隆的仓库(空文件夹不会被git保存)里ofstream会打开失败
	auto parentDir = std::filesystem::path(fileName).parent_path();
	// 当empty()->false时,也就是fileName写了父目录但是目录不存在时,会执行if,创建父目录
	if (!parentDir.empty())
	{
		std::error_code ec;
		std::filesystem::create_directories(parentDir, ec);
	}

	std::ofstream f(fileName, std::ios::binary);
	if (!f.is_open()) { return false; }

	// 开发阶段,断言对数据的检查
	permaAssertDevelopement(blocks.size() == w * h);
	permaAssertDevelopement(blocks.size() != 0);
	// 正式版本中,运行时的防御性检查
	if (blocks.size() != w * h) { return false; }
	if (blocks.size() == 0) { return false; }

	// write(首地址,字节数) 标准库二进制写入函数
	// write时,程序把数据拷贝到二进制文件中,不能改动传入参数,因此需要传入const char *
	// read时,程序从文件中读取数据,会改变传入参数,因此写char *
	f.write((const char *)&VERSION, sizeof(VERSION));
	f.write((const char *)&w, sizeof(w)); // w,h,block.data()必须通过强转成const char *让f.write识别
	f.write((const char *)&h, sizeof(h));

	// oldVersion: f.write((const char *)blocks.data(), sizeof(Block) * blocks.size());
	//  data()返回容器的首地址
	for (int i = 0; i < blocks.size(); i++)
	{
		auto b = toBlockRepresentation(blocks[i]);
		f.write((const char *)&b, sizeof(b));
	}

	f.close();
	return true;
}

// 读取数据时尽可能多检查,保证不不出现错误
// 读取失败时保留传入的原有数据,避免把复制好的structure清空
bool loadBlockDataFromFile(std::vector<Block> &blocks, int &w, int &h, const std::string &fileName)
{
	std::ifstream f(fileName, std::ios::binary); // ifsream可以不用写close(),离开作用域时自动close()
	if (!f.is_open()) { return false; }

	// 先读到临时变量,全部检查通过后再写回
	int loadedW = 0;
	int loadedH = 0;
	int readVersion = 00;
	// 读取版本
	f.read((char *)&readVersion, sizeof(readVersion));

	// 读取长宽数据
	f.read((char *)&loadedW, sizeof(loadedW));
	f.read((char *)&loadedH, sizeof(loadedH));
	// 检查长宽数据(save不会写入空structure,所以0也视为损坏)
	if (!f) { return false; }
	if (loadedW > 10000 || loadedH > 10000 || loadedW <= 0 || loadedH <= 0) { return false; }

	size_t blockCount = loadedH * loadedW;
	std::vector<Block> loadedBlocks(blockCount);
	// 读取方块的数据
	switch (readVersion)
	{
	case 01:
	{
		for (int i = 0; i < blockCount; i++)
		{
			BlockSaveRepresentation01 read;
			f.read((char *)&read, sizeof(read));

			// 检查数据流状态
			if (!f) { return false; }

			loadedBlocks[i] = read.toBlock();
		}
		break;
	}
	case 02:
		for (int i = 0; i < blockCount; i++)
		{
			BlockSaveRepresentation02 read;
			f.read((char *)&read, sizeof(read));

			// 检查数据流状态
			if (!f) { return false; }

			loadedBlocks[i] = read.toBlock();
		}
		break;
	default:
		return false;
	}
	// oldVersion:
	//  size_t blockCount = (size_t)loadedW * (size_t)loadedH;
	//  std::vector<Block> loadedBlocks(blockCount);
	//  f.read((char *)loadedBlocks.data(), sizeof(Block) * blockCount);
	// 若方块数据损毁,则修改为air
	for (auto &b : loadedBlocks)
		b.sanitize();

	blocks = std::move(loadedBlocks);
	w = loadedW;
	h = loadedH;
	return true;
}