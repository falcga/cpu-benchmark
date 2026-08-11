// CPU Benchmark - RISC-V CPU Capabilities Implementation

#include "cpu_capabilities_riscv.hpp"
#include <string>

// ============================================================================
// RISC-V Capabilities Implementation
// ============================================================================

RiscvCapabilities RiscvCapabilities::detect() {
    RiscvCapabilities caps;
    
    // Detect RISC-V architecture
#if RISCV_ARCH_AVAILABLE
    caps.is_riscv = true;
    
    // Detect bit width
    #if RISCV_64BIT
        caps.is_64bit = true;
        caps.arch_string = "RV64";
    #elif RISCV_32BIT
        caps.is_64bit = false;
        caps.arch_string = "RV32";
    #endif
    
    // Detect extensions
    caps.has_m = RISCV_HAS_M != 0;
    caps.has_a = RISCV_HAS_A != 0;
    caps.has_f = RISCV_HAS_F != 0;
    caps.has_d = RISCV_HAS_D != 0;
    caps.has_c = RISCV_HAS_C != 0;
    caps.has_v = RISCV_HAS_V != 0;
    caps.has_b = RISCV_HAS_B != 0;
    caps.has_k = RISCV_HAS_K != 0;
    
    // Derived capabilities
    caps.has_fp = caps.has_f || caps.has_d;
    caps.has_vector = caps.has_v;
    
    // Try to detect vendor from compiler macros
    #if defined(__riscv_vendor_sifive)
        caps.vendor = "SiFive";
    #elif defined(__riscv_vendor_espressif)
        caps.vendor = "Espressif";
    #elif defined(__riscv_vendor_thead)
        caps.vendor = "T-Head";
    #elif defined(__riscv_vendor_andes)
        caps.vendor = "Andes";
    #else
        caps.vendor = "Unknown";
    #endif
    
    // Build arch string with extensions
    if (caps.is_64bit) {
        caps.arch_string = "RV64";
    } else {
        caps.arch_string = "RV32";
    }
    
    if (caps.has_m) caps.arch_string += "M";
    if (caps.has_a) caps.arch_string += "A";
    if (caps.has_f) caps.arch_string += "F";
    if (caps.has_d) caps.arch_string += "D";
    if (caps.has_c) caps.arch_string += "C";
    if (caps.has_v) caps.arch_string += "V";
    if (caps.has_b) caps.arch_string += "B";
    if (caps.has_k) caps.arch_string += "K";
    
    if (caps.arch_string.empty()) {
        caps.arch_string = "RISC-V (unknown extensions)";
    }
#else
    caps.is_riscv = false;
    caps.arch_string = "Not RISC-V";
#endif
    
    return caps;
}

const RiscvCapabilities& RiscvCapabilities::get() {
    static const RiscvCapabilities instance = detect();
    return instance;
}

std::string RiscvCapabilities::to_string() const {
    std::string result = "RISC-V Capabilities:\n";
    result += "  Is RISC-V: " + std::string(is_riscv ? "Yes" : "No") + "\n";
    
    if (is_riscv) {
        result += "  Architecture: " + arch_string + "\n";
        result += "  64-bit: " + std::string(is_64bit ? "Yes" : "No") + "\n";
        result += "  Vendor: " + vendor + "\n";
        result += "  Extensions:\n";
        result += "    M (Mul/Div): " + std::string(has_m ? "Yes" : "No") + "\n";
        result += "    A (Atomic): " + std::string(has_a ? "Yes" : "No") + "\n";
        result += "    F (FP32): " + std::string(has_f ? "Yes" : "No") + "\n";
        result += "    D (FP64): " + std::string(has_d ? "Yes" : "No") + "\n";
        result += "    C (Compressed): " + std::string(has_c ? "Yes" : "No") + "\n";
        result += "    V (Vector): " + std::string(has_v ? "Yes" : "No") + "\n";
        result += "    B (Bitmanip): " + std::string(has_b ? "Yes" : "No") + "\n";
        result += "    K (Crypto): " + std::string(has_k ? "Yes" : "No") + "\n";
        result += "  Derived:\n";
        result += "    Has FP: " + std::string(has_fp ? "Yes" : "No") + "\n";
        result += "    Has Vector: " + std::string(has_vector ? "Yes" : "No") + "\n";
    }
    
    return result;
}

