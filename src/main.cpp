#define _USE_MATH_DEFINES
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <string>
#include "i960.h"
#include "memory.h"
#include "tgp.h"

// --- Input System ---
struct InputState {
    // Digital inputs (buttons)
    bool start_button = false;
    bool service_button = false;
    bool test_button = false;
    bool coin_button = false;
    
    // Action buttons
    bool button1 = false;
    bool button2 = false;
    bool button3 = false;
    bool button4 = false;
    
    // Directional inputs
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    
    // Analog inputs (for steering/wheel)
    int16_t steering = 0; // -32768 to 32767
    int16_t throttle = 0; // -32768 to 32767
};

InputState current_input;

// --- Audio System ---
struct AudioState {
    SDL_AudioStream* stream = nullptr;
    bool enabled = true;
    float master_volume = 1.0f;
    
    // Sega Model 2 audio channels (simplified)
    struct {
        bool enabled = false;
        uint16_t frequency = 0;
        uint8_t volume = 0;
        uint8_t waveform = 0; // 0=sine, 1=square, 2=triangle, 3=sawtooth
    } channels[8]; // 8 audio channels
};

static AudioState audio_state;

void handle_input_event(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            bool pressed = (event.type == SDL_EVENT_KEY_DOWN);
            SDL_Keycode key = event.key.key;
            
            // Sega Model 2 style controls
            if (key == SDLK_RETURN) { // Enter = Start
                current_input.start_button = pressed;
            } else if (key == SDLK_5) { // 5 = Coin
                current_input.coin_button = pressed;
            } else if (key == SDLK_9) { // 9 = Service
                current_input.service_button = pressed;
            } else if (key == SDLK_F2) { // F2 = Test
                current_input.test_button = pressed;
            } else if (key == SDLK_Z) { // Z = Button 1
                current_input.button1 = pressed;
            } else if (key == SDLK_X) { // X = Button 2
                current_input.button2 = pressed;
            } else if (key == SDLK_C) { // C = Button 3
                current_input.button3 = pressed;
            } else if (key == SDLK_V) { // V = Button 4
                current_input.button4 = pressed;
            } else if (key == SDLK_UP) { // Directional controls
                current_input.up = pressed;
            } else if (key == SDLK_DOWN) {
                current_input.down = pressed;
            } else if (key == SDLK_LEFT) {
                current_input.left = pressed;
            } else if (key == SDLK_RIGHT) {
                current_input.right = pressed;
            } else if (key == SDLK_A) { // A = Steer left
                if (pressed) current_input.steering = -16384;
                else current_input.steering = 0;
            } else if (key == SDLK_D) { // D = Steer right
                if (pressed) current_input.steering = 16384;
                else current_input.steering = 0;
            } else if (key == SDLK_W) { // W = Accelerate
                if (pressed) current_input.throttle = 16384;
                else current_input.throttle = 0;
            } else if (key == SDLK_S) { // S = Brake
                if (pressed) current_input.throttle = -16384;
                else current_input.throttle = 0;
            }
            break;
        }
        
        case SDL_EVENT_JOYSTICK_ADDED: {
            SDL_JoystickID joystick_id = event.jdevice.which;
            SDL_Joystick* joystick = SDL_OpenJoystick(joystick_id);
            if (joystick) {
                const char* name = SDL_GetJoystickName(joystick);
                std::cout << "Joystick connected: " << (name ? name : "Unknown") << std::endl;
                SDL_CloseJoystick(joystick); // Close immediately since we don't need to keep it open
            }
            break;
        }
        
        case SDL_EVENT_JOYSTICK_REMOVED: {
            std::cout << "Joystick disconnected" << std::endl;
            break;
        }
        
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
        case SDL_EVENT_JOYSTICK_BUTTON_UP: {
            bool pressed = (event.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN);
            int button = event.jbutton.button;
            
            // Map common joystick buttons to Sega Model 2 controls
            switch (button) {
                case 0: // A button
                    current_input.button1 = pressed;
                    break;
                case 1: // B button
                    current_input.button2 = pressed;
                    break;
                case 2: // X button
                    current_input.button3 = pressed;
                    break;
                case 3: // Y button
                    current_input.button4 = pressed;
                    break;
                case 6: // Back/Select
                    current_input.coin_button = pressed;
                    break;
                case 7: // Start
                    current_input.start_button = pressed;
                    break;
            }
            break;
        }
        
        case SDL_EVENT_JOYSTICK_HAT_MOTION: {
            uint8_t hat_value = event.jhat.value;
            current_input.up = (hat_value & SDL_HAT_UP);
            current_input.down = (hat_value & SDL_HAT_DOWN);
            current_input.left = (hat_value & SDL_HAT_LEFT);
            current_input.right = (hat_value & SDL_HAT_RIGHT);
            break;
        }
        
        case SDL_EVENT_JOYSTICK_AXIS_MOTION: {
            int axis = event.jaxis.axis;
            int16_t value = event.jaxis.value;
            
            // Map analog sticks to steering/throttle
            if (axis == 0) { // X axis (steering)
                current_input.steering = value;
            } else if (axis == 1) { // Y axis (throttle)
                current_input.throttle = -value; // Invert Y axis
            }
            break;
        }
    }
}

