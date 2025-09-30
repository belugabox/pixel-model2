#include "i960.h"
#include <iostream>

void i960_init(i960_cpu* cpu, MemoryBus* bus) {
    // On power-up, clear all global registers
    for (int i = 0; i < 16; ++i) {
        cpu->g[i] = 0;
    }

    // Set the instruction pointer to the reset vector address.
    // This address will need to be determined from hardware docs,
    // for now, we'll set it to 0.
    cpu->ip = 0x00000000;
    cpu->bus = bus; // Connect to the bus
    cpu->zero_flag = false; // Initialize flags

    // Initialize the stack pointer to the top of memory.
    // The stack grows downwards.
    cpu->sp = MEMORY_SIZE;

    // Initialize interrupt system
    cpu->interrupt_mode = false;
    cpu->interrupt_sp = 0;
    for (int i = 0; i < 256; ++i) {
        cpu->interrupt_vectors[i] = 0; // No handlers by default
    }

    std::cout << "i960 CPU Initialized and connected to Memory Bus." << std::endl;
}

void i960_step(i960_cpu* cpu) {
    // Fetch the opcode from memory
    uint8_t opcode = memory_read_byte(cpu->bus, cpu->ip);

    // Decode and execute the instruction
    switch (opcode) {
        case 0x90: { // Our custom ld_const instruction
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);

            uint32_t val = 0;
            val |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 2) << 24;
            val |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 3) << 16;
            val |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 4) << 8;
            val |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 5);

            if (dst_reg < 16) {
                cpu->g[dst_reg] = val;
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": ld_const -> Loaded 0x" << val << " into g" << std::dec << (int)dst_reg << std::endl;
            }

            cpu->ip += 6; // This instruction is 6 bytes long
            break;
        }

        case 0x58: { // Our custom add_reg instruction
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t src1_reg = memory_read_byte(cpu->bus, base_ip + 2);
            uint8_t src2_reg = memory_read_byte(cpu->bus, base_ip + 3);

            if (dst_reg < 16 && src1_reg < 16 && src2_reg < 16) {
                uint32_t val1 = cpu->g[src1_reg];
                uint32_t val2 = cpu->g[src2_reg];
                cpu->g[dst_reg] = val1 + val2;

                // Update zero flag based on the result
                cpu->zero_flag = (cpu->g[dst_reg] == 0);

                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": add_reg -> g" << std::dec << (int)dst_reg << " = 0x" 
                          << std::hex << val1 << " + 0x" << val2 << " = 0x" << cpu->g[dst_reg] << std::endl;
            }

            cpu->ip += 4; // This instruction is 4 bytes long
            break;
        }

        case 0xFF: { // halt
            std::cout << "Executing at 0x" << std::hex << cpu->ip << ": HALT" << std::endl;
            // Don't advance IP to halt execution
            break;
        }

        default:
            std::cerr << "Unhandled opcode: 0x" << std::hex << (int)opcode
                      << " at address 0x" << cpu->ip << std::endl;
            cpu->ip += 1; // Just skip unknown opcodes for now
            break;
    }
}

// --- Interrupt Management Functions ---

void i960_interrupt(i960_cpu* cpu, uint8_t vector) {
    if (cpu->interrupt_mode) {
        std::cerr << "Nested interrupts not supported yet" << std::endl;
        return;
    }

    if (vector >= 256 || cpu->interrupt_vectors[vector] == 0) {
        std::cerr << "Invalid or unhandled interrupt vector: " << (int)vector << std::endl;
        return;
    }

    // Save current state on interrupt stack
    if (cpu->interrupt_sp < 32) {
        cpu->interrupt_stack[cpu->interrupt_sp++] = cpu->ip;
        cpu->interrupt_stack[cpu->interrupt_sp++] = cpu->zero_flag ? 1 : 0;
        // Could save more state here (registers, etc.)
    }

    // Set interrupt mode
    cpu->interrupt_mode = true;

    // Jump to interrupt handler
    cpu->ip = cpu->interrupt_vectors[vector];

    std::cout << "Interrupt triggered: vector " << (int)vector
              << " -> handler at 0x" << std::hex << cpu->ip << std::endl;
}

void i960_return_from_interrupt(i960_cpu* cpu) {
    if (!cpu->interrupt_mode || cpu->interrupt_sp < 2) {
        std::cerr << "Not in interrupt mode or invalid interrupt stack" << std::endl;
        return;
    }

    // Restore state from interrupt stack
    cpu->zero_flag = (cpu->interrupt_stack[--cpu->interrupt_sp] != 0);
    cpu->ip = cpu->interrupt_stack[--cpu->interrupt_sp];
    // Could restore more state here

    // Clear interrupt mode
    cpu->interrupt_mode = false;

    std::cout << "Returned from interrupt, IP = 0x" << std::hex << cpu->ip << std::endl;
}

void i960_set_interrupt_vector(i960_cpu* cpu, uint8_t vector, uint32_t address) {
    if (vector < 256) {
        cpu->interrupt_vectors[vector] = address;
        std::cout << "Set interrupt vector " << (int)vector
                  << " to address 0x" << std::hex << address << std::endl;
    }
}