// ============================================================================
// Microcontroller Capabilities Implementation
// ============================================================================

MicrocontrollerCapabilities MicrocontrollerCapabilities::detect() {
    MicrocontrollerCapabilities caps;
    
    // Detect microcontroller type
#if ARCH_AVR
    caps.type = MicrocontrollerType::AVR;
    caps.name = "AVR";
    caps.description = "8-bit AVR microcontroller (ATmega, ATtiny, etc.)";
    caps.has_fpu = false;
    caps.has_simd = false;
    caps.has_multicore = false;
    caps.ram_size = 8;  // Typical AVR has 8-256 KB RAM
    caps.flash_size = 256;  // Typical flash size
    caps.max_alloc = 2;  // Max 2KB allocation
    caps.use_fixed_point = true;
    caps.use_integer_only = true;
    
#elif ARCH_ARDUINO
    #if defined(__AVR__)
        caps.type = MicrocontrollerType::Arduino;
        caps.name = "Arduino AVR";
        caps.description = "Arduino board with AVR microcontroller";
    #elif defined(__arm__)
        caps.type = MicrocontrollerType::Arduino;
        caps.name = "Arduino ARM";
        caps.description = "Arduino board with ARM microcontroller";
        caps.has_fpu = true;
    #else
        caps.type = MicrocontrollerType::Arduino;
        caps.name = "Arduino";
        caps.description = "Arduino board";
    #endif
    caps.has_simd = false;
    caps.has_multicore = false;
    caps.ram_size = 32;  // Typical Arduino has limited RAM
    caps.flash_size = 256;
    caps.max_alloc = 4;
    caps.use_fixed_point = !caps.has_fpu;
    caps.use_integer_only = !caps.has_fpu;
    
#elif ARCH_ESP
    #if defined(ESP32)
        caps.type = MicrocontrollerType::ESP32;
        caps.name = "ESP32";
        caps.description = "ESP32 dual-core microcontroller";
        caps.has_fpu = true;  // ESP32 has FPU
        caps.has_simd = true;  // ESP32 has SIMD instructions
        caps.has_multicore = true;  // Dual-core
        caps.ram_size = 512;  // ~512KB RAM
        caps.flash_size = 4096;  // Typical 4MB flash
        caps.max_alloc = 64;
        caps.use_fixed_point = false;
        caps.use_integer_only = false;
    #elif defined(ESP8266)
        caps.type = MicrocontrollerType::ESP8266;
        caps.name = "ESP8266";
        caps.description = "ESP8266 single-core microcontroller";
        caps.has_fpu = false;  // ESP8266 has no hardware FPU
        caps.has_simd = false;
        caps.has_multicore = false;
        caps.ram_size = 80;  // ~80KB RAM
        caps.flash_size = 4096;
        caps.max_alloc = 8;
        caps.use_fixed_point = true;
        caps.use_integer_only = false;  // Can do soft float
    #endif
    
#elif ARCH_ARMv6
    caps.type = MicrocontrollerType::ARMv6;
    caps.name = "ARMv6";
    caps.description = "ARMv6 architecture (Raspberry Pi Zero, etc.)";
    caps.has_fpu = false;  // ARMv6 typically has no FPU
    caps.has_simd = false;
    caps.has_multicore = false;  // Usually single-core
    caps.ram_size = 512;
    caps.flash_size = 0;
    caps.max_alloc = 64;
    caps.use_fixed_point = true;
    caps.use_integer_only = false;
    
#elif ARCH_ARMv7
    caps.type = MicrocontrollerType::ARMv7;
    caps.name = "ARMv7";
    caps.description = "ARMv7 architecture";
    caps.has_fpu = true;  // ARMv7 typically has FPU
    caps.has_simd = true;  // May have NEON
    caps.has_multicore = true;  // Usually multi-core
    caps.ram_size = 1024;
    caps.flash_size = 0;
    caps.max_alloc = 128;
    caps.use_fixed_point = false;
    caps.use_integer_only = false;
    
#elif ARCH_X86_LEGACY
    caps.type = MicrocontrollerType::LegacyX86;
    caps.name = "Legacy x86";
    caps.description = "Very old x86 (Pentium III and earlier)";
    caps.has_fpu = true;  // Even old x86 has x87 FPU
    caps.has_simd = false;  // No SIMD
    caps.has_multicore = false;
    caps.ram_size = 256;  // Typical old systems
    caps.flash_size = 0;
    caps.max_alloc = 32;
    caps.use_fixed_point = false;
    caps.use_integer_only = false;
    
#else
    // Not a microcontroller - return defaults
    caps.type = MicrocontrollerType::Unknown;
    caps.name = "Desktop/Server";
    caps.description = "Regular desktop or server system";
    caps.has_fpu = true;
    caps.has_simd = true;
    caps.has_multicore = true;
    caps.ram_size = 0;  // Unlimited
    caps.flash_size = 0;
    caps.max_alloc = 0;  // Unlimited
    caps.use_fixed_point = false;
    caps.use_integer_only = false;
#endif
    
    return caps;
}

