#include "tgp.h"
#include "memory.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

// OpenGL includes for rendering
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

void tgp_init(TGP* tgp, MemoryBus* bus) {
    std::cout << "Initializing TGP (Tile Generator Processor)..." << std::endl;

    // Initialize registers
    tgp->control_register = 0;
    tgp->vertex_buffer_addr = 0;
    tgp->index_buffer_addr = 0;
    tgp->texture_base_addr = 0;
    tgp->matrix_sp = 0;
    memset(tgp->matrix_stack, 0, sizeof(tgp->matrix_stack));

    // Initialize state
    tgp->busy = false;
    tgp->current_command = 0;
    tgp->bus = bus;

    // Initialize viewport (Model 2 native resolution)
    tgp->viewport_x = 0;
    tgp->viewport_y = 0;
    tgp->viewport_width = 496;
    tgp->viewport_height = 384;

    // Initialize matrices to identity
    tgp_matrix_identity(tgp->projection_matrix);
    tgp_matrix_identity(tgp->modelview_matrix);
    tgp_matrix_identity(tgp->current_matrix);

    // Initialize framebuffer and depth buffer
    memset(tgp->framebuffer, 0, sizeof(tgp->framebuffer));
    for (int i = 0; i < 496 * 384; i++) {
        tgp->depth_buffer[i] = 1.0f; // Far plane
    }

    std::cout << "TGP initialized successfully." << std::endl;
}

void tgp_reset(TGP* tgp) {
    tgp->control_register = 0;
    tgp->busy = false;
    tgp->current_command = 0;
    std::cout << "TGP reset." << std::endl;
}

void tgp_step(TGP* tgp) {
    // Process TGP commands if busy
    if (tgp->busy) {
        // For now, just mark as not busy after one cycle
        // In a real implementation, this would process actual 3D commands
        tgp->busy = false;
        std::cout << "TGP command completed." << std::endl;
    }
}

void tgp_execute_command(TGP* tgp) {
    // Extract command from control register (bits 8-15 typically contain command)
    uint32_t command = (tgp->control_register >> 8) & 0xFF;
    
    switch (command) {
        case CMD_CLEAR:
            std::cout << "TGP: Executing CLEAR command" << std::endl;
            tgp_clear_framebuffer(tgp);
            break;
        case CMD_DRAW_TRIANGLE:
            std::cout << "TGP: Executing DRAW_TRIANGLE command" << std::endl;
            tgp_draw_triangles(tgp);
            break;
        case CMD_SET_MATRIX:
            std::cout << "TGP: Executing SET_MATRIX command" << std::endl;
            tgp_load_matrix_from_memory(tgp);
            break;
        case CMD_PUSH_MATRIX:
            std::cout << "TGP: Executing PUSH_MATRIX command" << std::endl;
            tgp_push_matrix(tgp);
            break;
        case CMD_POP_MATRIX:
            std::cout << "TGP: Executing POP_MATRIX command" << std::endl;
            tgp_pop_matrix(tgp);
            break;
        case CMD_LOAD_IDENTITY:
            std::cout << "TGP: Executing LOAD_IDENTITY command" << std::endl;
            tgp_matrix_identity(tgp->current_matrix);
            break;
        case CMD_MULTIPLY_MATRIX:
            std::cout << "TGP: Executing MULTIPLY_MATRIX command" << std::endl;
            tgp_multiply_matrix(tgp);
            break;
        case CMD_TRANSLATE:
            std::cout << "TGP: Executing TRANSLATE command" << std::endl;
            tgp_translate_matrix(tgp);
            break;
        case CMD_ROTATE_X:
            std::cout << "TGP: Executing ROTATE_X command" << std::endl;
            tgp_rotate_matrix_x(tgp);
            break;
        case CMD_ROTATE_Y:
            std::cout << "TGP: Executing ROTATE_Y command" << std::endl;
            tgp_rotate_matrix_y(tgp);
            break;
        case CMD_ROTATE_Z:
            std::cout << "TGP: Executing ROTATE_Z command" << std::endl;
            tgp_rotate_matrix_z(tgp);
            break;
        default:
            std::cout << "TGP: Unknown command 0x" << std::hex << command << std::endl;
            break;
    }
    
    // Mark command as completed
    tgp->busy = false;
    tgp->control_register &= ~0x1; // Clear start bit
}

