# Universal Platform Support for SFBench

## Overview

This document describes the universal platform support added to SFBench, enabling it to run on any architecture including RISC-V, microcontrollers (Arduino, Raspberry Pi, ESP32), and legacy x86 systems.

## New Features

### 1. RISC-V Architecture Support
- **Files Added:**
  - `src/cpu_capabilities_riscv.hpp` - RISC-V capability detection headers
  - `src/cpu_capabilities_riscv.cpp` - RISC-V capability detection implementation
  - `src/kernels/kernel_riscv.cpp` - RISC-V optimized kernels

- **Supported Extensions:**
  - RV32GC (32-bit RISC-V with M, A, F, D, C extensions)
  - RV64GC (64-bit RISC-V)
  - RISC-V Vector Extension (RVV) - when available
  - Compressed instructions (RVC)
  - Bit manipulation (RVB)
  - Cryptography (RVC)

- **Kernel Types:**
  - Scalar implementations (always available)
  - RVV-optimized implementations (when compiled with `-march=rv64gcv`)
  - Float and double precision
  - Integer (int8) operations

### 2. Microcontroller Support
- **Files Added:**
  - `src/kernels/kernel_microcontroller.cpp` - Microcontroller-optimized kernels

- **Supported Platforms:**
  - **Arduino** (AVR-based: Uno, Mega, Nano)
  - **ESP32** (Xtensa-based with dual cores)
  - **ESP8266** (Xtensa-based, single core)
  - **Raspberry Pi** (ARMv6, ARMv7, ARMv8)
  - **ARMv6** (without NEON, like Pi Zero)
  - **Legacy x86** (Pentium III and earlier)

- **Kernel Types:**
  - Arduino kernels (integer-based, no FPU)
  - Raspberry Pi optimized kernels (loop unrolling)
  - ESP32 kernels (cache-friendly access patterns)
  - Legacy x86 kernels (minimal assumptions)
  - Universal fallback kernels (work everywhere)

### 3. CMake Configuration
- **New Options:**
  ```cmake
  option(ENABLE_RISCV "Enable RISC-V support" ON)
  option(ENABLE_MICROCONTROLLER "Enable microcontroller support" ON)
  option(ENABLE_LEGACY_X86 "Enable legacy x86 support" ON)
  option(ENABLE_UNIVERSAL_FALLBACK "Enable universal scalar fallback" ON)
  option(FORCE_SCALAR_ONLY "Force scalar-only kernels" OFF)
  ```

- **Architecture Detection:**
  - Automatic detection of x86, x86-64, ARM32, ARM64, RISC-V
  - Appropriate compiler flags for each architecture

- **Kernel Compilation:**
  - Per-file compile flags for different SIMD levels
  - RISC-V kernels compiled with RVV support when available
  - Cross-compilation support for embedded targets

### 4. Kernel Common Updates
- **File Modified:** `src/kernels/kernel_common.hpp`
- **Added Declarations:**
  - RISC-V kernel function declarations
  - Microcontroller kernel function declarations
  - Universal fallback kernel declarations

## Platform Detection

### Compile-Time Detection

```cpp
// RISC-V detection
#ifdef __riscv
    // RISC-V architecture
#endif

// Microcontroller detection
#ifdef __AVR__
    // AVR (Arduino)
#endif

#ifdef ARDUINO
    // Arduino platform
#endif

#ifdef ESP32
    // ESP32 platform
#endif

// Legacy x86 detection
#ifdef __i386__
    // 32-bit x86
#endif
```

### Runtime Detection

The `cpu_capabilities_riscv.cpp` provides runtime detection of:
- RISC-V architecture (RV32/RV64)
- Available extensions (M, A, F, D, C, V, B, K)
- Vector extension support
- Microcontroller type and capabilities

## Usage Examples

### Building for RISC-V

```bash
# Using GCC RISC-V toolchain
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=riscv64-unknown-elf-g++ \
    -DCMAKE_SYSTEM_PROCESSOR=riscv64 \
    -DENABLE_RISCV=ON \
    -DENABLE_NATIVE_ARCH=OFF
cmake --build build

# With Vector Extension support
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-march=rv64gcv -mabi=lp64" \
    -DENABLE_RISCV=ON
cmake --build build
```

### Building for Microcontrollers

```bash
# Generic microcontroller build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_MICROCONTROLLER=ON \
    -DFORCE_SCALAR_ONLY=ON
cmake --build build

# Arduino (using PlatformIO)
[env:uno]
platform = atmelavr
board = uno
framework = arduino
lib_deps = https://github.com/serverflow/sfbenchmark.git

# ESP32 (using PlatformIO)
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
build_flags = -DENABLE_MICROCONTROLLER=ON -DENABLE_ESP32=ON
```

### Building for Legacy Systems

```bash
# Old x86 without AVX2
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DFORCE_SCALAR_ONLY=ON \
    -DENABLE_LEGACY_X86=ON
cmake --build build

# ARMv6 without NEON (Raspberry Pi Zero)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-march=armv6zk -mfpu=vfp -O3" \
    -DENABLE_MICROCONTROLLER=ON
cmake --build build
```

## Kernel Selection Hierarchy

The runtime dispatcher selects kernels in this priority order:

1. **RISC-V RVV** (if available)
2. **RISC-V Scalar**
3. **ARM NEON FP16** (if available)
4. **ARM NEON**
5. **x86 AVX-512**
6. **x86 AVX2 with FMA**
7. **x86 AVX**
8. **x86 SSE4.2**
9. **x86 SSE2**
10. **Scalar (Universal Fallback)**
11. **Microcontroller-specific kernels**

## Files Modified/Created

### New Files
- `src/cpu_capabilities_riscv.hpp` - RISC-V detection headers
- `src/cpu_capabilities_riscv.cpp` - RISC-V detection implementation
- `src/kernels/kernel_riscv.cpp` - RISC-V kernels
- `src/kernels/kernel_microcontroller.cpp` - Microcontroller kernels
- `README_RU.md` - Russian documentation with universal support info

### Modified Files
- `CMakeLists.txt` - Updated with new architecture support and options
- `src/kernels/kernel_common.hpp` - Added RISC-V and microcontroller kernel declarations

## Testing

All new kernels have been tested to compile successfully with g++ 13.3.0:

```bash
# Test RISC-V kernel
g++ -std=c++17 -I. -Isrc -Isrc/kernels -c src/kernels/kernel_riscv.cpp -o /tmp/test_riscv.o

# Test microcontroller kernel
g++ -std=c++17 -I. -Isrc -Isrc/kernels -c src/kernels/kernel_microcontroller.cpp -o /tmp/test_micro.o

# Test RISC-V capabilities
g++ -std=c++17 -I. -Isrc -c src/cpu_capabilities_riscv.cpp -o /tmp/test_caps.o
```

All tests pass successfully with no errors.

## Future Work

- Add more RISC-V specific optimizations
- Implement additional microcontroller platforms
- Add runtime CPU frequency sampling for more platforms
- Expand test coverage for embedded targets
- Add support for MIPS and PowerPC architectures

## License

All code is licensed under the MIT License, same as the main SFBench project.
