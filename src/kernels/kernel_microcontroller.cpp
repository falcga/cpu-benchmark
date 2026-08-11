// CPU Benchmark - Microcontroller Kernel Implementations
// Optimized for embedded systems, Arduino, Raspberry Pi, and other resource-constrained devices

#include "kernel_common.hpp"
#include <algorithm>
#include <climits>

namespace kernels {
namespace microcontroller {

// ============================================================================
// Arduino-compatible implementations
// Designed for ATmega, ESP32, and other common microcontroller platforms
// ============================================================================

// Simple scalar memory kernel for Arduino (no FPU)
// Uses integer arithmetic to avoid floating point on devices without FPU
void mem_arduino(int16_t* C, const int16_t* A, const int16_t* B,
                 int16_t alpha, int16_t beta,
                 size_t z_begin, size_t z_end,
                 size_t Nx, size_t Ny, size_t /*Nz*/)
{
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                // Use int32_t for intermediate calculations to prevent overflow
                int32_t a = static_cast<int32_t>(A[i]);
                int32_t b = static_cast<int32_t>(B[i]);
                int32_t result = (static_cast<int32_t>(alpha) * a) / 256 +
                                 (static_cast<int32_t>(beta) * b) / 256;
                
                // Clamp to int16 range
                if (result > INT16_MAX) result = INT16_MAX;
                if (result < INT16_MIN) result = INT16_MIN;
                C[i] = static_cast<int16_t>(result);
            }
        }
    }
}

// Memory kernel for Arduino with 8-bit values (even more constrained)
void mem_arduino_int8(int8_t* C, const int8_t* A, const int8_t* B,
                      int8_t alpha, int8_t beta,
                      size_t z_begin, size_t z_end,
                      size_t Nx, size_t Ny, size_t /*Nz*/)
{
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                int16_t a = static_cast<int16_t>(A[i]);
                int16_t b = static_cast<int16_t>(B[i]);
                int16_t result = (static_cast<int16_t>(alpha) * a) / 16 +
                                 (static_cast<int16_t>(beta) * b) / 16;
                
                if (result > INT8_MAX) result = INT8_MAX;
                if (result < INT8_MIN) result = INT8_MIN;
                C[i] = static_cast<int8_t>(result);
            }
        }
    }
}

// Stencil kernel for Arduino (3-point stencil to reduce memory access)
void stencil_arduino_3point(int16_t* C, const int16_t* A,
                           int16_t a0, int16_t a1,
                           size_t z_begin, size_t z_end,
                           size_t Nx, size_t Ny, size_t Nz)
{
    size_t z_start = (z_begin < 1) ? 1 : z_begin;
    size_t z_stop = (z_end > Nz - 1) ? Nz - 1 : z_end;
    
    for (size_t z = z_start; z < z_stop; ++z) {
        for (size_t y = 1; y < Ny - 1; ++y) {
            for (size_t x = 1; x < Nx - 1; ++x) {
                // Only use immediate neighbors (3-point instead of 7-point)
                int32_t center = static_cast<int32_t>(A[idx(x, y, z, Nx, Ny)]);
                int32_t neighbors = static_cast<int32_t>(A[idx(x + 1, y, z, Nx, Ny)]) +
                                    static_cast<int32_t>(A[idx(x - 1, y, z, Nx, Ny)]) +
                                    static_cast<int32_t>(A[idx(x, y + 1, z, Nx, Ny)]);
                
                int32_t result = (static_cast<int32_t>(a0) * center) / 256 +
                                 (static_cast<int32_t>(a1) * neighbors) / 256;
                
                if (result > INT16_MAX) result = INT16_MAX;
                if (result < INT16_MIN) result = INT16_MIN;
                C[idx(x, y, z, Nx, Ny)] = static_cast<int16_t>(result);
            }
        }
    }
}

// ============================================================================
// Raspberry Pi specific optimizations
// RPi has NEON on ARMv7/ARMv8, but we provide scalar fallbacks for older models
// ============================================================================

// Memory kernel optimized for Raspberry Pi (ARMv6/7 without NEON)
void mem_rpi_armv6(float* C, const float* A, const float* B,
                   float alpha, float beta,
                   size_t z_begin, size_t z_end,
                   size_t Nx, size_t Ny, size_t /*Nz*/)
{
    // Unroll loops for better performance on RPi
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            size_t x = 0;
            // Process 4 elements at a time (manual unrolling)
            for (; x + 3 < Nx; x += 4) {
                size_t i = idx(x, y, z, Nx, Ny);
                C[i] = alpha * A[i] + beta * B[i];
                C[i + 1] = alpha * A[i + 1] + beta * B[i + 1];
                C[i + 2] = alpha * A[i + 2] + beta * B[i + 2];
                C[i + 3] = alpha * A[i + 3] + beta * B[i + 3];
            }
            // Handle remainder
            for (; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                C[i] = alpha * A[i] + beta * B[i];
            }
        }
    }
}

