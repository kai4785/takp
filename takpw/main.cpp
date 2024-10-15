#include <iostream>

#include "d3d8.h"

int main()
{
    std::cout << "Hello World!\n" << std::endl;
    auto d8 = Direct3DCreate8(220);
    std::cout << "Got a d3d thing: " << d8 << std::endl;

}
