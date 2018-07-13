#include "ColorTexture.h"
#include <iostream>

ColorTexture::ColorTexture(const std::wstring file_path) : img(file_path.c_str())
{
	path = file_path;
	width = img.getWidth();
	height = img.getHeight();
	imgMap = new Vec3float[width * height];
	for (int i = 0; i < width; i++)
	{
		for (int l = 0; l < height; l++)
		{
			Vec3float color;
			img.getPixel(i, l, color.x, color.y, color.z);
			imgMap[access2DArray(i, l, width)] = color;
		}
	}
}

Vec3float &ColorTexture::getTiledColor(int u, int v)
{
	return imgMap[access2DArray(u % width, v % height, width)];
}

ColorTexture::~ColorTexture()
{
	delete[] imgMap;
}
Vec3float* ColorTexture::getIMG() { return imgMap; }
const int &ColorTexture::getHeight() { return height; }
const int &ColorTexture::getWidth() { return width; }