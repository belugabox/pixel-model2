#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>

// Forward declaration to avoid circular dependency
struct TGP;

// Let's define a simple memory size for now. 16MB.
const uint32_t MEMORY_SIZE = 16 * 1024 * 1024;

// TGP registers are memory-mapped starting at this address
const uint32_t TGP_BASE_ADDRESS = 0xC0000000;

struct MemoryBus {
    uint8_t* ram;
    TGP* tgp;  // Pointer to TGP for memory-mapped access
};

// Allocates and initializes the memory bus
void memory_init(MemoryBus* bus);

// Frees the allocated memory
void memory_destroy(MemoryBus* bus);

// Read a single byte from a given address
uint8_t memory_read_byte(MemoryBus* bus, uint32_t address);

// Write a single byte to a given address
void memory_write_byte(MemoryBus* bus, uint32_t address, uint8_t value);

// Read a 32-bit word from a given address
uint32_t memory_read_dword(MemoryBus* bus, uint32_t address);

// Write a 32-bit word to a given address
void memory_write_dword(MemoryBus* bus, uint32_t address, uint32_t value);

// Connect TGP to memory bus for memory-mapped register access
void memory_connect_tgp(MemoryBus* bus, TGP* tgp);

// Load a binary file into memory at a specific offset
bool load_rom_from_file(MemoryBus* bus, const char* filepath, uint32_t offset);


#endif // MEMORY_H
