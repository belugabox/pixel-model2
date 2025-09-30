#include "memory.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <map>
#include <algorithm>
#include <cstdio>

int main()
{
    MemoryBus bus;
    memory_init(&bus);

    // Use repository-local roms path by default
    const char *zip_path = "roms/vcop2.zip";
    const char *filename = "epr-18518.14";
    const uint32_t offset = 0x0;

    // Extract the file directly from ZIP to get the expected bytes
    std::map<std::string, std::vector<uint8_t>> roms;
    if (!extract_zip_to_memory(zip_path, roms))
    {
        std::cerr << "Failed to extract ZIP for verification" << std::endl;
        memory_destroy(&bus);
        return 1;
    }

    std::string lower_name = filename;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    auto it = roms.find(lower_name);
    if (it == roms.end())
    {
        std::cerr << "Verification: file not found in ZIP: " << filename << std::endl;
        memory_destroy(&bus);
        return 1;
    }

    const std::vector<uint8_t> &expected_data = it->second;
    if (expected_data.size() < 16)
    {
        std::cerr << "Unexpected small file size for verification" << std::endl;
        memory_destroy(&bus);
        return 1;
    }

    if (!load_rom_from_zip(&bus, zip_path, filename, offset))
    {
        std::cerr << "load_rom_from_zip failed" << std::endl;
        memory_destroy(&bus);
        return 1;
    }

    bool ok = true;
    std::cout << "Memory first 16 bytes at offset 0x0: ";
    for (uint32_t i = 0; i < 16; ++i)
    {
        uint8_t b = memory_read_byte(&bus, offset + i);
        printf("%02X ", b);
        if (b != expected_data[i])
            ok = false;
    }
    std::cout << std::endl;

    if (!ok)
    {
        std::cerr << "Loaded data does not match extracted data" << std::endl;
        memory_destroy(&bus);
        return 2;
    }

    std::cout << "load_rom_from_zip succeeded and memory matches extracted data." << std::endl;
    memory_destroy(&bus);
    return 0;
}
