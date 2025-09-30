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

    // Initialize CPU state
    cpu->halted = false;

    // Initialize interrupt system
    cpu->interrupt_mode = false;
    cpu->interrupt_sp = 0;
    for (int i = 0; i < 256; ++i) {
        cpu->interrupt_vectors[i] = 0; // No handlers by default
    }

    std::cout << "i960 CPU Initialized and connected to Memory Bus." << std::endl;
}

void i960_step(i960_cpu* cpu) {
    // Check if CPU is halted
    if (cpu->halted) {
        return; // Do nothing if halted
    }

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
            cpu->halted = true;
            break;
        }

        // --- Load/Store Instructions ---
        case 0xC0: { // ld (load word from memory)
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint32_t addr = 0;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 2) << 24;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 3) << 16;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 4) << 8;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 5);

            if (dst_reg < 16) {
                cpu->g[dst_reg] = memory_read_dword(cpu->bus, addr);
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": ld -> g" << std::dec << (int)dst_reg << " = [0x" << std::hex << addr << "] = 0x" << cpu->g[dst_reg] << std::endl;
            }
            cpu->ip += 6;
            break;
        }

        case 0xC1: { // st (store word to memory)
            uint32_t base_ip = cpu->ip;
            uint8_t src_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint32_t addr = 0;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 2) << 24;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 3) << 16;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 4) << 8;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 5);

            if (src_reg < 16) {
                memory_write_dword(cpu->bus, addr, cpu->g[src_reg]);
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": st -> [0x" << addr << "] = g" << std::dec << (int)src_reg << " = 0x" << std::hex << cpu->g[src_reg] << std::endl;
            }
            cpu->ip += 6;
            break;
        }

        case 0xC2: { // ld_byte (load byte from memory)
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint32_t addr = 0;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 2) << 24;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 3) << 16;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 4) << 8;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 5);

            if (dst_reg < 16) {
                cpu->g[dst_reg] = memory_read_byte(cpu->bus, addr);
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": ld_byte -> g" << std::dec << (int)dst_reg << " = [0x" << std::hex << addr << "] = 0x" << cpu->g[dst_reg] << std::endl;
            }
            cpu->ip += 6;
            break;
        }

        case 0xC3: { // st_byte (store byte to memory)
            uint32_t base_ip = cpu->ip;
            uint8_t src_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint32_t addr = 0;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 2) << 24;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 3) << 16;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 4) << 8;
            addr |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 5);

            if (src_reg < 16) {
                memory_write_byte(cpu->bus, addr, cpu->g[src_reg] & 0xFF);
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": st_byte -> [0x" << addr << "] = g" << std::dec << (int)src_reg << " = 0x" << std::hex << (cpu->g[src_reg] & 0xFF) << std::endl;
            }
            cpu->ip += 6;
            break;
        }

        // --- Arithmetic Instructions ---
        case 0xD0: { // sub_reg (subtract registers)
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t src1_reg = memory_read_byte(cpu->bus, base_ip + 2);
            uint8_t src2_reg = memory_read_byte(cpu->bus, base_ip + 3);

            if (dst_reg < 16 && src1_reg < 16 && src2_reg < 16) {
                uint32_t val1 = cpu->g[src1_reg];
                uint32_t val2 = cpu->g[src2_reg];
                cpu->g[dst_reg] = val1 - val2;
                cpu->zero_flag = (cpu->g[dst_reg] == 0);

                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": sub_reg -> g" << std::dec << (int)dst_reg << " = 0x" 
                          << std::hex << val1 << " - 0x" << val2 << " = 0x" << cpu->g[dst_reg] << std::endl;
            }
            cpu->ip += 4;
            break;
        }

        case 0xD1: { // mul_reg (multiply registers)
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t src1_reg = memory_read_byte(cpu->bus, base_ip + 2);
            uint8_t src2_reg = memory_read_byte(cpu->bus, base_ip + 3);

            if (dst_reg < 16 && src1_reg < 16 && src2_reg < 16) {
                uint32_t val1 = cpu->g[src1_reg];
                uint32_t val2 = cpu->g[src2_reg];
                cpu->g[dst_reg] = val1 * val2;
                cpu->zero_flag = (cpu->g[dst_reg] == 0);

                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": mul_reg -> g" << std::dec << (int)dst_reg << " = 0x" 
                          << std::hex << val1 << " * 0x" << val2 << " = 0x" << cpu->g[dst_reg] << std::endl;
            }
            cpu->ip += 4;
            break;
        }

        case 0xD2: { // div_reg (divide registers)
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t src1_reg = memory_read_byte(cpu->bus, base_ip + 2);
            uint8_t src2_reg = memory_read_byte(cpu->bus, base_ip + 3);

            if (dst_reg < 16 && src1_reg < 16 && src2_reg < 16 && cpu->g[src2_reg] != 0) {
                uint32_t val1 = cpu->g[src1_reg];
                uint32_t val2 = cpu->g[src2_reg];
                cpu->g[dst_reg] = val1 / val2;
                cpu->zero_flag = (cpu->g[dst_reg] == 0);

                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": div_reg -> g" << std::dec << (int)dst_reg << " = 0x" 
                          << std::hex << val1 << " / 0x" << val2 << " = 0x" << cpu->g[dst_reg] << std::endl;
            }
            cpu->ip += 4;
            break;
        }

        // --- Logical Instructions ---
        case 0xE0: { // and_reg (bitwise AND)
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t src1_reg = memory_read_byte(cpu->bus, base_ip + 2);
            uint8_t src2_reg = memory_read_byte(cpu->bus, base_ip + 3);

            if (dst_reg < 16 && src1_reg < 16 && src2_reg < 16) {
                uint32_t val1 = cpu->g[src1_reg];
                uint32_t val2 = cpu->g[src2_reg];
                cpu->g[dst_reg] = val1 & val2;
                cpu->zero_flag = (cpu->g[dst_reg] == 0);

                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": and_reg -> g" << std::dec << (int)dst_reg << " = 0x" 
                          << std::hex << val1 << " & 0x" << val2 << " = 0x" << cpu->g[dst_reg] << std::endl;
            }
            cpu->ip += 4;
            break;
        }

        case 0xE1: { // or_reg (bitwise OR)
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t src1_reg = memory_read_byte(cpu->bus, base_ip + 2);
            uint8_t src2_reg = memory_read_byte(cpu->bus, base_ip + 3);

            if (dst_reg < 16 && src1_reg < 16 && src2_reg < 16) {
                uint32_t val1 = cpu->g[src1_reg];
                uint32_t val2 = cpu->g[src2_reg];
                cpu->g[dst_reg] = val1 | val2;
                cpu->zero_flag = (cpu->g[dst_reg] == 0);

                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": or_reg -> g" << std::dec << (int)dst_reg << " = 0x" 
                          << std::hex << val1 << " | 0x" << val2 << " = 0x" << cpu->g[dst_reg] << std::endl;
            }
            cpu->ip += 4;
            break;
        }

        case 0xE2: { // xor_reg (bitwise XOR)
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t src1_reg = memory_read_byte(cpu->bus, base_ip + 2);
            uint8_t src2_reg = memory_read_byte(cpu->bus, base_ip + 3);

            if (dst_reg < 16 && src1_reg < 16 && src2_reg < 16) {
                uint32_t val1 = cpu->g[src1_reg];
                uint32_t val2 = cpu->g[src2_reg];
                cpu->g[dst_reg] = val1 ^ val2;
                cpu->zero_flag = (cpu->g[dst_reg] == 0);

                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": xor_reg -> g" << std::dec << (int)dst_reg << " = 0x" 
                          << std::hex << val1 << " ^ 0x" << val2 << " = 0x" << cpu->g[dst_reg] << std::endl;
            }
            cpu->ip += 4;
            break;
        }

        case 0xE3: { // not_reg (bitwise NOT)
            uint32_t base_ip = cpu->ip;
            uint8_t dst_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t src_reg = memory_read_byte(cpu->bus, base_ip + 2);

            if (dst_reg < 16 && src_reg < 16) {
                uint32_t val = cpu->g[src_reg];
                cpu->g[dst_reg] = ~val;
                cpu->zero_flag = (cpu->g[dst_reg] == 0);

                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": not_reg -> g" << std::dec << (int)dst_reg << " = ~g" << (int)src_reg << " = 0x" 
                          << std::hex << val << " -> 0x" << cpu->g[dst_reg] << std::endl;
            }
            cpu->ip += 3;
            break;
        }

        // --- Comparison and Branch Instructions ---
        case 0xF0: { // cmp_reg (compare registers)
            uint32_t base_ip = cpu->ip;
            uint8_t src1_reg = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t src2_reg = memory_read_byte(cpu->bus, base_ip + 2);

            if (src1_reg < 16 && src2_reg < 16) {
                uint32_t val1 = cpu->g[src1_reg];
                uint32_t val2 = cpu->g[src2_reg];
                cpu->zero_flag = (val1 == val2);

                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": cmp_reg -> g" << std::dec << (int)src1_reg << " (0x" << std::hex << val1 
                          << ") cmp g" << (int)src2_reg << " (0x" << val2 << ") -> zero_flag = " << cpu->zero_flag << std::endl;
            }
            cpu->ip += 3;
            break;
        }

        case 0xF1: { // beq (branch if equal)
            uint32_t base_ip = cpu->ip;
            uint32_t target = 0;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 1) << 24;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 2) << 16;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 3) << 8;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 4);

            if (cpu->zero_flag) {
                cpu->ip = target;
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": beq -> branching to 0x" << target << " (zero_flag set)" << std::endl;
            } else {
                cpu->ip += 5;
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": beq -> not branching (zero_flag clear)" << std::endl;
            }
            break;
        }

        case 0xF2: { // bne (branch if not equal)
            uint32_t base_ip = cpu->ip;
            uint32_t target = 0;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 1) << 24;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 2) << 16;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 3) << 8;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 4);

            if (!cpu->zero_flag) {
                cpu->ip = target;
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": bne -> branching to 0x" << target << " (zero_flag clear)" << std::endl;
            } else {
                cpu->ip += 5;
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": bne -> not branching (zero_flag set)" << std::endl;
            }
            break;
        }

        case 0xF3: { // jmp (unconditional jump)
            uint32_t base_ip = cpu->ip;
            uint32_t target = 0;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 1) << 24;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 2) << 16;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 3) << 8;
            target |= (uint32_t)memory_read_byte(cpu->bus, base_ip + 4);

            if (target >= MEMORY_SIZE) {
                std::cout << "Executing at 0x" << std::hex << base_ip
                          << ": jmp -> target address 0x" << target << " is out of bounds, halting CPU" << std::endl;
                cpu->halted = true;
                break;
            }

            cpu->ip = target;
            std::cout << "Executing at 0x" << std::hex << base_ip
                      << ": jmp -> jumping to 0x" << target << std::endl;
            break;
        }

        // --- Additional Instructions ---
        case 0xDD: { // Unknown instruction, possibly a system call or special operation
            uint32_t base_ip = cpu->ip;
            uint8_t param1 = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t param2 = memory_read_byte(cpu->bus, base_ip + 2);

            std::cout << "Executing at 0x" << std::hex << base_ip
                      << ": unknown_0xDD -> params: 0x" << std::hex << (int)param1 << ", 0x" << (int)param2 << std::endl;
            // For now, just skip this instruction
            cpu->ip += 3;
            break;
        }

        case 0xCD: { // Unknown instruction, possibly a system call or special operation
            uint32_t base_ip = cpu->ip;
            uint8_t param1 = memory_read_byte(cpu->bus, base_ip + 1);
            uint8_t param2 = memory_read_byte(cpu->bus, base_ip + 2);

            std::cout << "Executing at 0x" << std::hex << base_ip
                      << ": unknown_0xCD -> params: 0x" << std::hex << (int)param1 << ", 0x" << (int)param2 << std::endl;
            // For now, just skip this instruction
            cpu->ip += 3;
            break;
        }

        case 0xFD: { // Unknown instruction
            uint32_t base_ip = cpu->ip;
            uint8_t param = memory_read_byte(cpu->bus, base_ip + 1);

            std::cout << "Executing at 0x" << std::hex << base_ip
                      << ": unknown_0xFD -> param: 0x" << std::hex << (int)param << std::endl;
            cpu->ip += 2;
            break;
        }

        case 0xFE: { // Unknown instruction
            uint32_t base_ip = cpu->ip;
            uint8_t param = memory_read_byte(cpu->bus, base_ip + 1);

            std::cout << "Executing at 0x" << std::hex << base_ip
                      << ": unknown_0xFE -> param: 0x" << std::hex << (int)param << std::endl;
            cpu->ip += 2;
            break;
        }

        case 0x01: { // Unknown instruction, possibly a nop or small immediate
            uint32_t base_ip = cpu->ip;
            std::cout << "Executing at 0x" << std::hex << base_ip << ": unknown_0x01" << std::endl;
            cpu->ip += 1;
            break;
        }

        case 0x02: { // Unknown instruction
            uint32_t base_ip = cpu->ip;
            std::cout << "Executing at 0x" << std::hex << base_ip << ": unknown_0x02" << std::endl;
            cpu->ip += 1;
            break;
        }

        case 0x03: { // Unknown instruction
            uint32_t base_ip = cpu->ip;
            std::cout << "Executing at 0x" << std::hex << base_ip << ": unknown_0x03" << std::endl;
            cpu->ip += 1;
            break;
        }

        case 0x21: { // Unknown instruction
            uint32_t base_ip = cpu->ip;
            uint8_t param = memory_read_byte(cpu->bus, base_ip + 1);
            std::cout << "Executing at 0x" << std::hex << base_ip
                      << ": unknown_0x21 -> param: 0x" << std::hex << (int)param << std::endl;
            cpu->ip += 2;
            break;
        }
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