uint32_t tgp_read_register(TGP* tgp, uint32_t offset) {
    switch (offset) {
        case 0x00: return tgp->control_register;
        case 0x04: return tgp->vertex_buffer_addr;
        case 0x08: return tgp->index_buffer_addr;
        case 0x0C: return tgp->texture_base_addr;
        default:
            std::cout << "TGP: Read from unknown register 0x" << std::hex << offset << std::endl;
            return 0;
    }
}

void tgp_write_register(TGP* tgp, uint32_t offset, uint32_t value) {
    switch (offset) {
        case 0x00: // Control register
            tgp->control_register = value;
            if (value & 0x1) { // Start command bit
                tgp->busy = true;
                std::cout << "TGP: Starting command execution" << std::endl;
            }
            break;
        case 0x04: // Vertex buffer address
            tgp->vertex_buffer_addr = value;
            std::cout << "TGP: Vertex buffer set to 0x" << std::hex << value << std::endl;
            break;
        case 0x08: // Index buffer address
            tgp->index_buffer_addr = value;
            std::cout << "TGP: Index buffer set to 0x" << std::hex << value << std::endl;
            break;
        case 0x0C: // Texture base address
            tgp->texture_base_addr = value;
            std::cout << "TGP: Texture base set to 0x" << std::hex << value << std::endl;
            break;
        default:
            std::cout << "TGP: Write to unknown register 0x" << std::hex << offset
                      << " = 0x" << value << std::endl;
            break;
    }
}

void tgp_process_command(TGP* tgp, uint32_t command) {
    // This function is called when a command is written to the control register
    tgp->current_command = command;
    tgp_execute_command(tgp);
}

// New 3D Pipeline Functions

void tgp_clear_framebuffer(TGP* tgp) {
    memset(tgp->framebuffer, 0, sizeof(tgp->framebuffer));
    for (int i = 0; i < 496 * 384; i++) {
        tgp->depth_buffer[i] = 1.0f; // Far plane
    }
    std::cout << "TGP: Framebuffer cleared" << std::endl;
}

void tgp_load_matrix_from_memory(TGP* tgp) {
    // Load 4x4 matrix from memory address stored in vertex_buffer_addr
    uint32_t addr = tgp->vertex_buffer_addr;
    for (int i = 0; i < 16; i++) {
        uint32_t value = memory_read_dword(tgp->bus, addr + i * 4);
        tgp->current_matrix[i] = *(float*)&value; // Convert uint32_t to float
    }
    std::cout << "TGP: Matrix loaded from memory address 0x" << std::hex << addr << std::endl;
}

void tgp_push_matrix(TGP* tgp) {
    if (tgp->matrix_sp < 32) {
        memcpy(&tgp->matrix_stack[tgp->matrix_sp * 16], tgp->current_matrix, sizeof(float) * 16);
        tgp->matrix_sp++;
        std::cout << "TGP: Matrix pushed to stack (SP=" << tgp->matrix_sp << ")" << std::endl;
    } else {
        std::cout << "TGP: Matrix stack overflow!" << std::endl;
    }
}

void tgp_pop_matrix(TGP* tgp) {
    if (tgp->matrix_sp > 0) {
        tgp->matrix_sp--;
        memcpy(tgp->current_matrix, &tgp->matrix_stack[tgp->matrix_sp * 16], sizeof(float) * 16);
        std::cout << "TGP: Matrix popped from stack (SP=" << tgp->matrix_sp << ")" << std::endl;
    } else {
        std::cout << "TGP: Matrix stack underflow!" << std::endl;
    }
}

void tgp_multiply_matrix(TGP* tgp) {
    float temp[16];
    memcpy(temp, tgp->current_matrix, sizeof(float) * 16);
    tgp_matrix_multiply(tgp->current_matrix, temp, tgp->modelview_matrix);
    std::cout << "TGP: Matrix multiplied with modelview" << std::endl;
}

void tgp_translate_matrix(TGP* tgp) {
    // Get translation parameters from index_buffer_addr (x, y, z)
    uint32_t addr = tgp->index_buffer_addr;
    uint32_t x_bits = memory_read_dword(tgp->bus, addr);
    uint32_t y_bits = memory_read_dword(tgp->bus, addr + 4);
    uint32_t z_bits = memory_read_dword(tgp->bus, addr + 8);
    
    float x = *(float*)&x_bits;
    float y = *(float*)&y_bits;
    float z = *(float*)&z_bits;
    
    tgp_matrix_translate(tgp->current_matrix, x, y, z);
    std::cout << "TGP: Matrix translated by (" << x << ", " << y << ", " << z << ")" << std::endl;
}

