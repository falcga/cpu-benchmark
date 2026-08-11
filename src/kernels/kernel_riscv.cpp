// CPU Benchmark - RISC-V Kernel Implementations
// Optimized kernels for RISC-V architecture (RV32/RV64)
// Supports scalar fallback and RISC-V Vector Extension (RVV) if available

#include "kernel_common.hpp"

namespace kernels {
namespace riscv {

// ============================================================================
// Scalar implementations for RISC-V (baseline - always available)
// ============================================================================

// Memory kernel for float - RISC-V optimized scalar
void mem_float(float* C, const float* A, const float* B,
               float alpha, float beta,
               size_t z_begin, size_t z_end,
               size_t Nx, size_t Ny, size_t /*Nz*/)
{
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                C[i] = alpha * A[i] + beta * B[i];
            }
        }
    }
}

// Memory kernel for double - RISC-V optimized scalar
void mem_double(double* C, const double* A, const double* B,
                double alpha, double beta,
                size_t z_begin, size_t z_end,
                size_t Nx, size_t Ny, size_t /*Nz*/)
{
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                C[i] = alpha * A[i] + beta * B[i];
            }
        }
    }
}

// Stencil kernel for float - RISC-V optimized scalar
void stencil_float(float* C, const float* A,
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

// Stencil kernel for double - RISC-V optimized scalar
void stencil_double(double* C, const double* A,
                    double a0, double a1,
                    size_t z_begin, size_t z_end,
                    size_t Nx, size_t Ny, size_t Nz)
{
    size_t z_start = (z_begin < 1) ? 1 : z_begin;
    size_t z_stop = (z_end > Nz - 1) ? Nz - 1 : z_end;
    
    for (size_t z = z_start; z < z_stop; ++z) {
        for (size_t y = 1; y < Ny - 1; ++y) {
            for (size_t x = 1; x < Nx - 1; ++x) {
                double center = A[idx(x, y, z, Nx, Ny)];
                double neighbors = A[idx(x + 1, y, z, Nx, Ny)] +
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

// Memory kernel for int8 - RISC-V optimized scalar
void mem_int8(int8_t* C, const int8_t* A, const int8_t* B,
              int8_t alpha, int8_t beta,
              size_t z_begin, size_t z_end,
              size_t Nx, size_t Ny, size_t /*Nz*/)
{
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                int32_t result = static_cast<int32_t>(alpha) * static_cast<int32_t>(A[i]) +
                                 static_cast<int32_t>(beta) * static_cast<int32_t>(B[i]);
                // Clamp to int8 range
                if (result > 127) result = 127;
                if (result < -128) result = -128;
                C[i] = static_cast<int8_t>(result);
            }
        }
    }
}

// Stencil kernel for int8 - RISC-V optimized scalar
void stencil_int8(int8_t* C, const int8_t* A,
                  int8_t a0, int8_t a1,
                  size_t z_begin, size_t z_end,
                  size_t Nx, size_t Ny, size_t Nz)
{
    size_t z_start = (z_begin < 1) ? 1 : z_begin;
    size_t z_stop = (z_end > Nz - 1) ? Nz - 1 : z_end;
    
    for (size_t z = z_start; z < z_stop; ++z) {
        for (size_t y = 1; y < Ny - 1; ++y) {
            for (size_t x = 1; x < Nx - 1; ++x) {
                int32_t center = static_cast<int32_t>(A[idx(x, y, z, Nx, Ny)]);
                int32_t neighbors = static_cast<int32_t>(A[idx(x + 1, y, z, Nx, Ny)]) +
                                    static_cast<int32_t>(A[idx(x - 1, y, z, Nx, Ny)]) +
                                    static_cast<int32_t>(A[idx(x, y + 1, z, Nx, Ny)]) +
                                    static_cast<int32_t>(A[idx(x, y - 1, z, Nx, Ny)]) +
                                    static_cast<int32_t>(A[idx(x, y, z + 1, Nx, Ny)]) +
                                    static_cast<int32_t>(A[idx(x, y, z - 1, Nx, Ny)]);
                int32_t result = static_cast<int32_t>(a0) * center + 
                                 static_cast<int32_t>(a1) * neighbors;
                if (result > 127) result = 127;
                if (result < -128) result = -128;
                C[idx(x, y, z, Nx, Ny)] = static_cast<int8_t>(result);
            }
        }
    }
}

// ============================================================================
// RISC-V Vector Extension (RVV) implementations
// These are only available when compiled with -march=rv64gc_v or similar
// ============================================================================

#if defined(__riscv_vector)

#include <riscv_vector.h>

// RVV-optimized memory kernel for float
void mem_float_rvv(float* C, const float* A, const float* B,
                   float alpha, float beta,
                   size_t z_begin, size_t z_end,
                   size_t Nx, size_t Ny, size_t /*Nz*/)
{
    size_t vl = __riscv_vsetvl_e32m1();
    vfloat32m1_t v_alpha = vfmv_v_f_f32m1(alpha, vl);
    vfloat32m1_t v_beta = vfmv_v_f_f32m1(beta, vl);
    
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            size_t x = 0;
            for (; x + vl <= Nx; x += vl) {
                size_t i = idx(x, y, z, Nx, Ny);
                vfloat32m1_t v_a = vle32_v_f32m1(&A[i], vl);
                vfloat32m1_t v_b = vle32_v_f32m1(&B[i], vl);
                vfloat32m1_t v_result = vfadd_vv_f32m1(
                    vfmul_vv_f32m1(v_alpha, v_a, vl),
                    vfmul_vv_f32m1(v_beta, v_b, vl),
                    vl);
                vse32_v_f32m1(&C[i], v_result, vl);
            }
            // Handle remainder
            for (; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                C[i] = alpha * A[i] + beta * B[i];
            }
        }
    }
}

