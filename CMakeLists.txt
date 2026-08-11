cmake_minimum_required(VERSION 3.14)
project(cpu_benchmark VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add cmake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ============================================================================
# Build Options
# ============================================================================

option(ENABLE_OPENMP "Enable OpenMP support" OFF)
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_NATIVE_ARCH "Enable native CPU optimizations (-march=native)" ON)
option(BUILD_PORTABLE "Build portable binary with runtime SIMD dispatch" OFF)
option(USE_RELEASE_BUILD_CONFIG "Include release build configuration module" OFF)

# New options for universal compatibility
option(ENABLE_RISCV "Enable RISC-V support" ON)
option(ENABLE_MICROCONTROLLER "Enable microcontroller (Arduino/RPi) support" ON)
option(ENABLE_LEGACY_X86 "Enable legacy x86 (pre-AVX2) support" ON)
option(ENABLE_UNIVERSAL_FALLBACK "Enable universal scalar fallback for all platforms" ON)
option(FORCE_SCALAR_ONLY "Force scalar-only kernels (no SIMD)" OFF)

# Include release build configuration if requested
if(USE_RELEASE_BUILD_CONFIG)
    include(release_build OPTIONAL)
endif()

# ============================================================================
# Static linking configuration for portable builds
# ============================================================================

if(BUILD_PORTABLE)
    message(STATUS "Building portable binary with static linking")
    if(MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        message(STATUS "MSVC: Using static runtime (MultiThreaded)")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_link_options(-static-libgcc -static-libstdc++)
        message(STATUS "GCC/Clang: Using static libgcc and libstdc++")
    endif()
endif()

# ============================================================================
# Architecture Detection
# ============================================================================

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64|i686|x86|i386")
    set(ARCH_X86 1)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
        set(ARCH_X86_64 1)
    else()
        set(ARCH_X86_32 1)
    endif()
elif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(ARCH_ARM64 1)
elif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM")
    set(ARCH_ARM32 1)
elif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv|RISCV")
    set(ARCH_RISCV 1)
endif()

message(STATUS "Detected architecture: ${CMAKE_SYSTEM_PROCESSOR}")

# ============================================================================
# SIMD optimization configuration
# ============================================================================

if(BUILD_PORTABLE)
    message(STATUS "Building portable binary with runtime SIMD dispatch")
    if(MSVC)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        if(ARCH_X86_64)
            add_compile_options(-msse2)
        elseif(ARCH_ARM64)
            add_compile_options(-march=armv8.2-a+fp16+simd)
        elseif(ARCH_RISCV)
            add_compile_options(-march=rv64gc -mabi=lp64)
        endif()
    endif()
else()
    if(MSVC)
        if(NOT FORCE_SCALAR_ONLY AND NOT ARCH_X86_32)
            add_compile_options(/arch:AVX2)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        if(ARCH_ARM64)
            add_compile_options(-march=armv8.2-a+fp16+simd)
        elseif(ARCH_RISCV)
            if(ENABLE_NATIVE_ARCH)
                add_compile_options(-march=native)
            else()
                add_compile_options(-march=rv64gcv -mabi=lp64)
            endif()
        elseif(ENABLE_NATIVE_ARCH)
            include(CheckCXXCompilerFlag)
            check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
            if(COMPILER_SUPPORTS_MARCH_NATIVE)
                add_compile_options(-march=native)
            endif()
        else()
            include(CheckCXXCompilerFlag)
            check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
            if(COMPILER_SUPPORTS_AVX2)
                add_compile_options(-mavx2 -mfma)
            endif()
        endif()
    endif()
endif()

# ============================================================================
# Source files
# ============================================================================

set(BENCHMARK_SOURCES
    src/main.cpp
    src/platform.cpp
    src/threading.cpp
    src/benchmark_core.cpp
    src/memory_validator.cpp
    src/runtime_dispatcher.cpp
    src/progress_reporter.cpp
    src/auto_size.cpp
    src/warmup.cpp
    src/score_calculator.cpp
    src/thread_affinity.cpp
    src/persistent_thread_pool.cpp
    src/server_performance.cpp
    src/result_submission.cpp
)

if(ENABLE_RISCV OR ENABLE_MICROCONTROLLER)
    list(APPEND BENCHMARK_SOURCES src/cpu_capabilities_riscv.cpp)
endif()

# ============================================================================
# Kernel source files
# ============================================================================

set(KERNEL_SOURCES
    src/kernels/kernel_scalar.cpp
    src/kernels/kernel_sse2.cpp
    src/kernels/kernel_avx.cpp
    src/kernels/kernel_avx2.cpp
    src/kernels/kernel_avx512.cpp
    src/kernels/kernel_neon.cpp
    src/kernels/kernel_compute.cpp
    src/kernels/kernel_compute_avx512.cpp
)

if(ENABLE_RISCV)
    list(APPEND KERNEL_SOURCES src/kernels/kernel_riscv.cpp)
endif()

if(ENABLE_MICROCONTROLLER)
    list(APPEND KERNEL_SOURCES src/kernels/kernel_microcontroller.cpp)
endif()

add_executable(cpu_benchmark ${BENCHMARK_SOURCES} ${KERNEL_SOURCES})
target_include_directories(cpu_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/kernels)

if(FORCE_SCALAR_ONLY)
    target_compile_definitions(cpu_benchmark PRIVATE FORCE_SCALAR_ONLY)
endif()

if(ENABLE_RISCV)
    target_compile_definitions(cpu_benchmark PRIVATE ENABLE_RISCV_SUPPORT)
endif()

if(ENABLE_MICROCONTROLLER)
    target_compile_definitions(cpu_benchmark PRIVATE ENABLE_MICROCONTROLLER_SUPPORT)
endif()

if(ENABLE_UNIVERSAL_FALLBACK)
    target_compile_definitions(cpu_benchmark PRIVATE ENABLE_UNIVERSAL_FALLBACK)
endif()

# ============================================================================
# Per-file compile flags for SIMD kernels
# ============================================================================

if(ARCH_X86_64 OR ARCH_X86_32)
    if(MSVC)
        set_source_files_properties(src/kernels/kernel_scalar.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_sse2.cpp PROPERTIES COMPILE_FLAGS "/arch:SSE2")
        set_source_files_properties(src/kernels/kernel_avx.cpp PROPERTIES COMPILE_FLAGS "/arch:AVX")
        set_source_files_properties(src/kernels/kernel_avx2.cpp PROPERTIES COMPILE_FLAGS "/arch:AVX2")
        set_source_files_properties(src/kernels/kernel_avx512.cpp PROPERTIES COMPILE_FLAGS "/arch:AVX512")
        set_source_files_properties(src/kernels/kernel_compute.cpp PROPERTIES COMPILE_FLAGS "/arch:AVX2")
        set_source_files_properties(src/kernels/kernel_compute_avx512.cpp PROPERTIES COMPILE_FLAGS "/arch:AVX512")
        set_source_files_properties(src/kernels/kernel_neon.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_riscv.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_microcontroller.cpp PROPERTIES COMPILE_FLAGS "")
    else()
        set_source_files_properties(src/kernels/kernel_scalar.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_sse2.cpp PROPERTIES COMPILE_FLAGS "-msse2")
        set_source_files_properties(src/kernels/kernel_avx.cpp PROPERTIES COMPILE_FLAGS "-mavx")
        set_source_files_properties(src/kernels/kernel_avx2.cpp PROPERTIES COMPILE_FLAGS "-mavx2 -mfma")
        set_source_files_properties(src/kernels/kernel_avx512.cpp PROPERTIES COMPILE_FLAGS "-mavx512f -mavx512dq")
        set_source_files_properties(src/kernels/kernel_compute.cpp PROPERTIES COMPILE_FLAGS "-mavx2 -mfma")
        set_source_files_properties(src/kernels/kernel_compute_avx512.cpp PROPERTIES COMPILE_FLAGS "-mavx512f -mavx512dq -mfma")
        set_source_files_properties(src/kernels/kernel_neon.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_riscv.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_microcontroller.cpp PROPERTIES COMPILE_FLAGS "")
    endif()

elseif(ARCH_ARM64 OR ARCH_ARM32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties(src/kernels/kernel_scalar.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_neon.cpp PROPERTIES COMPILE_FLAGS "-march=armv8.2-a+fp16+simd -O3 -ffast-math")
        set_source_files_properties(src/kernels/kernel_compute.cpp PROPERTIES COMPILE_FLAGS "-march=armv8.2-a+fp16+simd -O3 -ffast-math -funroll-loops")
        set_source_files_properties(src/kernels/kernel_compute_avx512.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_sse2.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_avx.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_avx2.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_avx512.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_riscv.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_microcontroller.cpp PROPERTIES COMPILE_FLAGS "")
    endif()

elseif(ARCH_RISCV)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties(src/kernels/kernel_scalar.cpp PROPERTIES COMPILE_FLAGS "")
        if(ENABLE_RISCV)
            set_source_files_properties(src/kernels/kernel_riscv.cpp PROPERTIES COMPILE_FLAGS "-march=rv64gcv -mabi=lp64 -O3")
        else()
            set_source_files_properties(src/kernels/kernel_riscv.cpp PROPERTIES COMPILE_FLAGS "-march=rv64gc -mabi=lp64")
        endif()
        set_source_files_properties(src/kernels/kernel_sse2.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_avx.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_avx2.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_avx512.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_neon.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_compute.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_compute_avx512.cpp PROPERTIES COMPILE_FLAGS "")
        set_source_files_properties(src/kernels/kernel_microcontroller.cpp PROPERTIES COMPILE_FLAGS "")
    endif()

else()
    message(WARNING "Unknown architecture. Using scalar-only kernels.")
    set_source_files_properties(src/kernels/kernel_scalar.cpp PROPERTIES COMPILE_FLAGS "")
    set_source_files_properties(src/kernels/kernel_sse2.cpp PROPERTIES COMPILE_FLAGS "")
    set_source_files_properties(src/kernels/kernel_avx.cpp PROPERTIES COMPILE_FLAGS "")
    set_source_files_properties(src/kernels/kernel_avx2.cpp PROPERTIES COMPILE_FLAGS "")
    set_source_files_properties(src/kernels/kernel_avx512.cpp PROPERTIES COMPILE_FLAGS "")
    set_source_files_properties(src/kernels/kernel_neon.cpp PROPERTIES COMPILE_FLAGS "")
    set_source_files_properties(src/kernels/kernel_riscv.cpp PROPERTIES COMPILE_FLAGS "")
    set_source_files_properties(src/kernels/kernel_microcontroller.cpp PROPERTIES COMPILE_FLAGS "")
    set_source_files_properties(src/kernels/kernel_compute.cpp PROPERTIES COMPILE_FLAGS "")
    set_source_files_properties(src/kernels/kernel_compute_avx512.cpp PROPERTIES COMPILE_FLAGS "")
endif()

# ============================================================================
# OpenMP support
# ============================================================================

if(ENABLE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(cpu_benchmark PUBLIC OpenMP::OpenMP_CXX)
        target_compile_definitions(cpu_benchmark PRIVATE USE_OPENMP)
    endif()
endif()

# ============================================================================
# Platform-specific threading and networking
# ============================================================================

if(WIN32)
    target_link_libraries(cpu_benchmark PRIVATE ws2_32 advapi32 pdh powrprof)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(cpu_benchmark PRIVATE Threads::Threads)
endif()

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "SFBench - Build Configuration")
message(STATUS "========================================")
message(STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  ENABLE_RISCV: ${ENABLE_RISCV}")
message(STATUS "  ENABLE_MICROCONTROLLER: ${ENABLE_MICROCONTROLLER}")
message(STATUS "  ENABLE_UNIVERSAL_FALLBACK: ${ENABLE_UNIVERSAL_FALLBACK}")
message(STATUS "  FORCE_SCALAR_ONLY: ${FORCE_SCALAR_ONLY}")
message(STATUS "  BUILD_PORTABLE: ${BUILD_PORTABLE}")
message(STATUS "  ENABLE_OPENMP: ${ENABLE_OPENMP}")
message(STATUS "  BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "========================================")