void tgp_rotate_matrix_x(TGP* tgp) {
    uint32_t addr = tgp->index_buffer_addr;
    uint32_t angle_bits = memory_read_dword(tgp->bus, addr);
    float angle = *(float*)&angle_bits;
    
    tgp_matrix_rotate_x(tgp->current_matrix, angle);
    std::cout << "TGP: Matrix rotated X by " << angle << " radians" << std::endl;
}

void tgp_rotate_matrix_y(TGP* tgp) {
    uint32_t addr = tgp->index_buffer_addr;
    uint32_t angle_bits = memory_read_dword(tgp->bus, addr);
    float angle = *(float*)&angle_bits;
    
    tgp_matrix_rotate_y(tgp->current_matrix, angle);
    std::cout << "TGP: Matrix rotated Y by " << angle << " radians" << std::endl;
}

void tgp_rotate_matrix_z(TGP* tgp) {
    uint32_t addr = tgp->index_buffer_addr;
    uint32_t angle_bits = memory_read_dword(tgp->bus, addr);
    float angle = *(float*)&angle_bits;
    
    tgp_matrix_rotate_z(tgp->current_matrix, angle);
    std::cout << "TGP: Matrix rotated Z by " << angle << " radians" << std::endl;
}

void tgp_draw_triangles(TGP* tgp) {
    // Simplified triangle drawing - in a real implementation this would be much more complex
    // For now, just draw some test triangles to demonstrate the 3D pipeline
    
    // Create a simple test triangle
    Vertex v1 = {100.0f, 100.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
    Vertex v2 = {200.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};
    Vertex v3 = {150.0f, 200.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 1.0f};
    
    // Transform vertices (simplified - just apply current matrix)
    tgp_transform_vertex(&v1, tgp->current_matrix);
    tgp_transform_vertex(&v2, tgp->current_matrix);
    tgp_transform_vertex(&v3, tgp->current_matrix);
    
    // Draw the triangle to framebuffer
    tgp_rasterize_triangle(tgp, v1, v2, v3);
    
    std::cout << "TGP: Test triangle drawn" << std::endl;
}

void tgp_transform_vertex(Vertex* v, const float matrix[16]) {
    // Apply 4x4 transformation matrix to vertex position
    float x = v->x * matrix[0] + v->y * matrix[4] + v->z * matrix[8] + matrix[12];
    float y = v->x * matrix[1] + v->y * matrix[5] + v->z * matrix[9] + matrix[13];
    float z = v->x * matrix[2] + v->y * matrix[6] + v->z * matrix[10] + matrix[14];
    float w = v->x * matrix[3] + v->y * matrix[7] + v->z * matrix[11] + matrix[15];
    
    // Perspective divide
    if (w != 0.0f) {
        v->x = x / w;
        v->y = y / w;
        v->z = z / w;
    }
}

void tgp_rasterize_triangle(TGP* tgp, const Vertex& v1, const Vertex& v2, const Vertex& v3) {
    // Very basic triangle rasterization - just fill with solid color for now
    // In a real implementation, this would include proper interpolation, clipping, etc.
    
    // Convert to screen coordinates (simple orthographic projection)
    int x1 = (int)(v1.x + tgp->viewport_width / 2);
    int y1 = (int)(v1.y + tgp->viewport_height / 2);
    int x2 = (int)(v2.x + tgp->viewport_width / 2);
    int y2 = (int)(v2.y + tgp->viewport_height / 2);
    int x3 = (int)(v3.x + tgp->viewport_width / 2);
    int y3 = (int)(v3.y + tgp->viewport_height / 2);
    
    // Simple bounding box fill (very inefficient but works for demonstration)
    int min_x = std::min({x1, x2, x3});
    int max_x = std::max({x1, x2, x3});
    int min_y = std::min({y1, y2, y3});
    int max_y = std::max({y1, y2, y3});
    
    // Clamp to viewport
    min_x = std::max(0, min_x);
    max_x = std::min((int)tgp->viewport_width - 1, max_x);
    min_y = std::max(0, min_y);
    max_y = std::min((int)tgp->viewport_height - 1, max_y);
    
    // Fill bounding box with average color
    uint8_t r = (uint8_t)((v1.r + v2.r + v3.r) / 3.0f * 255.0f);
    uint8_t g = (uint8_t)((v1.g + v2.g + v3.g) / 3.0f * 255.0f);
    uint8_t b = (uint8_t)((v1.b + v2.b + v3.b) / 3.0f * 255.0f);
    uint8_t a = (uint8_t)((v1.a + v2.a + v3.a) / 3.0f * 255.0f);
    uint32_t color = (r << 24) | (g << 16) | (b << 8) | a;
    
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            int index = y * tgp->viewport_width + x;
            tgp->framebuffer[index] = color;
        }
    }
}



