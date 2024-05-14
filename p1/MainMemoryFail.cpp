#include <iostream>

int main() {
    //accessing unlocated memory using array index
    int arr[5] = {1, 2, 3, 4, 5};
    std::cout << arr[6] << std::endl;

    return 0;
}