// RVV-optimized memory kernel for double
void mem_double_rvv(double* C, const double* A, const double* B,
                    double alpha, double beta,
                    size_t z_begin, size_t z_end,
                    size_t Nx, size_t Ny, size_t /*Nz*/)
{
    size_t vl = __riscv_vsetvl_e64m1();
    vfloat64m1_t v_alpha = vfmv_v_f_f64m1(alpha, vl);
    vfloat64m1_t v_beta = vfmv_v_f_f64m1(beta, vl);
    
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            size_t x = 0;
            for (; x + vl <= Nx; x += vl) {
                size_t i = idx(x, y, z, Nx, Ny);
                vfloat64m1_t v_a = vle64_v_f64m1(&A[i], vl);
                vfloat64m1_t v_b = vle64_v_f64m1(&B[i], vl);
                vfloat64m1_t v_result = vfadd_vv_f64m1(
                    vfmul_vv_f64m1(v_alpha, v_a, vl),
                    vfmul_vv_f64m1(v_beta, v_b, vl),
                    vl);
                vse64_v_f64m1(&C[i], v_result, vl);
            }
            // Handle remainder
            for (; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                C[i] = alpha * A[i] + beta * B[i];
            }
        }
    }
}

#endif // __riscv_vector

// ============================================================================
// Microcontroller-friendly implementations
// Optimized for small memory footprints and no floating point
// ============================================================================

namespace microcontroller {

// Fixed-point memory kernel for microcontrollers (16.16 fixed point)
void mem_fixed16(float* C, const float* A, const float* B,
                 float alpha, float beta,
                 size_t z_begin, size_t z_end,
                 size_t Nx, size_t Ny, size_t /*Nz*/)
{
    // Convert to fixed-point (16.16 format)
    const int32_t alpha_fp = static_cast<int32_t>(alpha * 65536.0f);
    const int32_t beta_fp = static_cast<int32_t>(beta * 65536.0f);
    
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                int32_t a_fp = static_cast<int32_t>(A[i] * 65536.0f);
                int32_t b_fp = static_cast<int32_t>(B[i] * 65536.0f);
                
                // Multiply and accumulate in fixed-point
                int64_t result = (static_cast<int64_t>(alpha_fp) * a_fp) >> 16;
                result += (static_cast<int64_t>(beta_fp) * b_fp) >> 16;
                
                C[i] = static_cast<float>(result) / 65536.0f;
            }
        }
    }
}

// Integer-only memory kernel for very constrained devices
void mem_int_only(int16_t* C, const int16_t* A, const int16_t* B,
                  int16_t alpha, int16_t beta,
                  size_t z_begin, size_t z_end,
                  size_t Nx, size_t Ny, size_t /*Nz*/)
{
    for (size_t z = z_begin; z < z_end; ++z) {
        for (size_t y = 0; y < Ny; ++y) {
            for (size_t x = 0; x < Nx; ++x) {
                size_t i = idx(x, y, z, Nx, Ny);
                int32_t result = static_cast<int32_t>(alpha) * static_cast<int32_t>(A[i]) +
                                 static_cast<int32_t>(beta) * static_cast<int32_t>(B[i]);
                // Clamp to int16 range
                if (result > 32767) result = 32767;
                if (result < -32768) result = -32768;
                C[i] = static_cast<int16_t>(result);
            }
        }
    }
}

} // namespace microcontroller
} // namespace riscv
} // namespace kernels
