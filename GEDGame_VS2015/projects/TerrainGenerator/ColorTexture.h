#pragma once

#include "stdafx.h"
#include <string>
#include <SimpleImage.h>

struct Vec3float
{
	float x, y, z;
	Vec3float(float a, float b, float c) : x(a), y(b), z(c) {};
	Vec3float() : x(0), y(0), z(0) {};
};

inline int access2DArray(int x, int y, int w)
{
	return x + y * w;
}

class ColorTexture
{
public:
	ColorTexture(const std::wstring file_path);
	~ColorTexture();
	Vec3float* getIMG();
	const int &getWidth();
	const int &getHeight();
	Vec3float &getTiledColor(int u, int v);

private:
	GEDUtils::SimpleImage img;
	std::wstring path;
	int width;
	int height;
	Vec3float* imgMap;
};