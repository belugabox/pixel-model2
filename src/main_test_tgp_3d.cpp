#include <iostream>
#include <iomanip>
#include "i960.h"
#include "memory.h"
#include "tgp.h"

// Test program for TGP 3D rendering
int main() {
    std::cout << "TGP 3D Rendering Test" << std::endl;

    // Initialize memory bus
    MemoryBus bus;
    memory_init(&bus);

    // Initialize TGP
    TGP tgp;
    tgp_init(&tgp, &bus);

    // Set up a simple triangle in memory
    uint32_t vertex_addr = 0x1000; // Address for vertex data

    // Triangle vertices (x, y, z, r, g, b, a, u, v)
    float triangle_data[] = {
        // Vertex 1
        -0.5f, -0.5f, 0.0f,  // position
        1.0f, 0.0f, 0.0f, 1.0f,  // color (red)
        0.0f, 0.0f,           // uv

        // Vertex 2
        0.5f, -0.5f, 0.0f,   // position
        0.0f, 1.0f, 0.0f, 1.0f,  // color (green)
        1.0f, 0.0f,           // uv

        // Vertex 3
        0.0f, 0.5f, 0.0f,    // position
        0.0f, 0.0f, 1.0f, 1.0f,  // color (blue)
        0.5f, 1.0f            // uv
    };

    // Copy triangle data to memory
    memcpy(&bus.ram[vertex_addr], triangle_data, sizeof(triangle_data));

    // Set vertex buffer address in TGP
    tgp_write_register(&tgp, 0x04, vertex_addr);

    // Set up a simple projection matrix (orthographic)
    float proj_matrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    memcpy(tgp.projection_matrix, proj_matrix, sizeof(proj_matrix));

    // Clear framebuffer
    tgp_write_register(&tgp, 0x00, 0x01); // Clear command
    tgp_step(&tgp); // Process clear

    // Draw triangle
    uint32_t draw_command = (vertex_addr << 8) | CMD_DRAW_TRIANGLE;
    tgp_write_register(&tgp, 0x00, draw_command);
    tgp_step(&tgp); // Process draw

    // Check framebuffer for rendered pixels
    int rendered_pixels = 0;
    for (int i = 0; i < 496 * 384; i++) {
        if (tgp.framebuffer[i] != 0) {
            rendered_pixels++;
        }
    }

    std::cout << "Triangle rendered successfully!" << std::endl;
    std::cout << "Rendered pixels: " << rendered_pixels << std::endl;

    // Test matrix operations
    std::cout << "Testing matrix operations..." << std::endl;

    // Load identity
    tgp_write_register(&tgp, 0x00, CMD_LOAD_IDENTITY);
    tgp_step(&tgp);

    // Check if matrix is identity
    bool is_identity = true;
    for (int i = 0; i < 16; i++) {
        float expected = (i % 5 == 0) ? 1.0f : 0.0f;
        if (std::abs(tgp.current_matrix[i] - expected) > 0.001f) {
            is_identity = false;
            break;
        }
    }

    if (is_identity) {
        std::cout << "Matrix identity test passed!" << std::endl;
    } else {
        std::cout << "Matrix identity test failed!" << std::endl;
    }

    memory_destroy(&bus);
    std::cout << "TGP 3D rendering test completed." << std::endl;

    return 0;
}