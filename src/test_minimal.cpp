#include <iostream>

int main() {
    std::cout << "Starting minimal test..." << std::endl;

    // Test basic operations
    int x = 42;
    std::cout << "Integer assignment works: " << x << std::endl;

    // Test memory allocation
    int* ptr = new int[10];
    std::cout << "Memory allocation works" << std::endl;
    delete[] ptr;

    std::cout << "Minimal test completed successfully" << std::endl;
    return 0;
}