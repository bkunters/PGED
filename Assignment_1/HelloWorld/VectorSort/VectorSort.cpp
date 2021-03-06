// VectorSort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <algorithm>
#include <iostream>

struct {
	bool operator() (int i, int j) { return (i>j); }
}str;

void print_vec(const std::vector<int> &vec) {
	for (auto it = vec.begin(); it != vec.end(); it++) {
		std::cout << *it << std::endl;
	}
}

int main()
{
	std::vector<int> vec;
	int input;
	while (true) {
		std::cin >> input;
		if (input == 0)	break;

		vec.push_back(input);
	}

	std::sort(vec.begin(), vec.end(), str);
	print_vec(vec);

	system("pause");
    return 0;
}

