#include "stdafx.h"
#include <iostream>
#include <SimpleImage.h>
#include <TextureGenerator.h>
#include "ColorBlender.h"
#include "ColorTexture.h"
#include "DiamondSquareGenerator.h"

// macro for accessing to a flattened 2D-Array
#define IDX(x, y, w) ((x) + (y) * (w))

void printField(float* arr, int width, int height)
{
	for (int i = 0; i < width; i++)
	{
		for (int l = 0; l < height; l++)
		{
			if (arr[IDX(i, l, width)] < 0.0f)
				std::cout << 0 << " ";
			else
				std::cout << arr[IDX(i, l, width)] << " ";
		}
		std::cout << "\n";
	}
}

// parses the command line and returns the resolution, returns -1 if parse fails
int parseCommandLine(int argc, _TCHAR* argv[], std::wstring& heightFile, 
	std::wstring& colorFile, std::wstring& normalsFile) 
{
	if (argc < 9 || _tcscmp(argv[1], _TEXT("-r")) != 0 || _tcscmp(argv[3], _TEXT("-o_height")) != 0
		|| _tcscmp(argv[5], _TEXT("-o_color")) || _tcscmp(argv[7], _TEXT("-o_normal")) != 0)
	{
		if (argc < 8)
			std::cout << "Amount of parameters are not enough.";
		else if (_tcscmp(argv[1], _TEXT("-r")) != 0)
			std::cout << "Missing Parameter: Resolution\n";
		else if (_tcscmp(argv[3], _TEXT("-o_height")) != 0)
			std::cout << "Missing Parameter: Heightfieldfile\n";
		else if (_tcscmp(argv[5], _TEXT("-o_color")) != 0)
			std::cout << "Missing Parameter: Colorfile\n";
		else
			std::cout << "Missing Parameter: Normalfile\n";
		system("pause");
		return -1;
	}
	int resolution = _tstoi(argv[2]);
	if (resolution <= 0)
	{
		std::cout << "Unvalid Resolution, exiting program...\n";
		system("pause");
		return -1;
	}
	heightFile = argv[4];
	colorFile = argv[6];
	normalsFile = argv[8];
	return resolution;
}

