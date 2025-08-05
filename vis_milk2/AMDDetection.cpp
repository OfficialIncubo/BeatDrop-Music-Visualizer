/*
    AMD/ATI Video/Graphic Card Detection Code
    For use in fixing Pixel Shader 4 Presets with AMD/ATI Video/Graphic Card.

    Implemented by DeepSeek: https://www.deepseek.com/en
    © 2025 Incubo_ & BeatDrop development team
*/

#include "AMDDetection.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

bool is_amd_ati() {
    // Only x86 and x64 architectures support CPUID
#if defined(__x86_64__) || defined(_M_AMD64) || defined(__i386__) || defined(_M_IX86)
    unsigned int regs[4]; // eax, ebx, ecx, edx

#if defined(_MSC_VER)
    __cpuid(reinterpret_cast<int*>(regs), 0);
#elif defined(__GNUC__) || defined(__clang__)
    asm volatile(
        "cpuid"
        : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
        : "a"(0)
        );
#else
    return false; // Unsupported compiler
#endif

    // Check for "AuthenticAMD" in little-endian integers
    if (regs[1] == 0x68747541 && // "Auth" (little-endian)
        regs[3] == 0x69746E65 && // "enti" (little-endian)
        regs[2] == 0x444D4163)   // "cAMD" (little-endian)
    {
        return true;
    }
    return false;
#else
    return false; // Non-x86 architecture
#endif
}