const MicrocontrollerCapabilities& MicrocontrollerCapabilities::get() {
    static const MicrocontrollerCapabilities instance = detect();
    return instance;
}

std::string MicrocontrollerCapabilities::to_string() const {
    std::string result = "Microcontroller Capabilities:\n";
    result += "  Type: " + name + "\n";
    result += "  Description: " + description + "\n";
    result += "  Has FPU: " + std::string(has_fpu ? "Yes" : "No") + "\n";
    result += "  Has SIMD: " + std::string(has_simd ? "Yes" : "No") + "\n";
    result += "  Multi-core: " + std::string(has_multicore ? "Yes" : "No") + "\n";
    result += "  RAM: " + std::to_string(ram_size) + " KB\n";
    result += "  Flash: " + std::to_string(flash_size) + " KB\n";
    
    if (max_alloc > 0) {
        result += "  Max allocation: " + std::to_string(max_alloc) + " KB\n";
    } else {
        result += "  Max allocation: Unlimited\n";
    }
    
    result += "  Use fixed-point: " + std::string(use_fixed_point ? "Yes" : "No") + "\n";
    result += "  Use integer-only: " + std::string(use_integer_only ? "Yes" : "No") + "\n";
    
    return result;
}

// ============================================================================
// Helper functions for platform detection
// ============================================================================

// Check if running on Raspberry Pi
bool is_raspberry_pi() {
    // This is a runtime check that works on Linux
    #ifdef __linux__
        // Check for Raspberry Pi in /proc/device-tree/model
        // This is a simple heuristic - in practice you'd need to read the file
        // For now, we'll use compile-time detection
        #if defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__)
            return true;  // Likely RPi Zero or similar
        #endif
    #endif
    return false;
}

// Check if running on very old Windows
bool is_legacy_windows() {
    #ifdef _WIN32
        // Check Windows version - legacy means pre-Windows 10
        // This would need actual Windows API calls at runtime
        // For now, return false (assume modern Windows)
        return false;
    #else
        return false;
    #endif
}

// Check if we need to use legacy x86 fallback
bool need_legacy_x86_fallback() {
    // Check at runtime if AVX2/FMA are not available
    // This is a fallback for when compile-time detection fails
    #if ARCH_X86_LEGACY
        return true;
    #else
        // Could add runtime detection here
        return false;
    #endif
}
