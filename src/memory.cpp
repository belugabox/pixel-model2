#include "memory.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include "tgp.h"  // Include here to access TGP struct
#include <filesystem>
#include <string>
#include <cstring>
#include <cstdlib>  // for system()
#include <map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <ctime>

#include "miniz.h"

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
        // Return 0 for out-of-bounds reads (could be peripheral space)
        return 0;
    }
    return bus->ram[address];
}

void memory_write_byte(MemoryBus* bus, uint32_t address, uint8_t value) {
    if (address >= MEMORY_SIZE) {
        // Ignore out-of-bounds writes (could be peripheral space)
        return;
    }
    bus->ram[address] = value;
}

uint32_t memory_read_dword(MemoryBus* bus, uint32_t address) {
    // Check if this is an input register access
    if (address >= INPUT_BASE_ADDRESS && address < INPUT_BASE_ADDRESS + 0x100) {
        if (bus->input_state) {
            // Cast to InputState structure (defined in main.cpp)
            struct InputState {
                bool start_button, service_button, test_button, coin_button;
                bool button1, button2, button3, button4;
                bool up, down, left, right;
                int16_t steering, throttle;
            } *input = (InputState*)bus->input_state;
            uint32_t offset = address - INPUT_BASE_ADDRESS;
            
            switch (offset) {
                case 0x00: return input->start_button ? 1 : 0;
                case 0x04: return input->service_button ? 1 : 0;
                case 0x08: return input->test_button ? 1 : 0;
                case 0x0C: return input->coin_button ? 1 : 0;
                case 0x10: return input->button1 ? 1 : 0;
                case 0x14: return input->button2 ? 1 : 0;
                case 0x18: return input->button3 ? 1 : 0;
                case 0x1C: return input->button4 ? 1 : 0;
                case 0x20: return (input->up ? 1 : 0) | (input->down ? 2 : 0) | (input->left ? 4 : 0) | (input->right ? 8 : 0);
                case 0x24: return (uint32_t)(int32_t)input->steering;
                case 0x28: return (uint32_t)(int32_t)input->throttle;
                default: return 0;
            }
        }
        return 0;
    }

    // Check if this is a TGP register access
    if (address >= TGP_BASE_ADDRESS && address < TGP_BASE_ADDRESS + 0x1000) {
        uint32_t offset = address - TGP_BASE_ADDRESS;
        return tgp_read_register(bus->tgp, offset);
    }

    // Check if this is an audio register access
    if (address >= AUDIO_BASE_ADDRESS && address < AUDIO_BASE_ADDRESS + 0x200) {
        if (bus->audio_state) {
            // Cast to AudioState structure (defined in main.cpp)
            struct AudioState {
                void* stream;
                bool enabled;
                float master_volume;
                struct {
                    bool enabled;
                    uint16_t frequency;
                    uint8_t volume;
                    uint8_t waveform;
                } channels[8];
            } *audio = (AudioState*)bus->audio_state;
            uint32_t offset = address - AUDIO_BASE_ADDRESS;
            
            switch (offset) {
                case 0x00: return audio->enabled ? 1 : 0;
                case 0x04: return (uint32_t)(audio->master_volume * 255.0f);
                // Channel registers (0x10 + channel*0x10 + register)
                default: {
                    uint32_t channel = (offset - 0x10) / 0x10;
                    uint32_t reg = (offset - 0x10) % 0x10;
                    if (channel < 8) {
                        switch (reg) {
                            case 0x00: return audio->channels[channel].enabled ? 1 : 0;
                            case 0x04: return audio->channels[channel].frequency;
                            case 0x08: return audio->channels[channel].volume;
                            case 0x0C: return audio->channels[channel].waveform;
                        }
                    }
                    return 0;
                }
            }
        }
        return 0;
    }

    uint32_t value = 0;
    value |= (uint32_t)memory_read_byte(bus, address + 0);
    value |= (uint32_t)memory_read_byte(bus, address + 1) << 8;
    value |= (uint32_t)memory_read_byte(bus, address + 2) << 16;
    value |= (uint32_t)memory_read_byte(bus, address + 3) << 24;
    return value;
}

