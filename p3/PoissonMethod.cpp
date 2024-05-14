#include <iostream>
// #include <cstdlib>
#include <string>
#include <cmath>
#include "PoissonMethod.hpp"
#include <libpoisson.so>





 long double Poisson(long double lambda, long double k){
    // Calc poisson distribution 
    long double result = (powf(lambda, k) * expf(-lambda)) / tgamma(k+1);
    return result;
 }