void smoothArray(float* arr, int width, int height)
{
	// Trivial Cases
	if (width == 1 && height == 1)
		return;
	else if (width == 2 && height == 1 || width == 1 && height == 2)
	{
		float avg = (arr[0] + arr[1]) / 2;
		arr[0] = avg;
		arr[1] = avg;
		return;
	}
	// Allocate temporary array to save changes
	float* smoothArr = new float[width * height];
	for (int i = 0; i < width; i++)
	{
		for (int l = 0; l < height; l++)
		{
			// Special Case: Corner Point
			if ((i == 0 || i == width - 1) && (l == 0 || l == height - 1))
			{
				if (i == 0)
				{
					if (l == 0)
						smoothArr[IDX(i, l, width)] = (arr[IDX(i, l, width)] + arr[IDX(i, l + 1, width)]
							+ arr[IDX(i + 1, l, width)] + arr[IDX(i + 1, l + 1, width)]) / 4;
					else
						smoothArr[IDX(i, l, width)] = (arr[IDX(i, l, width)] + arr[IDX(i, l - 1, width)]
							+ arr[IDX(i + 1, l, width)] + arr[IDX(i + 1, l - 1, width)]) / 4;
				}
				else
				{
					if (l == 0)
						smoothArr[IDX(i, l, width)] = (arr[IDX(i, l, width)] + arr[IDX(i, l + 1, width)]
							+ arr[IDX(i - 1, l, width)] + arr[IDX(i - 1, l + 1, width)]) / 4;
					else
						smoothArr[IDX(i, l, width)] = (arr[IDX(i, l, width)] + arr[IDX(i, l - 1, width)]
							+ arr[IDX(i - 1, l, width)] + arr[IDX(i - 1, l - 1, width)]) / 4;
				}
			}
			// Special Case: Point on the Edge of Matrix
			else if (i == 0 || i == width - 1 || l == 0 || l == height - 1)
			{
				if (i == 0)
					smoothArr[IDX(i, l, width)] = (arr[IDX(i, l, width)] + arr[IDX(i + 1, l, width)]
						+ arr[IDX(i, l - 1, width)] + arr[IDX(i, l + 1, width)]
						+ arr[IDX(i + 1, l + 1, width)] + arr[IDX(i + 1, l - 1, width)]) / 6;
				else if (i == width - 1)
					smoothArr[IDX(i, l, width)] = (arr[IDX(i, l, width)] + arr[IDX(i - 1, l, width)]
						+ arr[IDX(i, l - 1, width)] + arr[IDX(i, l + 1, width)]
						+ arr[IDX(i - 1, l + 1, width)] + arr[IDX(i - 1, l - 1, width)]) / 6;
				else if (l == 0)
					smoothArr[IDX(i, l, width)] = (arr[IDX(i, l, width)] + arr[IDX(i - 1, l, width)]
						+ arr[IDX(i + 1, l, width)] + arr[IDX(i, l + 1, width)]
						+ arr[IDX(i - 1, l + 1, width)] + arr[IDX(i + 1, l + 1, width)]) / 6;
				else if (l == height - 1)
					smoothArr[IDX(i, l, width)] = (arr[IDX(i, l, width)] + arr[IDX(i - 1, l, width)]
						+ arr[IDX(i + 1, l, width)] + arr[IDX(i, l - 1, width)]
						+ arr[IDX(i - 1, l - 1, width)] + arr[IDX(i + 1, l - 1, width)]) / 6;
			}
			// Standard Case
			else
			{
				smoothArr[IDX(i, l, width)] = (arr[IDX(i, l, width)] + arr[IDX(i, l + 1, width)] + arr[IDX(i, l - 1, width)]
					+ arr[IDX(i - 1, l, width)] + arr[IDX(i - 1, l - 1, width)] + arr[IDX(i - 1, l + 1, width)]
					+ arr[IDX(i + 1, l, width)] + arr[IDX(i + 1, l - 1, width)] + arr[IDX(i + 1, l + 1, width)]) / 9;
			}
		}
	}
	// copy the changes
	for (int i = 0; i < width; i++)
	{
		for (int l = 0; l < height; l++)
		{
			arr[IDX(i, l, width)] = smoothArr[IDX(i, l, width)];
		}
	}
	// free memory
	delete[] smoothArr;
}

Vec3float* generateNormalMap(const float* heightfield, const int width, const int height)
{
	// allocate
	Vec3float* normalMap = new Vec3float[width * height];
	for (int i = 0; i < width; i++)
	{
		//std::cout << "Normalmapping %" << (i * 100) / width << "\r";
		for (int l = 0; l < height; l++)
		{
			// set b-value
			float z = 1.0f;
			float x, y;

			// calculate x-value
			if (i == 0)
			{
				if (i + 1 >= width)
					x = 0.0;
				else
					x = -((heightfield[access2DArray(i + 1, l, width)] - heightfield[access2DArray(i, l, width)]) / (1.0f / width));
			}
			else if (i == width - 1)
			{
				if (i - 1 < 0)
					x = 0.0;
				else
					x = -((heightfield[access2DArray(i, l, width)] - heightfield[access2DArray(i - 1, l, width)]) / (1.0f / width));
			}
			else
			{
				x = -((heightfield[access2DArray(i + 1, l, width)] - heightfield[access2DArray(i - 1, l, width)]) / (2.0f / width));
			}
			// calculate y-value
			if (l == 0)
			{
				if (l + 1 >= height)
					y = 0.0;
				else
					y = -((heightfield[access2DArray(i, l + 1, width)] - heightfield[access2DArray(i, l, width)]) / (1.0f / height));
			}
			else if (l == height - 1)
			{
				if (l - 1 < 0)
					y = 0.0;
				else
					y = -((heightfield[access2DArray(i, l, width)] - heightfield[access2DArray(i, l - 1, width)]) / (1.0f / height));
			}
			else
			{
				y = -((heightfield[access2DArray(i, l + 1, width)] - heightfield[access2DArray(i, l - 1, width)]) / (2.0f / height));
			}

			// normalize the normal vector
			float length = std::sqrt(x * x + y * y + z * z);
			x = x / length;
			y = y / length;
			z = z / length;
			// save
			Vec3float normal(x, y, z);
			normalMap[access2DArray(i, l, width)] = normal;
		}
	}
	return normalMap;
}

