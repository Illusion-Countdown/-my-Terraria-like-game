#include <iostream>

int main()
{
    std::cout << "main1 is running." << std::endl;
#ifdef RESOURCES_PATH
    std::cout << "RESOURCES_PATH = " << RESOURCES_PATH << std::endl;
#endif
    return 0;
}
