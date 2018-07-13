#pragma once

#include "stdafx.h"
#include <random>
#include <math.h>
#include <time.h>
#include <iostream>

#define IDX(x, y, w) ((x) + (y) * (w))

class DiamondSquareGenerator 
{
	float* field;
	int resolution;
	float sigma = 1.0f;
	float H = 1.3f;
	std::default_random_engine generator;
	std::normal_distribution<float> distribute;

	float randomDisplacement(float val);
	void setRandomHeight(int x, int y);
	void initialize();
	void diamondStep(int x, int y, int s);
	void squareStep(int x, int y, int s);
	void generate();

public:
	DiamondSquareGenerator(int resolution, float deviation);
	~DiamondSquareGenerator();
	float* getField();
	int getResolution();
	float* generateField();
};
