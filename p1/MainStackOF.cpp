#include <iostream>

void recursiveFunction(int count) {
    std::cout << count << std::endl;
    recursiveFunction(count + 1); // Recursive call without base case
}

int main() {
    recursiveFunction(1);
    return 0;
}
