#include "stdafx.h"
#include "ConfigParser.h"

ConfigParser::ConfigParser()
{
}

const float &ConfigParser::get_spinning() const{
	return spinning;
}

const float &ConfigParser::get_spinSpeed() const {
	return spinSpeed;
}

const ConfigParser::Color &ConfigParser::get_backgroundColor() const {
	return backgroundColor;
}

const std::string &ConfigParser::get_terrainPath() const {
	return terrainPath;
}

const float &ConfigParser::get_terrainWidth() const {
	return terrainWidth;
}

const float &ConfigParser::get_terrainDepth() const {
	return terrainDepth;
}

const float &ConfigParser::get_terrainHeight() const {
	return terrainHeight;
}

void ConfigParser::load() {
	std::ifstream file("game.cfg");
	std::string line;

	std::regex reg("([0-9]*[.])?[0-9]+");
	std::smatch match;

	if (!file.is_open()) {
		throw "file could not be read.";
		file.close();
		return;
	}

	while (std::getline(file, line)) {

		if (line.find("terrainPath") != std::string::npos) {
			const int starting_index = 12;
			this->terrainPath = line.substr(starting_index, line.size() - starting_index);
		}
		else if (line.find("backgroundColor") != std::string::npos) {
			int starting_ind = 16;
			std::string s;

			// Initialize the color values.
			for (int i = 0; i < 2; i++) {
				while (line[starting_ind] != ' ') {
					s.push_back(line[starting_ind]);
					starting_ind++;
				}
				starting_ind++;
				if(i == 0)	backgroundColor.r = std::stof(s);
				else if(i == 1)	backgroundColor.g = std::stof(s);
				s = "";
			}

			while (starting_ind < line.size()) {
				s.push_back(line[starting_ind]);
				starting_ind++;
			}
			backgroundColor.b = std::stof(s);

		}

		if (std::regex_search(line, match, reg)) {

			if (line.find("spinning") != std::string::npos) {
				this->spinning = std::stof(match[0]);
			}

			else if (line.find("spinSpeed") != std::string::npos) {
				this->spinSpeed = std::stof(match[0]);
			}

			else if (line.find("terrainWidth") != std::string::npos) {
				this->terrainWidth = std::stof(match[0]);
			}

			else if (line.find("terrainDepth") != std::string::npos) {
				this->terrainDepth = std::stof(match[0]);
			}

			else if (line.find("terrainHeight") != std::string::npos) {
				this->terrainHeight = std::stof(match[0]);
			}
		}
	}

	file.close();
}

ConfigParser::~ConfigParser()
{
}
