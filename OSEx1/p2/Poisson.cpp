#include <iostream>
// #include <cstdlib>
#include <string>
#include <cmath>


int main(int argc, char const *argv[])
{
    //Check if we got 2 valid arguments

    if (argc != 3)
    {
        std::cout << "Error\n" << std::endl;
        return 1;
    } 

    // Calc poisson distribution first arg is lambda, second is k
    long double lambda = atof(argv[1]);
    long double k = atof(argv[2]);

    long double result = (powf(lambda, k) * expf(-lambda)) / tgamma(k+1);

    //print result with long double precision
    std::cout << result << std::endl;

    
    return 0;
}
