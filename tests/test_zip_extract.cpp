#include "memory.h"
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstdio>

int main()
{
    MemoryBus bus;
    memory_init(&bus);

    // Use repository-local roms directory so the tests work on any clone
    std::string roms_dir = "roms/";
    std::string zip_path = roms_dir + "vcop2.zip";

    std::map<std::string, std::vector<uint8_t>> roms;
    if (!extract_zip_to_memory(zip_path, roms))
    {
        std::cerr << "Failed to extract ZIP: " << zip_path << std::endl;
        return 1;
    }

    std::cout << "Extracted " << roms.size() << " files from " << zip_path << std::endl;

    // Known expected sizes for vcop2.zip entries (partial list)
    std::unordered_map<std::string, size_t> expected_sizes;
    expected_sizes["epr-18518.14"] = 524288; // 0x80000
    expected_sizes["epr-18519.15"] = 524288;
    expected_sizes["epr-18520.4"] = 524288;
    expected_sizes["mpr-18510.20"] = 2097152; // 0x200000

    bool all_ok = true;

    // Check each extracted file against expected sizes when known
    for (const auto &pair : roms)
    {
        const std::string &name = pair.first;
        size_t size = pair.second.size();
        std::cout << "File: " << name << " size=" << size;
        auto it = expected_sizes.find(name);
        if (it != expected_sizes.end())
        {
            if (size != it->second)
            {
                std::cout << "  [MISMATCH] expected=" << it->second << std::endl;
                all_ok = false;
            }
            else
            {
                std::cout << "  [OK]" << std::endl;
            }
        }
        else
        {
            std::cout << std::endl;
        }
    }

    if (!all_ok)
    {
        std::cerr << "One or more extracted files did not match expected sizes." << std::endl;
        memory_destroy(&bus);
        return 2;
    }

    // Print first 16 bytes of the first file
    if (!roms.empty())
    {
        auto it = roms.begin();
        std::cout << "First file: " << it->first << " size=" << it->second.size() << std::endl;
        std::cout << "First 16 bytes: ";
        for (size_t i = 0; i < it->second.size() && i < 16; ++i)
        {
            printf("%02X ", it->second[i]);
        }
        std::cout << std::endl;
    }

    memory_destroy(&bus);
    return 0;
}
