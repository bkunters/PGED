#include "stdafx.h"
#include <cstdlib>
#include <time.h>
#include <iostream>

#define WIDTH 5
#define HEIGHT 5

// Access a 2D array of width w at position x / y
#define IDX(x, y, w) ((x) + (y) * (w))

void printArray(float* arr, int width, int height) 
{
 	for (int i = 0; i < width; i++) 
 	{
 		for (int l = 0; l < height; l++) 
 		{
 			std::cout << arr[IDX(i, l, width)] << " ";
 		}
 		std::cout << "\n";
 	}
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


int main()
{
	float arr[WIDTH * HEIGHT];
	srand(time(NULL));
	for (int i = 0; i < WIDTH * HEIGHT; i++) 
	{
		arr[i] = (rand() % 10) / 10.0f;
	}
	printArray(arr, WIDTH, HEIGHT);
	smoothArray(arr, WIDTH, HEIGHT);
	std::cout << "--------------------------------\n";
	printArray(arr, WIDTH, HEIGHT);
	system("pause");
    return 0;
}

