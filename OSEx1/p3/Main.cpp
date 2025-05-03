#include <iostream>
#include "PoissonMethod.hpp"


int main()
{
    std::cout << Poisson(2,1) << std::endl;
    std::cout << Poisson(2,10) << std::endl;
    std::cout << Poisson(2,2) << std::endl;
    std::cout << Poisson(3,3) << std::endl;
    std::cout << Poisson(100,3) << std::endl;

    return 0;
}
