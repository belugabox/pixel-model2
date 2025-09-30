#include <iostream>
#include "memory.h"

int main() {
    std::cout << "Testing memory_init..." << std::endl;
    MemoryBus bus;
    memory_init(&bus);
    std::cout << "memory_init completed successfully" << std::endl;

    memory_destroy(&bus);
    std::cout << "Test completed successfully" << std::endl;
    return 0;
}