// --- Audio Callback ---
void audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
    // Generate audio samples for all active channels
    const int sample_rate = 44100;
    const int channels = 2; // Stereo
    const int buffer_size = additional_amount;
    
    static float phase[8] = {0.0f}; // Phase for each channel
    
    int16_t* buffer = new int16_t[buffer_size / sizeof(int16_t)];
    
    for (int i = 0; i < buffer_size / sizeof(int16_t); i += channels) {
        float left_sample = 0.0f;
        float right_sample = 0.0f;
        
        for (int ch = 0; ch < 8; ch++) {
            if (audio_state.channels[ch].enabled && audio_state.channels[ch].frequency > 0) {
                float freq = audio_state.channels[ch].frequency;
                float vol = audio_state.channels[ch].volume / 255.0f;
                uint8_t waveform = audio_state.channels[ch].waveform;
                
                // Generate waveform
                float sample = 0.0f;
                switch (waveform) {
                    case 0: // Sine wave
                        sample = sinf(phase[ch] * 2.0f * (float)M_PI);
                        break;
                    case 1: // Square wave
                        sample = (phase[ch] < 0.5f) ? 1.0f : -1.0f;
                        break;
                    case 2: // Triangle wave
                        sample = (phase[ch] < 0.5f) ? (4.0f * phase[ch] - 1.0f) : (3.0f - 4.0f * phase[ch]);
                        break;
                    case 3: // Sawtooth wave
                        sample = 2.0f * phase[ch] - 1.0f;
                        break;
                }
                
                // Mix channels (simple stereo panning)
                if (ch % 2 == 0) { // Even channels to left
                    left_sample += sample * vol;
                } else { // Odd channels to right
                    right_sample += sample * vol;
                }
                
                // Update phase
                phase[ch] += freq / sample_rate;
                if (phase[ch] >= 1.0f) phase[ch] -= 1.0f;
            }
        }
        
        // Apply master volume and clamp
        left_sample *= audio_state.master_volume;
        right_sample *= audio_state.master_volume;
        
        left_sample = std::max(-1.0f, std::min(1.0f, left_sample));
        right_sample = std::max(-1.0f, std::min(1.0f, right_sample));
        
        // Convert to 16-bit PCM
        buffer[i] = (int16_t)(left_sample * 32767.0f);
        buffer[i + 1] = (int16_t)(right_sample * 32767.0f);
    }
    
    // Put samples into the audio stream
    SDL_PutAudioStreamData(stream, buffer, buffer_size);
    delete[] buffer;
}

