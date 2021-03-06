// ConfigParser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ConfigParser.h"

int main()
{
	ConfigParser cfg;
	cfg.load();
	std::cout << cfg.get_spinning() << std::endl;
	std::cout << cfg.get_spinSpeed() << std::endl;
	std::cout << cfg.get_terrainPath() << std::endl;
	std::cout << cfg.get_terrainWidth() << std::endl;
	std::cout << cfg.get_terrainDepth() << std::endl;
	std::cout << cfg.get_terrainHeight() << std::endl;
	std::cout << cfg.get_backgroundColor().r << std::endl;
	std::cout << cfg.get_backgroundColor().g << std::endl;
	std::cout << cfg.get_backgroundColor().b << std::endl;

	system("pause");
    return 0;
}

