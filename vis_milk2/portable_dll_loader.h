#pragma once

#include <windows.h>

// Runtime libraries are shipped beside the resources instead of the executable.
// The implementation always chooses the folder matching this binary's architecture.
namespace BeatDropPortableDll
{
    HMODULE LoadLibrary(const wchar_t* name);
    HMODULE LoadD3DX9();
    bool PreloadD3DX9();
    bool IsD3DX9Available();
}
