#include "memory.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include "tgp.h"  // Include here to access TGP struct

void memory_init(MemoryBus* bus) {
    bus->ram = new uint8_t[MEMORY_SIZE];
    // Initialize all memory to a known value, e.g., 0
    for (uint32_t i = 0; i < MEMORY_SIZE; ++i) {
        bus->ram[i] = 0;
    }
    bus->tgp = nullptr;  // Will be set later when TGP is initialized
    std::cout << "Memory Bus Initialized: " << (MEMORY_SIZE / 1024 / 1024) << "MB" << std::endl;
}

void memory_destroy(MemoryBus* bus) {
    delete[] bus->ram;
    bus->ram = nullptr;
    bus->tgp = nullptr;
}


uint8_t memory_read_byte(MemoryBus* bus, uint32_t address) {
    if (address >= MEMORY_SIZE) {
        // For now, just log and return 0. In a real emulator, this would trigger an exception.
        std::cerr << "Warning: Out-of-bounds memory read at address 0x" << std::hex << address << std::endl;
        return 0;
    }
    return bus->ram[address];
}

void memory_write_byte(MemoryBus* bus, uint32_t address, uint8_t value) {
    if (address >= MEMORY_SIZE) {
        std::cerr << "Warning: Out-of-bounds memory write at address 0x" << std::hex << address << std::endl;
        return;
    }
    bus->ram[address] = value;
}

uint32_t memory_read_dword(MemoryBus* bus, uint32_t address) {
    // Check if this is a TGP register access
    if (address >= TGP_BASE_ADDRESS && address < TGP_BASE_ADDRESS + 0x1000) {
        uint32_t offset = address - TGP_BASE_ADDRESS;
        return tgp_read_register(bus->tgp, offset);
    }

    uint32_t value = 0;
    value |= (uint32_t)memory_read_byte(bus, address + 0);
    value |= (uint32_t)memory_read_byte(bus, address + 1) << 8;
    value |= (uint32_t)memory_read_byte(bus, address + 2) << 16;
    value |= (uint32_t)memory_read_byte(bus, address + 3) << 24;
    return value;
}

void memory_write_dword(MemoryBus* bus, uint32_t address, uint32_t value) {
    // Check if this is a TGP register access
    if (address >= TGP_BASE_ADDRESS && address < TGP_BASE_ADDRESS + 0x1000) {
        uint32_t offset = address - TGP_BASE_ADDRESS;
        tgp_write_register(bus->tgp, offset, value);
        return;
    }

    memory_write_byte(bus, address + 0, (value >> 0) & 0xFF);
    memory_write_byte(bus, address + 1, (value >> 8) & 0xFF);
    memory_write_byte(bus, address + 2, (value >> 16) & 0xFF);
    memory_write_byte(bus, address + 3, (value >> 24) & 0xFF);
}

bool load_rom_from_file(MemoryBus* bus, const char* filepath, uint32_t offset) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open ROM file: " << filepath << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (offset + size > MEMORY_SIZE) {
        std::cerr << "Error: ROM file is too large to fit in memory." << std::endl;
        return false;
    }

    if (file.read(reinterpret_cast<char*>(bus->ram + offset), size)) {
        std::cout << "Successfully loaded " << size << " bytes from " << filepath << " into memory at offset 0x" << std::hex << offset << std::endl;
        return true;
    }
    
    std::cerr << "Error: Could not read ROM file: " << filepath << std::endl;
    return false;
}

void memory_connect_tgp(MemoryBus* bus, TGP* tgp) {
    bus->tgp = tgp;
    std::cout << "TGP connected to memory bus at address 0x" << std::hex << TGP_BASE_ADDRESS << std::endl;
}
