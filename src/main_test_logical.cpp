#include <iostream>
#include "i960.h"
#include "memory.h"

int main(int argc, char* argv[]) {
    std::cout << "Testing new i960 logical instructions..." << std::endl;

    // Initialize memory bus
    MemoryBus bus;
    memory_init(&bus);

    // Test program with logical instructions
    uint8_t test_program[] = {
        // ld_const g0, 0x0000000F (15 = 1111b)
        0x90, 0x00, 0x00, 0x00, 0x00, 0x0F,
        // ld_const g1, 0x000000F0 (240 = 11110000b)
        0x90, 0x01, 0x00, 0x00, 0x00, 0xF0,
        // and_reg g2, g0, g1 (g2 = 15 & 240 = 0)
        0x5B, 0x02, 0x00, 0x01,
        // or_reg g3, g0, g1 (g3 = 15 | 240 = 255)
        0x5C, 0x03, 0x00, 0x01,
        // xor_reg g4, g0, g1 (g4 = 15 ^ 240 = 255)
        0x5D, 0x04, 0x00, 0x01,
        // not_reg g5, g0 (g5 = ~15 = 0xFFFFFFF0)
        0x5E, 0x05, 0x00,
        // cmp_reg g0, g0 (compare g0 with itself - should set zero_flag)
        0x5F, 0x00, 0x00,
        // cmp_reg g0, g1 (compare g0 with g1 - should clear zero_flag)
        0x5F, 0x00, 0x01,
        // halt
        0xFF
    };

    for (size_t i = 0; i < sizeof(test_program); ++i) {
        memory_write_byte(&bus, i, test_program[i]);
    }

    // Initialize CPU
    i960_cpu cpu;
    i960_init(&cpu, &bus);

    // Execute test program
    std::cout << "\n=== Executing logical operations test ===" << std::endl;
    bool running = true;
    int steps = 0;
    while (running && steps < 50) {
        uint8_t opcode = memory_read_byte(cpu.bus, cpu.ip);
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
    std::cout << "g0 = 0x" << std::hex << cpu.g[0] << " (should be 0xF)" << std::endl;
    std::cout << "g1 = 0x" << cpu.g[1] << " (should be 0xF0)" << std::endl;
    std::cout << "g2 = 0x" << cpu.g[2] << " (should be 0x0 = 15 & 240)" << std::endl;
    std::cout << "g3 = 0x" << cpu.g[3] << " (should be 0xFF = 15 | 240)" << std::endl;
    std::cout << "g4 = 0x" << cpu.g[4] << " (should be 0xFF = 15 ^ 240)" << std::endl;
    std::cout << "g5 = 0x" << cpu.g[5] << " (should be 0xFFFFFFF0 = ~15)" << std::endl;

    // Cleanup
    memory_destroy(&bus);
    std::cout << "\nLogical instructions test completed!" << std::endl;
    return 0;
}