#include <iostream>

int main()
{
	std::cout << "main1 is running." << std::endl;
	std::cout << "I changed the code for the second commit" << std::endl;
#ifdef RESOURCES_PATH
	std::cout << "RESOURCES_PATH = " << RESOURCES_PATH << std::endl;
#endif
	return 0;
}
