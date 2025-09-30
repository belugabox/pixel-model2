#include "memory.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include "tgp.h"  // Include here to access TGP struct
#include <filesystem>
#include <string>
#include <cstring>

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
    {"epr-17659.15", 0x000000, 0x80000},  // Main program ROM
    {"epr-17660.16", 0x080000, 0x80000},
    {"epr-17661.17", 0x100000, 0x80000},
    {"epr-17662.18", 0x180000, 0x80000},
    {"mpr-17631.1", 0x200000, 0x80000},  // Data ROM
    {"mpr-17632.2", 0x280000, 0x80000},
    {"mpr-17633.3", 0x300000, 0x80000},
    {"mpr-17634.4", 0x380000, 0x80000},
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
    
    for (int i = 0; i < config->num_roms; i++) {
        const RomFile* rom = &config->roms[i];
        
        // Build full path
        std::string filepath = std::string(rom_directory) + "/" + rom->filename;
        
        std::cout << "Loading ROM: " << rom->filename << " at offset 0x" << std::hex << rom->offset << std::endl;
        
        if (!load_rom_from_file(bus, filepath.c_str(), rom->offset)) {
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