// Matrix Operations

void tgp_draw_triangle(TGP* tgp, uint32_t vertex_addr) {
    std::cout << "TGP: Drawing triangle from vertices at 0x" << std::hex << vertex_addr << std::endl;

    Triangle triangle;

    // Read vertex data from memory (assuming 3 vertices, each with x,y,z,r,g,b,a,u,v)
    for (int i = 0; i < 3; i++) {
        uint32_t base_addr = vertex_addr + (i * 9 * 4); // 9 floats per vertex

        // Read vertex data (simplified - in real Model 2 this would be more complex)
        triangle.vertices[i].x = *reinterpret_cast<float*>(&tgp->bus->ram[base_addr]);
        triangle.vertices[i].y = *reinterpret_cast<float*>(&tgp->bus->ram[base_addr + 4]);
        triangle.vertices[i].z = *reinterpret_cast<float*>(&tgp->bus->ram[base_addr + 8]);
        triangle.vertices[i].r = *reinterpret_cast<float*>(&tgp->bus->ram[base_addr + 12]);
        triangle.vertices[i].g = *reinterpret_cast<float*>(&tgp->bus->ram[base_addr + 16]);
        triangle.vertices[i].b = *reinterpret_cast<float*>(&tgp->bus->ram[base_addr + 20]);
        triangle.vertices[i].a = *reinterpret_cast<float*>(&tgp->bus->ram[base_addr + 24]);
        triangle.vertices[i].u = *reinterpret_cast<float*>(&tgp->bus->ram[base_addr + 28]);
        triangle.vertices[i].v = *reinterpret_cast<float*>(&tgp->bus->ram[base_addr + 32]);
    }

    // Process the triangle through the 3D pipeline
    tgp_render_triangle(tgp, triangle);
}

void tgp_set_matrix(TGP* tgp, uint32_t matrix_addr) {
    std::cout << "TGP: Setting matrix from 0x" << std::hex << matrix_addr << std::endl;

    // Read 4x4 matrix from memory
    for (int i = 0; i < 16; i++) {
        tgp->current_matrix[i] = *reinterpret_cast<float*>(&tgp->bus->ram[matrix_addr + (i * 4)]);
    }
}

// Helper function: Check if point is inside triangle using barycentric coordinates
static bool tgp_is_point_in_triangle(float px, float py, const Triangle& triangle) {
    const Vertex& v0 = triangle.vertices[0];
    const Vertex& v1 = triangle.vertices[1];
    const Vertex& v2 = triangle.vertices[2];

    // Compute barycentric coordinates
    float denom = (v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y);
    if (std::abs(denom) < 1e-6f) return false; // Degenerate triangle

    float a = ((v1.y - v2.y) * (px - v2.x) + (v2.x - v1.x) * (py - v2.y)) / denom;
    float b = ((v2.y - v0.y) * (px - v2.x) + (v0.x - v2.x) * (py - v2.y)) / denom;
    float c = 1.0f - a - b;

    return (a >= 0.0f && a <= 1.0f) &&
           (b >= 0.0f && b <= 1.0f) &&
           (c >= 0.0f && c <= 1.0f);
}

// Helper function: Interpolate depth using barycentric coordinates
static float tgp_interpolate_depth(float px, float py, const Triangle& triangle) {
    const Vertex& v0 = triangle.vertices[0];
    const Vertex& v1 = triangle.vertices[1];
    const Vertex& v2 = triangle.vertices[2];

    // Compute barycentric coordinates
    float denom = (v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y);
    if (std::abs(denom) < 1e-6f) return v0.z; // Degenerate triangle

    float a = ((v1.y - v2.y) * (px - v2.x) + (v2.x - v1.x) * (py - v2.y)) / denom;
    float b = ((v2.y - v0.y) * (px - v2.x) + (v0.x - v2.x) * (py - v2.y)) / denom;
    float c = 1.0f - a - b;

    return a * v0.z + b * v1.z + c * v2.z;
}

