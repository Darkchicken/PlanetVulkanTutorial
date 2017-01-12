

#include "TesterGame.h"
#include <stdexcept>
#include <iostream>

int main(int argc, char** argv)
{
	TesterGame testGame;

	try
	{
		testGame.Run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}