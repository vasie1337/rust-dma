#include "include.hpp"

int main()
{
	try
	{
		Cheat cheat;
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		std::getchar();
		return 1;
	}
}