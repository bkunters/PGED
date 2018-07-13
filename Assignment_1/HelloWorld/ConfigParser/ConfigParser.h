#include <string>
#include <regex>
#include <fstream>
#include <iostream>

#pragma once
class ConfigParser
{
public:
	struct Color {
		float r;
		float g;
		float b;
	};

	ConfigParser();
	void load();

	// Getters.
	const float &get_spinning() const;
	const float &get_spinSpeed() const;
	const Color &get_backgroundColor() const;
	const std::string &get_terrainPath() const;
	const float &get_terrainWidth() const;
	const float &get_terrainDepth() const;
	const float &get_terrainHeight() const;

	~ConfigParser();
private:
	// Config properties.
	float spinning;
	float spinSpeed;
	Color backgroundColor;
	std::string terrainPath;
	float terrainWidth;
	float terrainDepth;
	float terrainHeight;

	
};

