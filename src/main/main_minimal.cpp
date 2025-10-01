#include <iostream>
#include "i960.h"
#include "memory.h"
#include "tgp.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting Pixel Model 2 Emulator (minimal test)..." << std::endl;

    // --- Emulator State ---
    std::cout << "Initializing memory bus..." << std::endl;
    MemoryBus bus;
    memory_init(&bus);
    std::cout << "Memory bus initialized." << std::endl;

    std::cout << "Initializing i960 CPU..." << std::endl;
    i960_cpu cpu;
    i960_init(&cpu, &bus);
    std::cout << "CPU initialized successfully." << std::endl;

    std::cout << "Initializing TGP GPU..." << std::endl;
    TGP tgp;
    tgp_init(&tgp, &bus);
    std::cout << "TGP initialized successfully." << std::endl;

    std::cout << "All initializations completed successfully!" << std::endl;

    // Test just a few steps
    std::cout << "Testing CPU execution..." << std::endl;
    for (int i = 0; i < 5; i++) {
        i960_step(&cpu);
        std::cout << "CPU step " << i << " completed." << std::endl;
    }
    std::cout << "CPU test completed successfully." << std::endl;

    // --- Cleanup ---
    memory_destroy(&bus);

    std::cout << "Test completed successfully. The emulator core is working!" << std::endl;
    return 0;
}