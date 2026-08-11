#pragma once
// CPU Benchmark - RISC-V CPU Capabilities Detection
// Detects RISC-V architecture features and extensions

#include "types.hpp"
#include <string>
#include <cstdint>

// RISC-V architecture detection
#ifdef __riscv
    #ifdef __riscv_xlen
        #if __riscv_xlen == 64
            #define RISCV_64BIT 1
        #elif __riscv_xlen == 32
            #define RISCV_32BIT 1
        #endif
    #endif
    
    // Check for RISC-V extensions
    #if defined(__riscv_mul) || defined(__riscv_m)
        #define RISCV_HAS_M 1  // Multiply/divide extension
    #endif
    
    #if defined(__riscv_atomic) || defined(__riscv_a)
        #define RISCV_HAS_A 1  // Atomic instructions extension
    #endif
    
    #if defined(__riscv_fdiv) || defined(__riscv_f)
        #define RISCV_HAS_F 1  // Single-precision floating-point
    #endif
    
    #if defined(__riscv_d) || defined(__riscv_fd)
        #define RISCV_HAS_D 1  // Double-precision floating-point
    #endif
    
    #if defined(__riscv_compressed) || defined(__riscv_c)
        #define RISCV_HAS_C 1  // Compressed instructions (RVC)
    #endif
    
    #if defined(__riscv_vector) || defined(__riscv_v)
        #define RISCV_HAS_V 1  // Vector extension (RVV)
    #endif
    
    #if defined(__riscv_bitmanip) || defined(__riscv_b)
        #define RISCV_HAS_B 1  // Bit manipulation extension
    #endif
    
    #if defined(__riscv_crypto) || defined(__riscv_k)
        #define RISCV_HAS_K 1  // Cryptography extension
    #endif
    
    #define RISCV_ARCH_AVAILABLE 1
#else
    #define RISCV_ARCH_AVAILABLE 0
#endif

// Microcontroller architecture detection
#ifdef __AVR__
    #define ARCH_AVR 1
    #define ARCH_MICROCONTROLLER 1
#elif defined(ARDUINO)
    #define ARCH_ARDUINO 1
    #define ARCH_MICROCONTROLLER 1
#elif defined(ESP32) || defined(ESP8266)
    #define ARCH_ESP 1
    #define ARCH_MICROCONTROLLER 1
#elif defined(__arm__) && !defined(__aarch64__)
    #define ARCH_ARM32 1
    #if defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || \
        defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
        #define ARCH_ARMv6 1
    #elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__)
        #define ARCH_ARMv7 1
    #endif
#else
    #define ARCH_AVR 0
    #define ARCH_ARDUINO 0
    #define ARCH_ESP 0
    #define ARCH_ARM32 0
#endif

// Legacy x86 detection (for very old CPUs)
#if defined(__i386__) || defined(_M_IX86)
    #define ARCH_X86_LEGACY 1
#else
    #define ARCH_X86_LEGACY 0
#endif

// ============================================================================
// RISC-V specific capability detection
// ============================================================================

struct RiscvCapabilities {
    // RISC-V base and extensions
    bool is_riscv;
    bool is_64bit;
    bool has_m;       // Multiply/divide
    bool has_a;       // Atomic instructions
    bool has_f;       // Single-precision FP
    bool has_d;       // Double-precision FP
    bool has_c;       // Compressed instructions
    bool has_v;       // Vector extension
    bool has_b;       // Bit manipulation
    bool has_k;       // Crypto extension
    
    // Derived capabilities
    bool has_fp;      // Any floating point (F or D)
    bool has_vector;  // Vector extension available
    
    // RISC-V vendor/implementation info
    std::string vendor;
    std::string arch_string;
    
    RiscvCapabilities()
        : is_riscv(false),
          is_64bit(false),
          has_m(false),
          has_a(false),
          has_f(false),
          has_d(false),
          has_c(false),
          has_v(false),
          has_b(false),
          has_k(false),
          has_fp(false),
          has_vector(false),
          vendor("Unknown"),
          arch_string("Unknown") {}
    
    static const RiscvCapabilities& get();
    std::string to_string() const;
    
private:
    static RiscvCapabilities detect();
};

// ============================================================================
// Microcontroller capability detection
// ============================================================================

enum class MicrocontrollerType {
    Unknown,
    AVR,        // ATmega, ATtiny, etc.
    Arduino,    // Arduino boards (may be AVR or ARM)
    ESP32,      // ESP32 (Xtensa or RISC-V)
    ESP8266,    // ESP8266 (Xtensa)
    RPi,        // Raspberry Pi
    RPiZero,    // Raspberry Pi Zero (ARMv6)
    ARMv6,      // ARMv6 without NEON
    ARMv7,      // ARMv7 (may have NEON)
    ARMv8,      // ARMv8/AArch64
    LegacyX86,  // Very old x86 (Pentium III and earlier)
};

struct MicrocontrollerCapabilities {
    MicrocontrollerType type;
    bool has_fpu;           // Has hardware FPU
    bool has_simd;          // Has some form of SIMD
    bool has_multicore;     // Has multiple cores
    size_t ram_size;       // Approximate RAM size in KB
    size_t flash_size;     // Approximate flash size in KB (if applicable)
    size_t max_alloc;      // Maximum recommended allocation size
    bool use_fixed_point;   // Should use fixed-point arithmetic
    bool use_integer_only; // Should avoid floating point entirely
    
    std::string name;
    std::string description;
    
    MicrocontrollerCapabilities()
        : type(MicrocontrollerType::Unknown),
          has_fpu(false),
          has_simd(false),
          has_multicore(false),
          ram_size(0),
          flash_size(0),
          max_alloc(0),
          use_fixed_point(false),
          use_integer_only(false),
          name("Unknown"),
          description("Unknown microcontroller") {}
    
    static const MicrocontrollerCapabilities& get();
    std::string to_string() const;
    
private:
    static MicrocontrollerCapabilities detect();
};

// ============================================================================
// Universal capability flags
// ============================================================================

// Check if current platform is a microcontroller
inline bool is_microcontroller() {
#if ARCH_MICROCONTROLLER
    return true;
#else
    return false;
#endif
}

// Check if current platform is RISC-V
inline bool is_riscv() {
#if RISCV_ARCH_AVAILABLE
    return true;
#else
    return false;
#endif
}

// Check if we should use universal scalar fallback
inline bool should_use_universal_fallback() {
    // Use universal fallback on microcontrollers and very old x86
#if ARCH_MICROCONTROLLER || ARCH_X86_LEGACY
    return true;
#else
    return false;
#endif
}

// Get platform name for display
inline const char* get_platform_name() {
#if RISCV_ARCH_AVAILABLE
    return "RISC-V";
#elif ARCH_ARDUINO
    return "Arduino";
#elif ARCH_ESP
    return "ESP32/ESP8266";
#elif ARCH_AVR
    return "AVR";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "ARM64";
#elif defined(__arm__)
    return "ARM32";
#elif defined(__x86_64__) || defined(_M_X64)
    return "x86-64";
#elif defined(__i386__) || defined(_M_IX86)
    return "x86";
#else
    return "Unknown";
#endif
}
