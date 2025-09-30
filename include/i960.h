#ifndef I960_H
#define I960_H

#include <cstdint>
#include "memory.h" // Include memory bus definition

// The Intel i960 has 16 global 32-bit registers (g0-g15)
// and 16 local 32-bit registers (r0-r15).
// For simplicity, we'll start with the global registers.

struct i960_cpu {
    // --- Registers ---
    uint32_t g[16]; // Global registers
    // We will add local registers and special function registers later.

    // --- Program Counter ---
    uint32_t ip; // Instruction Pointer

    // Connection to the memory bus
    MemoryBus* bus;

    // --- CPU Flags ---
    bool zero_flag;

    // --- Special Registers ---
    uint32_t sp; // Stack Pointer

    // --- CPU State ---
    bool halted; // True when CPU is halted and should not execute further

    // --- Interrupt System ---
    bool interrupt_mode;        // True when processing an interrupt
    uint32_t interrupt_stack[32]; // Stack for interrupt context (IP, flags, etc.)
    uint32_t interrupt_sp;      // Interrupt stack pointer
    uint32_t interrupt_vectors[256]; // Interrupt vector table (addresses of handlers)

    // We will add other state like Arithmetic Controls, etc. later
};

// Initializes the CPU to a default power-on state
void i960_init(i960_cpu* cpu, MemoryBus* bus);

// Executes a single instruction cycle (fetch-decode-execute)
void i960_step(i960_cpu* cpu);

// --- Interrupt Management ---
// Triggers a software interrupt
void i960_interrupt(i960_cpu* cpu, uint8_t vector);

// Returns from interrupt handler
void i960_return_from_interrupt(i960_cpu* cpu);

// Sets an interrupt vector (handler address)
void i960_set_interrupt_vector(i960_cpu* cpu, uint8_t vector, uint32_t address);

#endif // I960_H
