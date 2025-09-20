/*
    AMD/ATI Processor/Graphic Card Detection Code
    For use in fixing Pixel Shader 4 Presets with AMD/ATI Processor/Graphic Card.

    Implemented by DeepSeek: https://www.deepseek.com/en
    © 2025 Incubo_ & BeatDrop development team
*/

#include "AMDDetection.h"

//#include <intrin.h> moved to header.

bool is_amd_cpu() {
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

bool is_amd_gpu() {
#if defined(_MSC_VER) && defined(_WIN32)
    // Initialize COM
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        return false;
    }

    // Create DXGI factory
    IDXGIFactory* pFactory = nullptr;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    // Enumerate adapters
    IDXGIAdapter* pAdapter = nullptr;
    UINT i = 0;
    bool amdFound = false;

    while (pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC desc;
        if (SUCCEEDED(pAdapter->GetDesc(&desc))) {
            // AMD vendor ID is 0x1002
            if (desc.VendorId == 0x1002) {
                amdFound = true;
                pAdapter->Release();
                break;
            }
        }
        pAdapter->Release();
        i++;
    }

    pFactory->Release();
    CoUninitialize();
    return amdFound;
#else
    // For non-Windows platforms, we could use other APIs
    // but for simplicity, we'll just return false
    return false;
#endif
}

bool is_amd_ati() {
    return is_amd_cpu() || is_amd_gpu();
}