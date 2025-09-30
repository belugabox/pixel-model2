#ifndef TGP_H
#define TGP_H

#include <cstdint>
// #include "memory.h"  // Removed to avoid circular dependency

// Forward declaration
struct MemoryBus;

// SEGA Model 2 Tile Generator Processor (TGP) Emulation
// The TGP is the main GPU responsible for 3D rendering

// Vertex structure for 3D rendering
struct Vertex {
    float x, y, z;        // Position
    float r, g, b, a;     // Color
    float u, v;           // Texture coordinates
};

// Triangle structure
struct Triangle {
    Vertex vertices[3];
};

// TGP command types
enum TGPCommand {
    CMD_CLEAR = 0x01,
    CMD_DRAW_TRIANGLE = 0x02,
    CMD_SET_MATRIX = 0x03,
    CMD_PUSH_MATRIX = 0x04,
    CMD_POP_MATRIX = 0x05,
    CMD_LOAD_IDENTITY = 0x06
};

struct TGP {
    // TGP Memory-mapped registers (accessible by i960 CPU)
    uint32_t control_register;     // Control/status register
    uint32_t vertex_buffer_addr;   // Address of vertex buffer in main memory
    uint32_t index_buffer_addr;    // Address of index buffer in main memory
    uint32_t texture_base_addr;    // Base address for textures
    uint32_t matrix_stack[32];     // Matrix stack for transformations
    uint32_t matrix_sp;            // Matrix stack pointer

    // Internal TGP state
    bool busy;                     // True when TGP is processing
    uint32_t current_command;      // Current command being processed

    // Connection to memory bus
    MemoryBus* bus;

    // Rendering state
    uint32_t viewport_x, viewport_y;
    uint32_t viewport_width, viewport_height;
    float projection_matrix[16];
    float modelview_matrix[16];
    float current_matrix[16];      // Combined transformation matrix

    // Framebuffer (simplified - in real Model 2 this would be much more complex)
    uint32_t framebuffer[496 * 384]; // RGBA pixels
    float depth_buffer[496 * 384];    // Depth values
};

// Initialize TGP
void tgp_init(TGP* tgp, MemoryBus* bus);

// Reset TGP to power-on state
void tgp_reset(TGP* tgp);

// Process one TGP command cycle
void tgp_step(TGP* tgp);

// Memory-mapped register access (called by memory bus)
uint32_t tgp_read_register(TGP* tgp, uint32_t offset);
void tgp_write_register(TGP* tgp, uint32_t offset, uint32_t value);

// TGP command processing
void tgp_process_command(TGP* tgp, uint32_t command);

// Basic rendering functions
void tgp_clear_framebuffer(TGP* tgp);
void tgp_draw_triangle(TGP* tgp, uint32_t vertex_addr);
void tgp_set_matrix(TGP* tgp, uint32_t matrix_addr);

// 3D rendering pipeline functions
void tgp_render_triangle(TGP* tgp, const Triangle& triangle);
void tgp_transform_vertex(TGP* tgp, Vertex& vertex);
bool tgp_clip_triangle(TGP* tgp, Triangle& triangle);
void tgp_rasterize_triangle(TGP* tgp, const Triangle& triangle);
void tgp_draw_pixel(TGP* tgp, int x, int y, float z, uint32_t color);

// Matrix operations
void tgp_matrix_multiply(float result[16], const float a[16], const float b[16]);
void tgp_matrix_identity(float matrix[16]);
void tgp_matrix_translate(float matrix[16], float x, float y, float z);
void tgp_matrix_scale(float matrix[16], float sx, float sy, float sz);
void tgp_matrix_rotate_x(float matrix[16], float angle);
void tgp_matrix_rotate_y(float matrix[16], float angle);
void tgp_matrix_rotate_z(float matrix[16], float angle);

// OpenGL rendering functions
void tgp_render_to_opengl(TGP* tgp);

#endif // TGP_H