// Helper function: Interpolate vertex attribute using barycentric coordinates
static float tgp_interpolate_attribute(float px, float py, const Triangle& triangle,
                                      float attr0, float attr1, float attr2) {
    const Vertex& v0 = triangle.vertices[0];
    const Vertex& v1 = triangle.vertices[1];
    const Vertex& v2 = triangle.vertices[2];

    // Compute barycentric coordinates
    float denom = (v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y);
    if (std::abs(denom) < 1e-6f) return attr0; // Degenerate triangle

    float a = ((v1.y - v2.y) * (px - v2.x) + (v2.x - v1.x) * (py - v2.y)) / denom;
    float b = ((v2.y - v0.y) * (px - v2.x) + (v0.x - v2.x) * (py - v2.y)) / denom;
    float c = 1.0f - a - b;

    return a * attr0 + b * attr1 + c * attr2;
}

// 3D Rendering Pipeline Implementation

void tgp_render_triangle(TGP* tgp, const Triangle& triangle) {
    Triangle transformed_triangle = triangle;

    // Transform vertices
    for (int i = 0; i < 3; i++) {
        tgp_transform_vertex(tgp, transformed_triangle.vertices[i]);
    }

    // Clip triangle against view frustum
    if (!tgp_clip_triangle(tgp, transformed_triangle)) {
        return; // Triangle is completely outside frustum
    }

    // Rasterize the triangle
    tgp_rasterize_triangle(tgp, transformed_triangle);
}

void tgp_transform_vertex(TGP* tgp, Vertex& vertex) {
    // Apply modelview transformation
    float temp[4] = {vertex.x, vertex.y, vertex.z, 1.0f};
    float result[4] = {0, 0, 0, 0};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i] += tgp->current_matrix[i * 4 + j] * temp[j];
        }
    }

    vertex.x = result[0] / result[3];
    vertex.y = result[1] / result[3];
    vertex.z = result[2] / result[3];

    // Apply projection transformation
    temp[0] = vertex.x; temp[1] = vertex.y; temp[2] = vertex.z; temp[3] = 1.0f;
    result[0] = result[1] = result[2] = result[3] = 0;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i] += tgp->projection_matrix[i * 4 + j] * temp[j];
        }
    }

    vertex.x = result[0] / result[3];
    vertex.y = result[1] / result[3];
    vertex.z = result[2] / result[3];

    // Convert to screen coordinates
    vertex.x = (vertex.x + 1.0f) * 0.5f * tgp->viewport_width;
    vertex.y = (1.0f - vertex.y) * 0.5f * tgp->viewport_height;
}

bool tgp_clip_triangle(TGP* tgp, Triangle& triangle) {
    // Simple clipping against near/far planes and viewport bounds
    // In a real implementation, this would be more sophisticated

    for (int i = 0; i < 3; i++) {
        // Clip against near plane (z < -1)
        if (triangle.vertices[i].z < -1.0f) {
            return false;
        }
        // Clip against far plane (z > 1)
        if (triangle.vertices[i].z > 1.0f) {
            return false;
        }
        // Clip against viewport
        if (triangle.vertices[i].x < 0 || triangle.vertices[i].x >= tgp->viewport_width ||
            triangle.vertices[i].y < 0 || triangle.vertices[i].y >= tgp->viewport_height) {
            return false;
        }
    }

    return true;
}

