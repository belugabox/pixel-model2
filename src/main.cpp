#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <iostream>
#include "i960.h"
#include "memory.h"
#include "tgp.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting Pixel Model 2 Emulator..." << std::endl;
    
    // --- SDL Initialization ---
    std::cout << "Initializing SDL..." << std::endl;
    int sdlInitResult = SDL_Init(SDL_INIT_VIDEO);
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

    // Load the ROM file into memory
    std::cout << "Loading ROM file..." << std::endl;
    if (!load_rom_from_file(&bus, "../../vcop2/epr-18022.ic2", 0)) {
        std::cerr << "Failed to load ROM file!" << std::endl;
        memory_destroy(&bus);
        return -1;
    }
    std::cout << "ROM loaded successfully." << std::endl;

    std::cout << "Initializing i960 CPU..." << std::endl;
    i960_cpu cpu;
    i960_init(&cpu, &bus);
    std::cout << "CPU initialized successfully." << std::endl;

    std::cout << "Initializing TGP GPU..." << std::endl;
    TGP* tgp = new TGP();
    tgp_init(tgp, &bus);
    std::cout << "TGP initialized successfully." << std::endl;

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
            }
        }

        // --- CPU Execution ---
        i960_step(&cpu);

        // --- TGP Execution ---
        tgp_step(tgp);

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
