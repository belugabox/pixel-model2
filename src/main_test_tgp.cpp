#include <iostream>
#include "i960.h"
#include "memory.h"
#include "tgp.h"

int main(int argc, char* argv[]) {
    std::cout << "Testing TGP integration..." << std::endl;

    // Initialize memory bus
    MemoryBus bus;
    memory_init(&bus);

    // Initialize TGP
    TGP tgp;
    tgp_init(&tgp, &bus);

    // Connect TGP to memory bus
    memory_connect_tgp(&bus, &tgp);

    // Initialize CPU
    i960_cpu cpu;
    i960_init(&cpu, &bus);

    // Test program: Write to TGP registers via CPU
    uint8_t test_program[] = {
        // ld_const g0, 0xC0000004 (TGP vertex buffer register address)
        0x90, 0x00, 0xC0, 0x00, 0x00, 0x04,
        // ld_const g1, 0x00100000 (vertex buffer address = 0x100000)
        0x90, 0x01, 0x00, 0x10, 0x00, 0x00,
        // st g1, g0 (write vertex buffer address to TGP register)
        0x61, 0x01, 0x00, 0x00,

        // ld_const g0, 0xC0000008 (TGP index buffer register address)
        0x90, 0x00, 0xC0, 0x00, 0x00, 0x08,
        // ld_const g1, 0x00200000 (index buffer address = 0x200000)
        0x90, 0x01, 0x00, 0x20, 0x00, 0x00,
        // st g1, g0 (write index buffer address to TGP register)
        0x61, 0x01, 0x00, 0x00,

        // ld_const g0, 0xC0000000 (TGP control register address)
        0x90, 0x00, 0xC0, 0x00, 0x00, 0x00,
        // ld_const g1, 0x00000001 (start command)
        0x90, 0x01, 0x00, 0x00, 0x00, 0x01,
        // st g1, g0 (start TGP command)
        0x61, 0x01, 0x00, 0x00,

        // halt
        0xFF
    };

    // Load the program
    for (size_t i = 0; i < sizeof(test_program); ++i) {
        memory_write_byte(&bus, i, test_program[i]);
    }

    // Execute test program
    std::cout << "\n=== Executing TGP integration test ===" << std::endl;
    bool running = true;
    int steps = 0;
    while (running && steps < 50) {
        uint8_t opcode = memory_read_byte(cpu.bus, cpu.ip);
        if (opcode == 0xFF) {  // HALT
            std::cout << "Program halted successfully!" << std::endl;
            running = false;
        } else {
            i960_step(&cpu);
            // Also step the TGP
            tgp_step(&tgp);
            steps++;
        }
    }

    // Verify TGP state
    std::cout << "\n=== TGP Register Values ===" << std::endl;
    std::cout << "Control Register = 0x" << std::hex << tgp.control_register << std::endl;
    std::cout << "Vertex Buffer Addr = 0x" << tgp.vertex_buffer_addr << std::endl;
    std::cout << "Index Buffer Addr = 0x" << tgp.index_buffer_addr << std::endl;
    std::cout << "TGP Busy = " << (tgp.busy ? "true" : "false") << std::endl;

    // Cleanup
    memory_destroy(&bus);
    std::cout << "\nTGP integration test completed!" << std::endl;
    return 0;
}