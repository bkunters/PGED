#include "DiamondSquareGenerator.h"

DiamondSquareGenerator::DiamondSquareGenerator(int resolution, float deviation) 
{
	if (resolution <= 0.0f || deviation <= 0.0f)
		throw("Deviation and resolution cannot be equal to or smaller than zero!");
	this->resolution = resolution;
	distribute = std::normal_distribution<float>(0.0f, deviation);
	//generator = std::default_random_engine(time(NULL));
	generator = std::default_random_engine(1.0f); // fixed-seed
	field = new float[(resolution) * (resolution)];
	initialize();
}

DiamondSquareGenerator::~DiamondSquareGenerator() 
{
	delete[] field;
}

float* DiamondSquareGenerator::getField() { return field; }
int DiamondSquareGenerator::getResolution() { return resolution; }

float* DiamondSquareGenerator::generateField()
{
	generate();
	return field;
}

float DiamondSquareGenerator::randomDisplacement(float val)
{
	float result = val + distribute(generator) * sigma;
	while (result < 0.0f || result > 1.0f) {
		result = val + distribute(generator) * sigma;
	}
	return result;
}

void DiamondSquareGenerator::setRandomHeight(int x, int y)
{
	field[IDX(x, y, resolution)] = distribute(generator);
	while (field[IDX(x, y, resolution)] < 0.0f || field[IDX(x, y, resolution)] > 1.0f)
		field[IDX(x, y, resolution)] = distribute(generator);
	std::cout << "x: " << x << " y: " << y << " init val: " << field[IDX(x, y, resolution)] << "\n";
}

void DiamondSquareGenerator::initialize()
{
	setRandomHeight(0, 0);
	setRandomHeight(0, resolution - 1);
	setRandomHeight(resolution - 1, 0);
	setRandomHeight(resolution - 1, resolution - 1);
}

void DiamondSquareGenerator::diamondStep(int x, int y, int distance)
{
	int count = 0;
	float sum = 0.0f;

	if (x - distance >= 0)
	{
		if (y - distance >= 0)
		{
			sum += field[IDX(x - distance, y - distance, resolution)];
			count++;
		}
		if (y + distance < resolution)
		{
			sum += field[IDX(x - distance, y + distance, resolution)];
			count++;
		}
	}
	if (x + distance < resolution)
	{
		if (y - distance >= 0)
		{
			sum += field[IDX(x + distance, y - distance, resolution)];
			count++;
		}
		if (y + distance < resolution)
		{
			sum += field[IDX(x + distance, y + distance, resolution)];
			count++;
		}
	}
	float avg = sum / (count * 1.0f);
	field[IDX(x, y, resolution)] = randomDisplacement(avg);
}

void DiamondSquareGenerator::squareStep(int x, int y, int distance)
{
	int count = 0;
	float sum = 0.0f;
	if (x - distance >= 0)
	{
		sum += field[IDX(x - distance, y, resolution)];
		count++;
	}
	if (x + distance < resolution)
	{
		sum += field[IDX(x + distance, y, resolution)];
		count++;
	}
	if (y - distance >= 0)
	{
		sum += field[IDX(x, y - distance, resolution)];
		count++;
	}
	if (y + distance < resolution)
	{
		sum += field[IDX(x, y + distance, resolution)];
		count++;
	}
	float avg = sum / (count * 1.0f);
	field[IDX(x, y, resolution)] = randomDisplacement(avg);
}

void DiamondSquareGenerator::generate()
{
	int r = resolution;
	for (int s = r/2; s >= 1; s /= 2)
	{
		for (int y = s; y < r; y += 2 * s) 
		{			
			for (int x = s; x < r; x += 2 * s) 
			{
				diamondStep(x, y, s);
			}
		}
		for (int y = s; y < r; y += 2 * s) 
		{
			for (int x = s; x < r; x += 2 * s)
			{
				squareStep(x - s, y, s);
				squareStep(x, y - s, s);
				squareStep(x + s, y, s);
				squareStep(x, y + s, s);
			}
		}		
		sigma /= pow(2.0f, H);
	}
}