void downsize_map(GEDUtils::SimpleImage &old_image, GEDUtils::SimpleImage &new_image) {
	for (int i = 0; i < old_image.getWidth(); i += 4) {
		//std::cout << "Downsizing %" << (i * 100) / (old_image.getWidth()) << "\r";
		for (int j = 0; j < old_image.getHeight(); j += 4) {
			
			float mean = 0.0f;

			for (int x = i; x < (i + 4); x++) {
				for (int y = j; y < (j + 4); y++) {
					mean += old_image.getPixel(x, y);
				}
			}

			mean = mean / 16;
			new_image.setPixel(i/4, j/4, mean);

		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::wstring heightFile;
	std::wstring colorFile;
	std::wstring normalFile;
	int resolution = parseCommandLine(argc, argv, heightFile, colorFile, normalFile);
	if (resolution == -1)
		return -1;
	std::cout << "Generating the heightfield...\n";
	DiamondSquareGenerator generator(resolution + 1, 1.0f);
	float* height_map = generator.generateField();
	std::cout << "Heightfield generated\n" << "Beginning the smoothing process..." << std::endl;
	for (int i = 0; i < 60; i++) 
	{
		smoothArray(height_map, resolution + 1, resolution + 1);
		// std::cout << "%" << ((i+1)*100)/60 << " Smoothing done.\r";
	}
	std::cout << "Smoothing is completed.\n";
 	GEDUtils::SimpleImage height_map_img(resolution, resolution);
	for (UINT i = 0; i < resolution; i++) 
	{
		for (UINT l = 0; l < resolution; l++) 
		{
			height_map_img.setPixel(i, l, height_map[IDX(i, l, resolution + 1)]);
		}
	}
	
	std::cout << "Generating the color texture and the normal map...\n";
	
	Vec3float* normalMap = generateNormalMap(height_map, resolution + 1, resolution + 1);
	GEDUtils::SimpleImage normalIMG(resolution, resolution);
	for (UINT i = 0; i < resolution; i++)
	{
		for (UINT l = 0; l < resolution; l++)
		{
			// adjust interval [-1;1] to [0,1]
			float r = (normalMap[access2DArray(i, l, resolution + 1)].x + 1.0f) / 2.0f;
			float g = (normalMap[access2DArray(i, l, resolution + 1)].y + 1.0f) / 2.0f;
			float b = (normalMap[access2DArray(i, l, resolution + 1)].z + 1.0f) / 2.0f;
			normalIMG.setPixel(i, l, r, g, b);
		}
	}
	if (!normalIMG.save(normalFile.c_str()))
		throw "Unvalid file path for normal map:";

	std::wstring grass = L"..\\..\\..\\..\\external\\textures\\gras15.jpg";
	std::wstring dirt = L"..\\..\\..\\..\\external\\textures\\mud02.jpg";
	std::wstring pebble = L"..\\..\\..\\..\\external\\textures\\pebble01.jpg";
	std::wstring rock = L"..\\..\\..\\..\\external\\textures\\rock1.jpg";

	ColorTexture texGrass(grass);
	ColorTexture texDirt(dirt);
	ColorTexture texPebble(pebble);
	ColorTexture texRock(rock);

	// Color texture generation. Creates a color map with resolution * resolution, but takes resolution + 1 as argument
	ColorBlender blender(texGrass, texDirt, texPebble, texRock, height_map, normalMap, resolution + 1, resolution + 1);
	GEDUtils::SimpleImage colorIMG = blender.getIMG();
	if (!colorIMG.save(colorFile.c_str()))
		throw "Unvalid file path for color map:";

	// Downsize the heightmap.
	std::cout << "Downsizing the height map..." << std::endl;
	GEDUtils::SimpleImage height_map_new_img(resolution / 4, resolution / 4);
	downsize_map(height_map_img, height_map_new_img);
	std::cout << "Downsizing done." << std::endl;
	if (!height_map_new_img.save(heightFile.c_str()))
		throw "Unvalid file path for heightfield:";

	delete[] normalMap;
	std::cout << "Done.\n";
    return 0;
}



