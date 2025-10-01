#include <iostream>
#include "i960.h"
#include "memory.h"

int main(int argc, char* argv[]) {
    std::cout << "Testing i960 interrupt system..." << std::endl;

    // Initialize memory bus
    MemoryBus bus;
    memory_init(&bus);

    // Test program with interrupt handling
    uint8_t test_program[] = {
        // Set up interrupt vector 0x10 to point to interrupt handler at address 0x100
        // (This would normally be done by the OS, here we simulate it)

        // ld_const g0, 0x00000042 (66)
        0x90, 0x00, 0x00, 0x00, 0x00, 0x42,
        // ld_const g1, 0x00000001 (1)
        0x90, 0x01, 0x00, 0x00, 0x00, 0x01,
        // add_reg g2, g0, g1 (g2 = 66 + 1 = 67)
        0x58, 0x02, 0x00, 0x01,
        // int 0x10 (trigger interrupt vector 0x10)
        0xA0, 0x10,
        // After interrupt returns, g2 should be 100
        // ld_const g3, 0x00000001 (1)
        0x90, 0x03, 0x00, 0x00, 0x00, 0x01,
        // halt
        0xFF,

        // Interrupt handler at address 0x100
        // ld_const g2, 0x00000064 (100) - modify g2 during interrupt
        0x90, 0x02, 0x00, 0x00, 0x00, 0x64,
        // reti (return from interrupt)
        0xA1
    };

    // Load the program
    for (size_t i = 0; i < sizeof(test_program); ++i) {
        memory_write_byte(&bus, i, test_program[i]);
    }

    // Initialize CPU
    i960_cpu cpu;
    i960_init(&cpu, &bus);

    // Set up interrupt vector 0x10 to point to address 0x100
    i960_set_interrupt_vector(&cpu, 0x10, 0x100);

    // Execute test program
    std::cout << "\n=== Executing interrupt test ===" << std::endl;
    bool running = true;
    int steps = 0;
    while (running && steps < 100) {
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
    std::cout << "g0 = 0x" << std::hex << cpu.g[0] << " (should be 0x42)" << std::endl;
    std::cout << "g1 = 0x" << cpu.g[1] << " (should be 0x1)" << std::endl;
    std::cout << "g2 = 0x" << cpu.g[2] << " (should be 0x64 = 100, modified by interrupt)" << std::endl;
    std::cout << "g3 = 0x" << cpu.g[3] << " (should be 0x1)" << std::endl;

    // Cleanup
    memory_destroy(&bus);
    std::cout << "\nInterrupt system test completed!" << std::endl;
    return 0;
}