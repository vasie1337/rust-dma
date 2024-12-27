#include "include.hpp"

int main()
{
	std::cout << "Starting DMA Cheat" << std::endl;

	try
	{
		Cheat cheat;
		cheat.Run();
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
		Sleep(5000);
		return 1;
	}
}