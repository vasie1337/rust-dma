#include "include.hpp"

int main()
{
	//try
	{
		Cheat cheat;
		cheat.Run();
		return 0;
	}
	//catch (const std::exception& e)
	{
	//	std::cout << "Error: " << e.what() << std::endl;
	//	Sleep(5000);
	//	return 1;
	}
}