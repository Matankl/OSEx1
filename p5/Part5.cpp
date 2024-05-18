// main.cpp

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <climits>
#include "Part5.hpp"

// Function to generate a random array
std::vector<int> generateRandomArray(int size, int seed) {
    srand(seed);
    std::vector<int> arr(size);
    for (int i = 0; i < size; ++i) {
        arr[i] = rand() % 100 - 25; // Random number in the range (-25, 74)
    }
    return arr;
}

// O(n^3) algorithm
int maxSubarraySumCubic(int seed, int size) {
    std::vector<int> arr = generateRandomArray(size, seed);
    int maxSum = INT_MIN;
    for (int i = 0; i < size; ++i) {
        for (int j = i; j < size; ++j) {
            int sum = 0;
            for (int k = i; k <= j; ++k) {
                sum += arr[k];
            }
            if (sum > maxSum) {
                maxSum = sum;
            }
        }
    }
    return maxSum;
}

// O(n^2) algorithm
int maxSubarraySumQuadratic(int seed, int size) {
    std::vector<int> arr = generateRandomArray(size, seed);
    int maxSum = INT_MIN;
    for (int i = 0; i < size; ++i) {
        int sum = 0;
        for (int j = i; j < size; ++j) {
            sum += arr[j];
            if (sum > maxSum) {
                maxSum = sum;
            }
        }
    }
    return maxSum;
}

// O(n) algorithm (Kadane's Algorithm)
int maxSubarraySumLinear(int seed, int size) {
    std::vector<int> arr = generateRandomArray(size, seed);
    int maxSum = arr[0];
    int currentSum = arr[0];
    for (int i = 1; i < size; ++i) {
        currentSum = std::max(arr[i], currentSum + arr[i]);
        maxSum = std::max(maxSum, currentSum);
    }
    return maxSum;
}

int main() {
    int seed = static_cast<int>(time(0));

    for (int size : {100, 1000, 10000}) {
        std::cout << "Size: " << size << std::endl;

        std::cout << "Max Subarray Sum (O(n^3)): " << maxSubarraySumCubic(seed, size) << std::endl;
        std::cout << "Max Subarray Sum (O(n^2)): " << maxSubarraySumQuadratic(seed, size) << std::endl;
        std::cout << "Max Subarray Sum (O(n)): " << maxSubarraySumLinear(seed, size) << std::endl;

        std::cout << "-----------------------------------------" << std::endl;
    }

    return 0;
}