void memory_write_dword(MemoryBus* bus, uint32_t address, uint32_t value) {
    // Check if this is an input register access (read-only, ignore writes)
    if (address >= INPUT_BASE_ADDRESS && address < INPUT_BASE_ADDRESS + 0x100) {
        return;
    }

    // Check if this is a TGP register access
    if (address >= TGP_BASE_ADDRESS && address < TGP_BASE_ADDRESS + 0x1000) {
        uint32_t offset = address - TGP_BASE_ADDRESS;
        tgp_write_register(bus->tgp, offset, value);
        return;
    }

    // Check if this is an audio register access
    if (address >= AUDIO_BASE_ADDRESS && address < AUDIO_BASE_ADDRESS + 0x200) {
        if (bus->audio_state) {
            // Cast to AudioState structure (defined in main.cpp)
            struct AudioState {
                void* stream;
                bool enabled;
                float master_volume;
                struct {
                    bool enabled;
                    uint16_t frequency;
                    uint8_t volume;
                    uint8_t waveform;
                } channels[8];
            } *audio = (AudioState*)bus->audio_state;
            uint32_t offset = address - AUDIO_BASE_ADDRESS;
            
            switch (offset) {
                case 0x00: audio->enabled = (value != 0); break;
                case 0x04: audio->master_volume = (float)value / 255.0f; break;
                // Channel registers (0x10 + channel*0x10 + register)
                default: {
                    uint32_t channel = (offset - 0x10) / 0x10;
                    uint32_t reg = (offset - 0x10) % 0x10;
                    if (channel < 8) {
                        switch (reg) {
                            case 0x00: audio->channels[channel].enabled = (value != 0); break;
                            case 0x04: audio->channels[channel].frequency = (uint16_t)value; break;
                            case 0x08: audio->channels[channel].volume = (uint8_t)value; break;
                            case 0x0C: audio->channels[channel].waveform = (uint8_t)value; break;
                        }
                    }
                    break;
                }
            }
        }
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

// Extract a file from ZIP archive using PowerShell and load it
bool load_rom_from_zip(MemoryBus* bus, const char* zip_path, const char* filename, uint32_t offset) {
    // Extract all files from ZIP to memory
    std::map<std::string, std::vector<uint8_t>> rom_data;
    if (!extract_zip_to_memory(std::string(zip_path), rom_data)) {
        std::cerr << "Error: Failed to extract ZIP file: " << zip_path << std::endl;
        return false;
    }

    // Convert filename to lowercase for case-insensitive matching
    std::string lower_filename = filename;
    std::transform(lower_filename.begin(), lower_filename.end(), lower_filename.begin(), ::tolower);

    // Find the requested file
    auto it = rom_data.find(lower_filename);
    if (it == rom_data.end()) {
        std::cerr << "Error: File not found in ZIP: " << filename << std::endl;
        std::cout << "Available files in ZIP:" << std::endl;
        for (const auto& pair : rom_data) {
            std::cout << "  " << pair.first << std::endl;
        }
        return false;
    }

    // Load the file data directly from memory
    const std::vector<uint8_t>& file_data = it->second;
    std::cout << "Loading " << filename << " (" << file_data.size() << " bytes) from ZIP to offset 0x" << std::hex << offset << std::dec << std::endl;

    // Bounds check to ensure the ROM fits in memory
    if ((uint64_t)offset + file_data.size() > (uint64_t)MEMORY_SIZE) {
        std::cerr << "Error: ROM data would overflow memory when loaded at offset 0x" << std::hex << offset << std::dec << std::endl;
        return false;
    }

    // Write data to memory bus using a 32-bit index to avoid size_t->uint32_t conversion warnings
    for (uint32_t i = 0; i < (uint32_t)file_data.size(); ++i) {
        memory_write_byte(bus, offset + i, file_data[i]);
    }

    return true;
}

// Extract all files from ZIP archive to memory map
bool extract_zip_to_memory(const std::string& zip_path, std::map<std::string, std::vector<uint8_t>>& rom_data) {
    std::cout << "Attempting to open ZIP file: " << zip_path << std::endl;
    
    // Read entire ZIP file into memory
    std::ifstream file(zip_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open ZIP file: " << zip_path << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size > 0x7FFFFFFF) {
        std::cerr << "ZIP file too large to process in-memory: " << size << " bytes" << std::endl;
        return false;
    }

    std::vector<uint8_t> zip_data((size_t)size);
    if (!file.read(reinterpret_cast<char*>(zip_data.data()), size)) {
        std::cerr << "Failed to read ZIP file content" << std::endl;
        return false;
    }
    file.close();
    
    std::cout << "Successfully read " << size << " bytes from ZIP file" << std::endl;

    // Initialize miniz ZIP reader
    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));

    if (!mz_zip_reader_init_mem(&zip_archive, zip_data.data(), zip_data.size(), 0)) {
        std::cerr << "Failed to initialize ZIP reader" << std::endl;
        return false;
    }
    
    std::cout << "ZIP reader initialized successfully" << std::endl;

    // Get number of files
    uint32_t file_count = (uint32_t)mz_zip_reader_get_num_files(&zip_archive);
    std::cout << "ZIP contains " << file_count << " files" << std::endl;

    // Extract all files
    for (uint32_t i = 0; i < file_count; i++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
            continue;
        }

        // Skip directories
        if (file_stat.m_filename[strlen(file_stat.m_filename) - 1] == '/' ||
            file_stat.m_filename[strlen(file_stat.m_filename) - 1] == '\\') {
            continue;
        }

        // Check compression method - skip unsupported methods
        // if (file_stat.m_method != 0 && file_stat.m_method != 8) {
        //     std::cout << "Skipping compressed file: " << file_stat.m_filename << " (method: " << file_stat.m_method << ")" << std::endl;
        //     continue;
        // }

        std::cout << "Extracting file: " << file_stat.m_filename << " (size: " << file_stat.m_uncomp_size << ", method: " << file_stat.m_method << ")" << std::endl;

        // Skip files with invalid size
        if (file_stat.m_uncomp_size == 0 || file_stat.m_uncomp_size > 10 * 1024 * 1024) { // 10MB max
            std::cout << "Skipping file with invalid size: " << file_stat.m_filename << " (" << file_stat.m_uncomp_size << " bytes)" << std::endl;
            continue;
        }

        // Extract file to memory (miniz supports only stored files in this simplified header)
        std::vector<uint8_t> buffer(file_stat.m_uncomp_size);
        bool extracted = false;

        if (file_stat.m_method == 0) {
            // Stored (uncompressed) - miniz can copy directly
            if (mz_zip_reader_extract_to_mem(&zip_archive, i, buffer.data(), buffer.size(), 0)) {
                extracted = true;
                std::cout << "Successfully extracted (stored) file: " << file_stat.m_filename << " (" << buffer.size() << " bytes)" << std::endl;
            } else {
                std::cout << "Failed to extract stored file: " << file_stat.m_filename << std::endl;
            }
        } else {
        std::cout << "Entry uses compression method " << file_stat.m_method << ", extracting in-process (miniz) or falling back to external extraction only if needed" << std::endl;
        }

        // If not extracted yet and the entry is compressed (e.g., deflate method 8),
        // try extracting to heap via miniz helper (zlib-backed) which supports deflate.
        if (!extracted) {
            if (file_stat.m_method == 8) {
                size_t out_size = 0;
                void* out_buf = mz_zip_reader_extract_to_heap(&zip_archive, i, &out_size, 0);
                if (out_buf) {
                    buffer.assign((uint8_t*)out_buf, (uint8_t*)out_buf + out_size);
                    extracted = true;
                    std::cout << "Successfully extracted (deflate) file: " << file_stat.m_filename << " (" << out_size << " bytes)" << std::endl;
                    mz_free(out_buf);
                } else {
                    std::cout << "Failed to extract (deflate) file via heap: " << file_stat.m_filename << std::endl;
                }
            } else {
                std::cout << "Unsupported compression method " << file_stat.m_method << " for file: " << file_stat.m_filename << std::endl;
            }
        }

        if (!extracted) {
            std::cout << "Skipping file (could not extract): " << file_stat.m_filename << std::endl;
            continue;
        }

        // Convert filename to lowercase for case-insensitive matching
        std::string filename = file_stat.m_filename;
        std::string lower_filename = filename;
        std::transform(lower_filename.begin(), lower_filename.end(), lower_filename.begin(), ::tolower);

        rom_data[lower_filename] = std::move(buffer);
    }

    mz_zip_reader_end(&zip_archive);
    
    std::cout << "Extracted " << rom_data.size() << " files from ZIP" << std::endl;
    return !rom_data.empty();
}

