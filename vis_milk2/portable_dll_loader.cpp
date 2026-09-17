#include "portable_dll_loader.h"

#include <delayimp.h>
#include <string>

namespace
{
#ifdef _WIN64
    constexpr wchar_t kArchitectureFolder[] = L"x64";
#else
    constexpr wchar_t kArchitectureFolder[] = L"x86";
#endif

    bool IsManagedLibrary(const char* name)
    {
        if (!name)
            return false;

        return _stricmp(name, "d3dx9_43.dll") == 0 ||
            _strnicmp(name, "avcodec-", 8) == 0 ||
            _strnicmp(name, "avformat-", 9) == 0 ||
            _strnicmp(name, "avutil-", 7) == 0 ||
            _strnicmp(name, "swscale-", 8) == 0 ||
            _strnicmp(name, "swresample-", 11) == 0;
    }

    bool IsFfmpegLibrary(const char* name)
    {
        return name && (_strnicmp(name, "avcodec-", 8) == 0 ||
            _strnicmp(name, "avformat-", 9) == 0 ||
            _strnicmp(name, "avutil-", 7) == 0 ||
            _strnicmp(name, "swscale-", 8) == 0 ||
            _strnicmp(name, "swresample-", 11) == 0);
    }

    bool BuildPackagedLibraryPath(const wchar_t* name, std::wstring& path)
    {
        wchar_t executablePath[MAX_PATH] = {};
        const DWORD length = GetModuleFileNameW(nullptr, executablePath, _countof(executablePath));
        if (length == 0 || length >= _countof(executablePath))
            return false;

        std::wstring executable(executablePath, length);
        const std::wstring::size_type slash = executable.find_last_of(L"\\/");
        if (slash == std::wstring::npos)
            return false;

        path = executable.substr(0, slash + 1);
        path += L"BeatDrop Resources\\dlls\\";
        path += kArchitectureFolder;
        path += L"\\";
        path += name;
        return true;
    }

    HMODULE LoadFromPackage(const wchar_t* name)
    {
        std::wstring path;
        if (!BuildPackagedLibraryPath(name, path))
            return nullptr;

        // Supplying a full path keeps both architectures isolated and lets each
        // FFmpeg dependency resolve next to the DLL that imports it.
        return ::LoadLibraryW(path.c_str());
    }

    HMODULE FindOrLoadFromPackage(const wchar_t* name)
    {
        if (HMODULE library = GetModuleHandleW(name))
            return library;
        return LoadFromPackage(name);
    }

    bool LoadFfmpegRuntime()
    {
        // Load every dependency from the package directory before loading the
        // FFmpeg entry point.  This avoids the normal DLL search path for the
        // dependencies of avformat/avcodec and works on Vista without newer
        // AddDllDirectory or LoadLibraryEx search flags.
        static const wchar_t* const libraries[] = {
            L"avutil-61.dll",
            L"swresample-7.dll",
            L"avcodec-63.dll",
            L"avformat-63.dll",
            L"swscale-10.dll"
        };

        for (const wchar_t* library : libraries)
        {
            if (!FindOrLoadFromPackage(library))
                return false;
        }
        return true;
    }

    FARPROC WINAPI DelayLoadHook(unsigned notification, PDelayLoadInfo delayInfo)
    {
        if (notification != dliNotePreLoadLibrary || !delayInfo || !IsManagedLibrary(delayInfo->szDll))
            return nullptr;

        if (IsFfmpegLibrary(delayInfo->szDll) && !LoadFfmpegRuntime())
            return nullptr;

        wchar_t libraryName[MAX_PATH] = {};
        if (MultiByteToWideChar(CP_ACP, 0, delayInfo->szDll, -1, libraryName, _countof(libraryName)) == 0)
            return nullptr;

        return reinterpret_cast<FARPROC>(FindOrLoadFromPackage(libraryName));
    }
}

extern "C" const PfnDliHook __pfnDliNotifyHook2 = DelayLoadHook;

namespace BeatDropPortableDll
{
    HMODULE LoadLibrary(const wchar_t* name)
    {
        if (!name || !*name)
            return nullptr;

        if (HMODULE library = LoadFromPackage(name))
            return library;

        // Keep compatibility with existing development layouts and installs that
        // still place the dependency beside the executable.
        return ::LoadLibraryW(name);
    }

    HMODULE LoadD3DX9()
    {
        return LoadLibrary(L"d3dx9_43.dll");
    }

    bool PreloadD3DX9()
    {
        // d3d9.dll is a regular Windows dependency and is already loaded by
        // the executable.  Keep the one packaged DirectX helper BeatDrop uses
        // resident before sprites, textures, or text first call into D3DX.
        if (GetModuleHandleW(L"d3dx9_43.dll"))
            return true;

        return LoadD3DX9() != nullptr;
    }

    bool IsD3DX9Available()
    {
        return PreloadD3DX9();
    }
}
