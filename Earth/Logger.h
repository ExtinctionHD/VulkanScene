#pragma once

#include <iostream>

// критические ошибки
#define LOGGER_FATAL(msg)														\
{																				\
	std::string message = (msg);												\
	Logger::fatal(Logger::VALIDATION_LAYERS_NOT_AVAILABLE, __FILE__, __LINE__);	\
}

class Logger
{
public:
	static const std::string VALIDATION_LAYERS_NOT_AVAILABLE;
	static const std::string INSTANCE_EXTENSIONS_NOT_AVAILABLE;
	static const std::string FAILED_TO_CREATE_INSTANCE;

	static void infoValidationLayers(bool enabled);  // вывод информации о слоях проверки

	static void fatal(std::string message, std::string file, int line);  // критические ошибки

private:
	static void printInfo();

	static std::string getFilename(std::string path);  // получение имени из полного пути
};

