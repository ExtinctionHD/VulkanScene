#include "Application.h"
#include <iostream>

int main()
{
	Application app;

	try
	{
		app.run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		getchar();
	}
}