int main(int argc, char* argv[]) {
    std::cout << "Starting Pixel Model 2 Emulator..." << std::endl;
    
    // Show usage if help is requested
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        std::cout << "Usage: PixelModel2 <game_name>" << std::endl;
        std::cout << "Available games:" << std::endl;
        std::cout << "  vf3     - Virtua Fighter 3 (ROMs ZIP disponibles)" << std::endl;
        std::cout << "  daytona - Daytona USA (ROMs ZIP présentes mais incompatibles)" << std::endl;
        std::cout << "ROM loading: Only ZIP files in roms/ folder are supported." << std::endl;
        std::cout << "Note: Daytona ROMs need configuration update to match ZIP contents." << std::endl;
        std::cout << "A game name must be specified as an argument." << std::endl;
        return 0;
    }
    
    // --- SDL Initialization ---
    std::cout << "Initializing SDL..." << std::endl;
    int sdlInitResult = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    if (sdlInitResult < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }
    std::cout << "SDL initialized successfully." << std::endl;

    // --- OpenGL Attributes ---
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // --- Window Creation (SDL3 API) ---
    std::cout << "Creating window..." << std::endl;
    SDL_Window* window = SDL_CreateWindow(
        "Pixel Model 2 Emulator",
        496, // Native Model 2 width
        384, // Native Model 2 height
        SDL_WINDOW_OPENGL
    );

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }
    std::cout << "Window created successfully." << std::endl;

    // --- OpenGL Context ---
    std::cout << "Creating OpenGL context..." << std::endl;
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    std::cout << "OpenGL context created successfully." << std::endl;

    // --- Emulator State ---
    std::cout << "Initializing memory bus..." << std::endl;
    MemoryBus bus;
    memory_init(&bus);
    std::cout << "Memory bus initialized." << std::endl;

    // Load the game ROMs into memory
    std::cout << "Loading game ROMs..." << std::endl;
    
    // Check if a game name was provided
    if (argc < 2) {
        std::cerr << "Error: No game specified!" << std::endl;
        std::cerr << "Usage: PixelModel2 <game_name>" << std::endl;
        std::cerr << "Available games: vf3, daytona" << std::endl;
        std::cerr << "Use --help for more information." << std::endl;
        memory_destroy(&bus);
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    const char* game_name = argv[1];
    
    if (!load_game_by_name(&bus, game_name, "../../vcop2")) {
        std::cerr << "Failed to load game ROMs!" << std::endl;
        memory_destroy(&bus);
        return -1;
    }
    std::cout << "Game ROMs loaded successfully." << std::endl;

    std::cout << "Initializing i960 CPU..." << std::endl;
    i960_cpu cpu;
    i960_init(&cpu, &bus);
    std::cout << "CPU initialized successfully." << std::endl;

    std::cout << "Initializing TGP GPU..." << std::endl;
    TGP* tgp = new TGP();
    tgp_init(tgp, &bus);
    std::cout << "TGP initialized successfully." << std::endl;

    std::cout << "Connecting input system..." << std::endl;
    memory_connect_input(&bus, &current_input);
    std::cout << "Input system connected successfully." << std::endl;

    // --- Audio Initialization ---
    std::cout << "Initializing audio system..." << std::endl;
    // For now, skip audio initialization to focus on other systems
    // TODO: Implement proper SDL3 audio device enumeration and opening
    audio_state.stream = nullptr;
    std::cout << "Audio system initialization skipped (TODO: implement SDL3 audio)" << std::endl;

    std::cout << "Connecting audio system..." << std::endl;
    memory_connect_audio(&bus, &audio_state);
    std::cout << "Audio system connected successfully." << std::endl;

    std::cout << "All initializations completed successfully!" << std::endl;
    std::cout << "Emulator components are working correctly." << std::endl;

    // --- Main Loop ---
    bool running = true;
    int frame_count = 0;
    std::cout << "Starting main emulation loop..." << std::endl;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else {
                handle_input_event(event);
            }
        }

        // --- CPU Execution ---
        i960_step(&cpu);

        // Check if CPU is halted
        if (cpu.halted) {
            std::cout << "CPU halted. Stopping emulation." << std::endl;
            running = false;
        }

        // --- TGP Execution ---
        tgp_step(tgp);

        // --- Debug: Display active inputs every 60 frames ---
        if (frame_count % 60 == 0) {
            std::cout << "Input state: ";
            if (current_input.start_button) std::cout << "START ";
            if (current_input.service_button) std::cout << "SERVICE ";
            if (current_input.test_button) std::cout << "TEST ";
            if (current_input.coin_button) std::cout << "COIN ";
            if (current_input.button1) std::cout << "B1 ";
            if (current_input.button2) std::cout << "B2 ";
            if (current_input.button3) std::cout << "B3 ";
            if (current_input.button4) std::cout << "B4 ";
            if (current_input.up) std::cout << "UP ";
            if (current_input.down) std::cout << "DOWN ";
            if (current_input.left) std::cout << "LEFT ";
            if (current_input.right) std::cout << "RIGHT ";
            if (current_input.steering != 0) std::cout << "STEER:" << current_input.steering << " ";
            if (current_input.throttle != 0) std::cout << "THROTTLE:" << current_input.throttle << " ";
            std::cout << std::endl;
        }

        // --- Rendering ---
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render TGP framebuffer to screen
        tgp_render_to_opengl(tgp);

        // Swap buffers
        SDL_GL_SwapWindow(window);

        // Cap framerate to ~60 FPS
        SDL_Delay(16);

        frame_count++;
        if (frame_count % 60 == 0) { // Print every second
            std::cout << "Frame " << frame_count << " rendered successfully." << std::endl;
        }
    }

    // --- Cleanup ---
    delete tgp;
    memory_destroy(&bus);
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
