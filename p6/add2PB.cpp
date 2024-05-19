#include <iostream>
#include <fstream>
#include <cstdlib>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <name> <phone>" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Open the phonebook file
    std::ofstream file("phonebook.txt", std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open phonebook.txt" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Write the entry to the phonebook
    file << argv[1] << ", " << argv[2] << std::endl;

    // Close the file
    file.close();
    return 0;
}
