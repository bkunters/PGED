#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

struct Color
{
	float r, g, b;
};

struct Spawn 
{
	float intervall, minHeight, maxHeight;
};

struct CockpitObject 
{
	std::string name;
	float scale, rotX, rotY, rotZ, transX, transY, transZ;
	CockpitObject(std::string n, float s, float rX, float rY, float rZ, float tX, float tY, float tZ) 
	{
		name = n; scale = s; rotX = rX; rotY = rY; rotZ = rZ; transX = tX; transY = tY; transZ = tZ;
	};
};

struct GroundObject
{
	std::string name;
	float scale, rotX, rotY, rotZ, transX, transY, transZ;
	GroundObject(std::string n, float s, float rX, float rY, float rZ, float tX, float tY, float tZ)
	{
		name = n; scale = s; rotX = rX; rotY = rY; rotZ = rZ; transX = tX; transY = tY; transZ = tZ;
	};
};

struct EnemyType
{
	std::string mesh;
	int hitpoints;
	float speed, size;
	float scale, rotX, rotY, rotZ, transX, transY, transZ;
	EnemyType(std::string m, int h, float spd, float sze, float s, float rX, 
		float rY, float rZ, float tX, float tY, float tZ)
	{
		mesh = m; hitpoints = h; speed = spd; size = sze; scale = s; rotX = rX; 
		rotY = rY; rotZ = rZ; transX = tX; transY = tY; transZ = tZ;
	};
	EnemyType() {};
};

struct Gun
{
	float x, y, z;
	float speed, gravity, cooldown;
	float damage, sprite_tex_index, sprite_radius;
	bool firing;

	Gun() {};

	Gun(float infos[]) 
	{
		firing = false;
		x = infos[0]; y = infos[1]; z = infos[2]; speed = infos[3]; gravity = infos[4]; cooldown = infos[5]; 
		damage = infos[6]; sprite_tex_index = infos[7]; sprite_radius = infos[8];
	};
	
	void setFiring(bool state) 
	{
		firing = state;
		if (state)
			std::cout << "1\n";
		else
			std::cout << "0\n";
	};
};

struct Explosion 
{
	float lifetime;
	int texIndex;
};

struct ExplosionParticle 
{
	float amount;
	float lifetime;
	float min;
	float max;
	float size;
	float gravityInfluence;
};

class ConfigParser
{
public:
	ConfigParser();
	~ConfigParser();

	void load(std::string filename);

	std::string toString() 
	{
		std::ostringstream output;
		output << "terrainWidth: " << terrainWidth << "\n" << "terrainDepth: " << terrainDepth
			<< "\n" << "terrainHeight: " << terrainHeight << "\n\n" << "heightPath: " << pathHeight
			<< "\n" << "colorPath: " << pathColor << "\n" << "normalPath: " << pathNormal << "\n" << "spinning: " << spinning << "\n\n";
		return output.str();
	};

	float getSpinning();
	//float getSpinSpeed();
	//Color getBackgroundColor();
	//std::string getTerrainPath();
	float getTerrainWidth();
	float getTerrainDepth();
	float getTerrainHeight();
	std::string getPathHeight();
	std::string getPathColor();
	std::string getPathNormal();
	std::map<std::string, std::vector<std::string>> getMeshes();
	std::vector<CockpitObject> getCockpitObjects();
	std::vector<GroundObject> getGroundObjects();
	std::map<std::string, EnemyType> getEnemyTypes();
	std::vector<std::string> getEnemyTypesList();
	Spawn getSpawnInfo();
	std::vector<std::wstring> getSprites();
	Gun getGatling();
	Gun getPlasma();
	Explosion getExplosion();
	ExplosionParticle getExplosionParticle();

private:
	float spinning;
	std::string pathHeight;
	std::string pathColor;
	std::string pathNormal;
	float terrainWidth;
	float terrainDepth;
	float terrainHeight;
	std::map<std::string, std::vector<std::string>> meshes;
	std::vector<CockpitObject> cockpitObjects;
	std::vector<GroundObject> groundObjects;
	std::map<std::string, EnemyType> enemyTypes;
	std::vector<std::string> enemyTypesList;
	Spawn spawnInfo;
	std::vector<std::wstring> sprites;
	Gun gatling;
	Gun plasma;
	Explosion explosion;
	ExplosionParticle explosionParticle;

	//float spinSpeed;
	//Color backgroundColor;
	//std::string terrainPath;
};

