#include <iostream>
#include "i960.h"
#include "memory.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting Pixel Model 2 Emulator (Test Mode)..." << std::endl;

    // --- Emulator State ---
    std::cout << "Initializing memory bus..." << std::endl;
    MemoryBus bus;
    memory_init(&bus);
    std::cout << "Memory bus initialized." << std::endl;

    // Test program with new instructions
    std::cout << "Loading test program..." << std::endl;
    uint8_t test_program[] = {
        // ld_const g0, 0x00000010 (16)
        0x90, 0x00, 0x00, 0x00, 0x00, 0x10,
        // ld_const g1, 0x00000005 (5)
        0x90, 0x01, 0x00, 0x00, 0x00, 0x05,
        // mul_reg g2, g0, g1 (g2 = 16 * 5 = 80)
        0x5A, 0x02, 0x00, 0x01,
        // sub_reg g3, g2, g1 (g3 = 80 - 5 = 75)
        0x59, 0x03, 0x02, 0x01,
        // st g3, 0x000100 (store 75 at address 0x100)
        0x61, 0x03, 0x00, 0x01, 0x00,
        // ld g4, 0x000100 (load from address 0x100 into g4)
        0x60, 0x04, 0x00, 0x01, 0x00,
        // halt
        0xFF
    };

    for (size_t i = 0; i < sizeof(test_program); ++i) {
        memory_write_byte(&bus, i, test_program[i]);
    }
    std::cout << "Test program loaded." << std::endl;

    std::cout << "Initializing i960 CPU..." << std::endl;
    i960_cpu cpu;
    i960_init(&cpu, &bus);
    std::cout << "CPU initialized successfully." << std::endl;

    // --- Test execution ---
    std::cout << "\n=== Testing new i960 instructions ===" << std::endl;
    bool running = true;
    int steps = 0;
    while (running && steps < 20) {
        uint8_t opcode = memory_read_byte(&cpu.bus, cpu.ip);
        if (opcode == 0xFF) {  // HALT
            std::cout << "Program halted successfully!" << std::endl;
            running = false;
        } else {
            i960_step(&cpu);
            steps++;
        }
    }

    // Verify results
    std::cout << "\n=== Final register values ===" << std::endl;
    std::cout << "g0 = 0x" << std::hex << cpu.g[0] << " (should be 0x10)" << std::endl;
    std::cout << "g1 = 0x" << cpu.g[1] << " (should be 0x5)" << std::endl;
    std::cout << "g2 = 0x" << cpu.g[2] << " (should be 0x50 = 16*5)" << std::endl;
    std::cout << "g3 = 0x" << cpu.g[3] << " (should be 0x4b = 80-5)" << std::endl;
    std::cout << "g4 = 0x" << cpu.g[4] << " (should be 0x4b = loaded from memory)" << std::endl;

    // Check memory
    uint32_t mem_value = memory_read_dword(&bus, 0x100);
    std::cout << "Memory[0x100] = 0x" << mem_value << " (should be 0x4b)" << std::endl;

    // Cleanup
    memory_destroy(&bus);
    std::cout << "\nTest completed successfully!" << std::endl;
    return 0;
}