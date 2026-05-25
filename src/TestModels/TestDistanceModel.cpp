#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <torch/torch.h>
#include <chrono>
#include <algorithm>
#include <cmath>

int count_x(const char* p, char x)
// count the number of occurrences of x in p[]
// p is assumed to point to a zero-terminated array of char (or to nothing)
{
    if (p == nullptr)
        return 0;

    int count = 0;

    for (; *p != '\0'; ++p)
        if (*p == x)
            ++count;

    return count;
}

int main(){
	const char* p = "hello world";
    	int result = count_x(p, 'o');
	std::cout<<result<<std::endl;
	return 0;
}

