#include "ColorBlender.h"
#include "ColorTexture.h"
#include <iostream>
#include <math.h>


ColorBlender::ColorBlender(ColorTexture& grass, ColorTexture& dirt, ColorTexture& pebble,
	ColorTexture& rock, float* heightfield, Vec3float* normalfield, UINT width, UINT height) : colorMap(width - 1 , height - 1)
{
	this->width = width;
	this->height = height;
	for (int i = 0; i < width - 1; i++)
	{
		for (int l = 0; l < height - 1; l++)
		{
			Vec3float colGrass = grass.getTiledColor(i, l);
			Vec3float colDirt = dirt.getTiledColor(i, l);
			Vec3float colPebble = pebble.getTiledColor(i, l);
			Vec3float colRock = rock.getTiledColor(i, l);

			float alpha1, alpha2, alpha3;

			// slope implementation that is described in the slides, should be used with simpleCalculateAlphas()
			float simpleSlope = 1.0f - normalfield[access2DArray(i, l, width)].z;
			simpleCalculateAlphas(simpleSlope, heightfield[access2DArray(i, l, width)], alpha1, alpha2, alpha3);

			// a different slope-formula, but similar to 1 - n, is much more realistic, should be used with calculateAlpha(), just comment out to overwrite
			//float slope = (1.0f - normalfield[access2DArray(i, l, width)].z)
			//	/ sqrt(pow(normalfield[access2DArray(i, l, width)].x, 2) + pow(normalfield[access2DArray(i, l, width)].y, 2));
			//calculateAlphas(slope, heightfield[access2DArray(i, l, width)], alpha1, alpha2, alpha3);

			// remember: alpha0 = 1.0f
			Vec3float finalCol = calculateColor(colGrass, colDirt, colPebble, colRock, alpha1, alpha2, alpha3);
			colorMap.setPixel(i, l, finalCol.x, finalCol.y, finalCol.z);
		}
	}

}

GEDUtils::SimpleImage &ColorBlender::getIMG() { return colorMap; }

void ColorBlender::calculateAlphas(float slope, float height, float& alpha1, float& alpha2, float& alpha3)
{
	alpha1 = (1.0f - height) * slope;
	// for pebbles a pre-factor is used to make result better, can be removed but makes the result too pebbled :D
	alpha2 = 0.7f * height * (1.0f - slope);
	alpha3 = height * slope;
	// alpha values too low, alpha0 wins
	if (alpha1 <= 0.2f && alpha2 <= 0.2f && alpha3 <= 0.2f) {
		return;
	}
	// find the winning alpha-and reward it
	if (alpha1 >= alpha2 || alpha1 >= alpha3) {
		alpha1 *= 2.0f;
		if (alpha1 >= 1.0f)
			alpha1 = 1.0f;
	}
	else if (alpha2 >= alpha1 || alpha2 >= alpha3) {
		alpha2 *= 2.0f;
		if (alpha2 >= 1.0f)
			alpha2 = 1.0f;
	}
	else if (alpha3 >= alpha1 || alpha3 >= alpha2) {
		alpha3 *= 2.0f;
		if (alpha3 >= 1.0f)
			alpha3 = 1.0f;
	}
}

void ColorBlender::simpleCalculateAlphas(float slope, float height, float& alpha1, float& alpha2, float& alpha3)
{
	alpha3 = (1 - height) * slope;
	alpha2 = height * slope;
	alpha1 = height;
}

Vec3float ColorBlender::calculateColor(Vec3float col0, Vec3float col1, Vec3float col2, Vec3float col3, float alpha1, float alpha2, float alpha3)
{
	Vec3float blended;
	blended.x = alpha3 * col3.x + (1 - alpha3) * (alpha2 * col2.x + (1 - alpha2) * (alpha1 * col1.x + (1 - alpha1) * col0.x));
	blended.y = alpha3 * col3.y + (1 - alpha3) * (alpha2 * col2.y + (1 - alpha2) * (alpha1 * col1.y + (1 - alpha1) * col0.y));
	blended.z = alpha3 * col3.z + (1 - alpha3) * (alpha2 * col2.z + (1 - alpha2) * (alpha1 * col1.z + (1 - alpha1) * col0.z));
	return blended;
}