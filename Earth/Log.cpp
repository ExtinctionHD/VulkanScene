#include <iostream>

#include "Log.h"

void Log::validationLayers(bool enabled)
{
	if (enabled)
	{
		std::cout << "Validation layers enabled." << std::endl;
	}
	else
	{
		std::cout << "Validation layers not enabled." << std::endl;
	}
}
