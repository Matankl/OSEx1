#include <iostream>

int main() {
    //
    int* ptr = nullptr;
    *ptr = 10; // Accessing memory through a null pointer
    std::cout << *ptr << std::endl;
    return 0;
}