void tgp_rasterize_triangle(TGP* tgp, const Triangle& triangle) {
    // Simple triangle rasterization using barycentric coordinates
    // Find bounding box
    float min_x = std::min({triangle.vertices[0].x, triangle.vertices[1].x, triangle.vertices[2].x});
    float max_x = std::max({triangle.vertices[0].x, triangle.vertices[1].x, triangle.vertices[2].x});
    float min_y = std::min({triangle.vertices[0].y, triangle.vertices[1].y, triangle.vertices[2].y});
    float max_y = std::max({triangle.vertices[0].y, triangle.vertices[1].y, triangle.vertices[2].y});

    // Clamp to viewport
    min_x = std::max(min_x, 0.0f);
    max_x = std::min(max_x, static_cast<float>(tgp->viewport_width - 1));
    min_y = std::max(min_y, 0.0f);
    max_y = std::min(max_y, static_cast<float>(tgp->viewport_height - 1));

    // Rasterize pixels
    for (int y = static_cast<int>(min_y); y <= static_cast<int>(max_y); y++) {
        for (int x = static_cast<int>(min_x); x <= static_cast<int>(max_x); x++) {
            // Check if pixel is inside triangle using barycentric coordinates
            if (tgp_is_point_in_triangle(x + 0.5f, y + 0.5f, triangle)) {
                // Interpolate depth
                float depth = tgp_interpolate_depth(x + 0.5f, y + 0.5f, triangle);

                // Interpolate color
                float r = tgp_interpolate_attribute(x + 0.5f, y + 0.5f, triangle,
                    triangle.vertices[0].r, triangle.vertices[1].r, triangle.vertices[2].r);
                float g = tgp_interpolate_attribute(x + 0.5f, y + 0.5f, triangle,
                    triangle.vertices[0].g, triangle.vertices[1].g, triangle.vertices[2].g);
                float b = tgp_interpolate_attribute(x + 0.5f, y + 0.5f, triangle,
                    triangle.vertices[0].b, triangle.vertices[1].b, triangle.vertices[2].b);

                // Convert to 32-bit RGBA
                uint32_t color = (static_cast<uint32_t>(r * 255) << 24) |
                                (static_cast<uint32_t>(g * 255) << 16) |
                                (static_cast<uint32_t>(b * 255) << 8) |
                                static_cast<uint32_t>(triangle.vertices[0].a * 255);

                // Draw pixel with depth test
                tgp_draw_pixel(tgp, x, y, depth, color);
            }
        }
    }
}

void tgp_draw_pixel(TGP* tgp, int x, int y, float z, uint32_t color) {
    int index = y * tgp->viewport_width + x;

    // Depth test
    if (z < tgp->depth_buffer[index]) {
        tgp->depth_buffer[index] = z;
        tgp->framebuffer[index] = color;
    }
}

// Matrix Operations

void tgp_matrix_multiply(float result[16], const float a[16], const float b[16]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                result[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
            }
        }
    }
}

void tgp_matrix_identity(float matrix[16]) {
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

void tgp_matrix_translate(float matrix[16], float x, float y, float z) {
    matrix[12] += x;
    matrix[13] += y;
    matrix[14] += z;
}

void tgp_matrix_scale(float matrix[16], float sx, float sy, float sz) {
    matrix[0] *= sx;
    matrix[5] *= sy;
    matrix[10] *= sz;
}

void tgp_matrix_rotate_x(float matrix[16], float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    float temp[16];
    memcpy(temp, matrix, sizeof(float) * 16);

    matrix[5] = temp[5] * c - temp[9] * s;
    matrix[6] = temp[6] * c - temp[10] * s;
    matrix[9] = temp[5] * s + temp[9] * c;
    matrix[10] = temp[6] * s + temp[10] * c;
}

void tgp_matrix_rotate_y(float matrix[16], float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    float temp[16];
    memcpy(temp, matrix, sizeof(float) * 16);

    matrix[0] = temp[0] * c + temp[8] * s;
    matrix[2] = temp[2] * c + temp[10] * s;
    matrix[8] = -temp[0] * s + temp[8] * c;
    matrix[10] = -temp[2] * s + temp[10] * c;
}

void tgp_matrix_rotate_z(float matrix[16], float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    float temp[16];
    memcpy(temp, matrix, sizeof(float) * 16);

    matrix[0] = temp[0] * c - temp[4] * s;
    matrix[1] = temp[1] * c - temp[5] * s;
    matrix[4] = temp[0] * s + temp[4] * c;
    matrix[5] = temp[1] * s + temp[5] * c;
}

// OpenGL Rendering

void tgp_render_to_opengl(TGP* tgp) {
    // Render the TGP framebuffer to OpenGL
    glViewport(0, 0, tgp->viewport_width, tgp->viewport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, tgp->viewport_width, tgp->viewport_height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    // Draw pixels from framebuffer
    glBegin(GL_POINTS);
    for (size_t y = 0; y < tgp->viewport_height; y++) {
        for (size_t x = 0; x < tgp->viewport_width; x++) {
            int index = y * tgp->viewport_width + x;
            uint32_t color = tgp->framebuffer[index];

            if (color != 0) { // Only draw non-black pixels
                float r = ((color >> 24) & 0xFF) / 255.0f;
                float g = ((color >> 16) & 0xFF) / 255.0f;
                float b = ((color >> 8) & 0xFF) / 255.0f;
                float a = (color & 0xFF) / 255.0f;

                glColor4f(r, g, b, a);
                glVertex2i(x, y);
            }
        }
    }
    glEnd();
}