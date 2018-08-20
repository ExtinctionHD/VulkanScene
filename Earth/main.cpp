#include "Application.h"
#include <iostream>

int main()
{
	try
	{
		Application app;

		app.run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		getchar();
	}
}