void memory_connect_tgp(MemoryBus* bus, TGP* tgp) {
    bus->tgp = tgp;
    std::cout << "TGP connected to memory bus at address 0x" << std::hex << TGP_BASE_ADDRESS << std::endl;
}

void memory_connect_input(MemoryBus* bus, void* input_state) {
    bus->input_state = input_state;
    std::cout << "Input system connected to memory bus at address 0x" << std::hex << INPUT_BASE_ADDRESS << std::endl;
}

void memory_connect_audio(MemoryBus* bus, void* audio_state) {
    bus->audio_state = audio_state;
    std::cout << "Audio system connected to memory bus at address 0x" << std::hex << AUDIO_BASE_ADDRESS << std::endl;
}

// --- Game ROM Configurations ---
// Sega Model 2 games typically have multiple ROM files
static RomFile daytona_roms[] = {
    {"epr-16724a.6", 0x000000, 0x80000},  // Main program ROM (524288 bytes = 0x80000)
    {"epr-16725a.7", 0x080000, 0x80000},  // Main program ROM
    {"mpr-16491.32", 0x100000, 0x200000}, // Data ROM (2097152 bytes = 0x200000)
    {"mpr-16492.33", 0x300000, 0x200000}, // Data ROM
    {"mpr-16493.4", 0x500000, 0x200000},  // Data ROM
    {"mpr-16494.5", 0x700000, 0x200000},  // Data ROM
};