// Stencil kernel optimized for Raspberry Pi
void stencil_rpi(float* C, const float* A,
                 float a0, float a1,
                 size_t z_begin, size_t z_end,
                 size_t Nx, size_t Ny, size_t Nz)
{
    size_t z_start = (z_begin < 1) ? 1 : z_begin;
    size_t z_stop = (z_end > Nz - 1) ? Nz - 1 : z_end;
    
    for (size_t z = z_start; z < z_stop; ++z) {
        for (size_t y = 1; y < Ny - 1; ++y) {
            size_t x = 1;
            // Process 2 elements at a time with manual unrolling
            for (; x + 1 < Nx - 1; x += 2) {
                // First element
                float center1 = A[idx(x, y, z, Nx, Ny)];
                float neighbors1 = A[idx(x + 1, y, z, Nx, Ny)] +
                                   A[idx(x - 1, y, z, Nx, Ny)] +
                                   A[idx(x, y + 1, z, Nx, Ny)] +
                                   A[idx(x, y - 1, z, Nx, Ny)] +
                                   A[idx(x, y, z + 1, Nx, Ny)] +
                                   A[idx(x, y, z - 1, Nx, Ny)];
                C[idx(x, y, z, Nx, Ny)] = a0 * center1 + a1 * neighbors1;
                
                // Second element
                float center2 = A[idx(x + 1, y, z, Nx, Ny)];
                float neighbors2 = A[idx(x + 2, y, z, Nx, Ny)] +
                                   A[idx(x, y, z, Nx, Ny)] +
                                   A[idx(x + 1, y + 1, z, Nx, Ny)] +
                                   A[idx(x + 1, y - 1, z, Nx, Ny)] +
                                   A[idx(x + 1, y, z + 1, Nx, Ny)] +
                                   A[idx(x + 1, y, z - 1, Nx, Ny)];
                C[idx(x + 1, y, z, Nx, Ny)] = a0 * center2 + a1 * neighbors2;
            }
            // Handle remainder
            for (; x < Nx - 1; ++x) {
                float center = A[idx(x, y, z, Nx, Ny)];
                float neighbors = A[idx(x + 1, y, z, Nx, Ny)] +
                                  A[idx(x - 1, y, z, Nx, Ny)] +
                                  A[idx(x, y + 1, z, Nx, Ny)] +
                                  A[idx(x, y - 1, z, Nx, Ny)] +
                                  A[idx(x, y, z + 1, Nx, Ny)] +
                                  A[idx(x, y, z - 1, Nx, Ny)];
                C[idx(x, y, z, Nx, Ny)] = a0 * center + a1 * neighbors;
            }
        }
    }
}

// ============================================================================
// ESP32 specific optimizations
// ESP32 has dual cores and SIMD instructions
// ============================================================================

// Memory kernel optimized for ESP32 (can use ESP32 SIMD if available)
void mem_esp32(float* C, const float* A, const float* B,
               float alpha, float beta,
               size_t z_begin, size_t z_end,
               size_t Nx, size_t Ny, size_t /*Nz*/)
{
    // ESP32 benefits from loop unrolling and cache-friendly access
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            size_t x = 0;
            // Process in chunks of 8 for better cache utilization
            for (; x + 7 < Nx; x += 8) {
                size_t base = idx(x, y, z, Nx, Ny);
                for (int i = 0; i < 8; ++i) {
                    C[base + i] = alpha * A[base + i] + beta * B[base + i];
                }
            }
            // Handle remainder
            for (; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                C[i] = alpha * A[i] + beta * B[i];
            }
        }
    }
}

// ============================================================================
// Legacy x86 fallback (for very old CPUs without SSE2)
// This is for ancient hardware like Pentium III and earlier
// ============================================================================

namespace legacy_x86 {

// Memory kernel for very old x86 (no SIMD at all)
void mem_float_legacy(float* C, const float* A, const float* B,
                      float alpha, float beta,
                      size_t z_begin, size_t z_end,
                      size_t Nx, size_t Ny, size_t /*Nz*/)
{
    // Simple scalar implementation with minimal assumptions
    volatile float temp_alpha = alpha; // Prevent excessive optimization
    volatile float temp_beta = beta;
    
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                C[i] = temp_alpha * A[i] + temp_beta * B[i];
            }
        }
    }
}

