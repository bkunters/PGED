#include "ConfigParser.h"
#include <string>

#include "debug.h"

ConfigParser::ConfigParser()
{
}


ConfigParser::~ConfigParser()
{
}

void ConfigParser::load(std::string filename)
{
	std::ifstream ifs(filename);
	if (ifs.is_open())
	{
		std::string varType;
		while (ifs >> varType)
		{
			std::string strValue;
			ifs >> strValue;
			if (varType.compare("TerrainPath") == 0) 
			{
				pathHeight = strValue;
				ifs >> pathColor >> pathNormal;
			}
			else if (varType.compare("spinning") == 0)
				spinning = std::stof(strValue);
			//else if (varType.compare("backgroundColor") == 0)
			//{
			//	backgroundColor.r = std::stof(strValue);
			//	std::string strVal2, strVal3;
			//	ifs >> strVal2 >> strVal3;
			//	backgroundColor.g = std::stof(strVal2);
			//	backgroundColor.b = std::stof(strVal3);
			//}
			else if (varType.compare("TerrainWidth") == 0)
				terrainWidth = std::stof(strValue);
			else if (varType.compare("TerrainDepth") == 0)
				terrainDepth = std::stof(strValue);
			else if (varType.compare("TerrainHeight") == 0)
				terrainHeight = std::stof(strValue);
			//else if (varType.compare("terrainPath") == 0)
			//{
			//	terrainPath = strValue;
			//}
			else if (varType.compare("Mesh") == 0)
			{
				std::vector<std::string> resources;
				std::string fileName;
				for (int i = 0; i < 4; i++) 
				{
					ifs >> fileName;
					resources.push_back(fileName);
				}
				meshes.insert(std::pair<std::string, std::vector<std::string>>(strValue, resources));
			}
			else if (varType.compare("CockpitObject") == 0) 
			{
				std::string tmp;
				float objectInfos[7];
				for (int i = 0; i < 7; i++) 
				{
					ifs >> tmp;
					objectInfos[i] = std::stof(tmp);
				}
				CockpitObject obj(strValue, objectInfos[0], objectInfos[1], objectInfos[2], 
					objectInfos[3], objectInfos[4], objectInfos[5], objectInfos[6]);
				cockpitObjects.push_back(obj);
			}
			else if (varType.compare("GroundObject") == 0)
			{
				std::string tmp;
				float objectInfos[7];
				for (int i = 0; i < 7; i++)
				{
					ifs >> tmp;
					objectInfos[i] = std::stof(tmp);
				}
				GroundObject obj(strValue, objectInfos[0], objectInfos[1], objectInfos[2],
					objectInfos[3], objectInfos[4], objectInfos[5], objectInfos[6]);
				groundObjects.push_back(obj);
			}
			else if (varType.compare("EnemyType") == 0) 
			{
				std::string mesh;
				std::string tmp;
				float infos[10];
				for (int i = 0; i < 11; i++) 
				{
					if (i == 3)
						ifs >> mesh;
					else 
					{
						ifs >> tmp;
						if (i > 3)
							infos[i - 1] = std::stof(tmp);
						else
							infos[i] = std::stof(tmp);
					}
				}
				EnemyType obj(mesh, infos[0], infos[1], infos[2], infos[3], infos[4], infos[5], infos[6], infos[7], infos[8], infos[9]);
				enemyTypes[strValue] = obj;
				enemyTypesList.push_back(strValue);
			}
			else if (varType.compare("Spawn") == 0) 
			{
				float time, min, max;
				time = std::stof(strValue);
				ifs >> strValue;
				min = std::stof(strValue);
				ifs >> strValue;
				max = std::stof(strValue);
				spawnInfo.intervall = time;
				spawnInfo.minHeight = min;
				spawnInfo.maxHeight = max;
			}
			else if (varType.compare("Sprite") == 0) 
			{
				// conversion from codeguru: http://forums.codeguru.com/showthread.php?193852-How-to-convert-string-to-wstring
				std::wstring path(strValue.length(), L' ');
				std::copy(strValue.begin(), strValue.end(), path.begin());
				sprites.push_back(path);
			}
			else if (varType.compare("GatlingGun") == 0 || varType.compare("PlasmaGun") == 0) 
			{
				float infos[9];
				infos[0] = std::stof(strValue);
				for (int i = 1; i < 9; i++) 
				{
					ifs >> strValue;
					infos[i] = std::stof(strValue);
				}
				Gun gun(infos);
				if (varType.compare("GatlingGun") == 0)
					gatling = gun;
				else
					plasma = gun;
			}
			else if (varType.compare("Explosion") == 0) 
			{
				float lifetime = std::stof(strValue);
				ifs >> strValue;
				int texIndex = (int) std::stof(strValue);
				explosion = { lifetime, texIndex };
			}
			else if (varType.compare("ExplosionParticle") == 0) 
			{
				float infos[6];
				infos[0] = std::stof(strValue);
				for (int i = 1; i < 6; i++) 
				{
					ifs >> strValue;
					infos[i] = std::stof(strValue);
				}
				ExplosionParticle p = { infos[0], infos[1], infos[2], infos[3], infos[4], infos[5] };
				explosionParticle = p;
			}
			else
			{
				std::cout << "Unknown variable type: " << varType;
				ifs.close();
				return;
			}
		}
		ifs.close();
	}
	else
		std::cout << "Could not find the file: " << filename << "\n";
}

float ConfigParser::getSpinning() { return spinning; }
//float ConfigParser::getSpinSpeed() { return spinSpeed; }
//Color ConfigParser::getBackgroundColor() { return backgroundColor; }
//std::string ConfigParser::getTerrainPath() { return terrainPath; }
std::string ConfigParser::getPathHeight() { return pathHeight; }
std::string ConfigParser::getPathColor() { return pathColor; }
std::string ConfigParser::getPathNormal() { return pathNormal; }
float ConfigParser::getTerrainWidth() { return terrainWidth; }
float ConfigParser::getTerrainDepth() { return terrainDepth; }
float ConfigParser::getTerrainHeight() { return terrainHeight; }
std::map<std::string, std::vector<std::string>> ConfigParser::getMeshes() { return meshes; }
std::vector<CockpitObject> ConfigParser::getCockpitObjects() { return cockpitObjects; }
std::vector<GroundObject> ConfigParser::getGroundObjects() { return groundObjects; }
std::map<std::string, EnemyType> ConfigParser::getEnemyTypes() { return enemyTypes; }
std::vector<std::string> ConfigParser::getEnemyTypesList() { return enemyTypesList; }
Spawn ConfigParser::getSpawnInfo() { return spawnInfo; }
std::vector<std::wstring> ConfigParser::getSprites() { return sprites; }
Gun ConfigParser::getGatling() { return gatling; }
Gun ConfigParser::getPlasma() { return plasma; }
Explosion ConfigParser::getExplosion() { return explosion; }
ExplosionParticle ConfigParser::getExplosionParticle() { return explosionParticle; }

