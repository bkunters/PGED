#include "ColorTexture.h"
#include "SimpleImage.h"

#pragma once

class ColorBlender
{
public:
	// todo: std::vector<ColorTexture> for in-game use over multiple objects.
	ColorBlender(ColorTexture& grass, ColorTexture& dirt, ColorTexture& pebble, ColorTexture& rock,
		float* heightfield, Vec3float* normalfield, UINT width, UINT height);
	GEDUtils::SimpleImage &getIMG();

private:
	GEDUtils::SimpleImage colorMap;
	// alphas length must be slopeDegree * heightDegree
	// index-number also defines the texture order
	void calculateAlphas(float height, float slope, float& alpha1, float& alpha2, float&alpha3);
	void simpleCalculateAlphas(float slope, float height, float& alpha1, float& alpha2, float& alpha3);
	Vec3float calculateColor(Vec3float col0, Vec3float col1, Vec3float col2, Vec3float col3, float alpha1, float alpha2, float alpha3);
	int width;
	int height;
};