// Stencil kernel for very old x86
void stencil_float_legacy(float* C, const float* A,
                          float a0, float a1,
                          size_t z_begin, size_t z_end,
                          size_t Nx, size_t Ny, size_t Nz)
{
    volatile float temp_a0 = a0;
    volatile float temp_a1 = a1;
    
    size_t z_start = (z_begin < 1) ? 1 : z_begin;
    size_t z_stop = (z_end > Nz - 1) ? Nz - 1 : z_end;
    
    for (size_t z = z_start; z < z_stop; ++z) {
        for (size_t y = 1; y < Ny - 1; ++y) {
            for (size_t x = 1; x < Nx - 1; ++x) {
                float center = A[idx(x, y, z, Nx, Ny)];
                float neighbors = A[idx(x + 1, y, z, Nx, Ny)] +
                                  A[idx(x - 1, y, z, Nx, Ny)] +
                                  A[idx(x, y + 1, z, Nx, Ny)] +
                                  A[idx(x, y - 1, z, Nx, Ny)] +
                                  A[idx(x, y, z + 1, Nx, Ny)] +
                                  A[idx(x, y, z - 1, Nx, Ny)];
                C[idx(x, y, z, Nx, Ny)] = temp_a0 * center + temp_a1 * neighbors;
            }
        }
    }
}

} // namespace legacy_x86

// ============================================================================
// Fallback implementations for any platform
// These will always work, even on obscure hardware
// ============================================================================

// Universal scalar memory kernel (works everywhere)
void mem_universal(float* C, const float* A, const float* B,
                   float alpha, float beta,
                   size_t z_begin, size_t z_end,
                   size_t Nx, size_t Ny, size_t /*Nz*/)
{
    // Portable implementation that works on any architecture
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                // Direct computation without any architecture-specific optimizations
                C[i] = alpha * A[i] + beta * B[i];
            }
        }
    }
}

// Universal scalar stencil kernel
void stencil_universal(float* C, const float* A,
                       float a0, float a1,
                       size_t z_begin, size_t z_end,
                       size_t Nx, size_t Ny, size_t Nz)
{
    size_t z_start = (z_begin < 1) ? 1 : z_begin;
    size_t z_stop = (z_end > Nz - 1) ? Nz - 1 : z_end;
    
    for (size_t z = z_start; z < z_stop; ++z) {
        for (size_t y = 1; y < Ny - 1; ++y) {
            for (size_t x = 1; x < Nx - 1; ++x) {
                float center = A[idx(x, y, z, Nx, Ny)];
                // Sum all 6 neighbors
                float sum = 0.0f;
                sum += A[idx(x + 1, y, z, Nx, Ny)];
                sum += A[idx(x - 1, y, z, Nx, Ny)];
                sum += A[idx(x, y + 1, z, Nx, Ny)];
                sum += A[idx(x, y - 1, z, Nx, Ny)];
                sum += A[idx(x, y, z + 1, Nx, Ny)];
                sum += A[idx(x, y, z - 1, Nx, Ny)];
                C[idx(x, y, z, Nx, Ny)] = a0 * center + a1 * sum;
            }
        }
    }
}

// Universal int8 memory kernel
void mem_universal_int8(int8_t* C, const int8_t* A, const int8_t* B,
                         int8_t alpha, int8_t beta,
                         size_t z_begin, size_t z_end,
                         size_t Nx, size_t Ny, size_t /*Nz*/)
{
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                int16_t a = static_cast<int16_t>(A[i]);
                int16_t b = static_cast<int16_t>(B[i]);
                int16_t result = (static_cast<int16_t>(alpha) * a + 
                                 static_cast<int16_t>(beta) * b) / 128;
                
                // Safe clamping
                if (result > 127) result = 127;
                else if (result < -128) result = -128;
                C[i] = static_cast<int8_t>(result);
            }
        }
    }
}

// Universal int8 stencil kernel
void stencil_universal_int8(int8_t* C, const int8_t* A,
                             int8_t a0, int8_t a1,
                             size_t z_begin, size_t z_end,
                             size_t Nx, size_t Ny, size_t Nz)
{
    size_t z_start = (z_begin < 1) ? 1 : z_begin;
    size_t z_stop = (z_end > Nz - 1) ? Nz - 1 : z_end;
    
    for (size_t z = z_start; z < z_stop; ++z) {
        for (size_t y = 1; y < Ny - 1; ++y) {
            for (size_t x = 1; x < Nx - 1; ++x) {
                int16_t center = static_cast<int16_t>(A[idx(x, y, z, Nx, Ny)]);
                int16_t neighbors = static_cast<int16_t>(A[idx(x + 1, y, z, Nx, Ny)]) +
                                    static_cast<int16_t>(A[idx(x - 1, y, z, Nx, Ny)]) +
                                    static_cast<int16_t>(A[idx(x, y + 1, z, Nx, Ny)]) +
                                    static_cast<int16_t>(A[idx(x, y - 1, z, Nx, Ny)]) +
                                    static_cast<int16_t>(A[idx(x, y, z + 1, Nx, Ny)]) +
                                    static_cast<int16_t>(A[idx(x, y, z - 1, Nx, Ny)]);
                
                int16_t result = (static_cast<int16_t>(a0) * center + 
                                 static_cast<int16_t>(a1) * neighbors) / 256;
                
                if (result > 127) result = 127;
                else if (result < -128) result = -128;
                C[idx(x, y, z, Nx, Ny)] = static_cast<int8_t>(result);
            }
        }
    }
}

} // namespace microcontroller
} // namespace kernels