static RomFile vf3_roms[] = {
    {"epr-18518.14", 0x000000, 0x80000},  // Main program ROM (currently loaded)
};

static GameConfig available_games[] = {
    {"daytona", daytona_roms, sizeof(daytona_roms) / sizeof(RomFile)},
    {"vf3", vf3_roms, sizeof(vf3_roms) / sizeof(RomFile)},
};

bool load_game_roms(MemoryBus* bus, const GameConfig* config, const char* rom_directory) {
    std::cout << "Loading game: " << config->name << std::endl;
    
    // Map game names to ZIP filenames
    std::string zip_filename;
    if (strcmp(config->name, "vf3") == 0) {
        zip_filename = "vcop2.zip";
    } else if (strcmp(config->name, "daytona") == 0) {
        zip_filename = "daytona.zip";
    } else {
        // Default: use game name + .zip
        zip_filename = std::string(config->name) + ".zip";
    }
    
    for (int i = 0; i < config->num_roms; i++) {
        const RomFile* rom = &config->roms[i];
        
        // Load from ZIP file only (roms/ directory)
        std::string zip_path = std::string(rom_directory) + zip_filename;
        bool loaded = false;
        
        if (std::filesystem::exists(zip_path)) {
            std::cout << "Loading ROM: " << rom->filename << " from ZIP at offset 0x" << std::hex << rom->offset << std::endl;
            loaded = load_rom_from_zip(bus, zip_path.c_str(), rom->filename, rom->offset);
        } else {
            std::cerr << "Error: ZIP file not found: " << zip_path << std::endl;
        }
        
        if (!loaded) {
            std::cerr << "Failed to load ROM: " << rom->filename << std::endl;
            return false;
        }
    }
    
    std::cout << "Successfully loaded game: " << config->name << std::endl;
    return true;
}

bool load_game_by_name(MemoryBus* bus, const char* game_name, const char* rom_directory) {
    for (const auto& game : available_games) {
        if (strcmp(game.name, game_name) == 0) {
            return load_game_roms(bus, &game, rom_directory);
        }
    }
    
    std::cerr << "Unknown game: " << game_name << std::endl;
    std::cout << "Available games:" << std::endl;
    for (const auto& game : available_games) {
        std::cout << "  " << game.name << std::endl;
    }
    
    return false;
}
