#include "portable_dll_loader.h"

#include <delayimp.h>
#include <string>
#include <winver.h>

#pragma comment(lib, "version.lib")

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

    bool IsRunningUnderWine()
    {
        // Wine exposes this private ntdll export specifically so applications
        // can select compatible workarounds without relying on an OS-version
        // check (which Wine may intentionally emulate).
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        return ntdll && GetProcAddress(ntdll, "wine_get_version") != nullptr;
    }

    bool IsWineBuiltinD3DX9(HMODULE library)
    {
        if (!IsRunningUnderWine() || !library)
            return false;

        wchar_t path[MAX_PATH] = {};
        const DWORD pathLength = GetModuleFileNameW(library, path, _countof(path));
        if (pathLength == 0 || pathLength >= _countof(path))
            return true; // Do not risk the known Wine fallback when unknown.

        DWORD ignored = 0;
        const DWORD bytes = GetFileVersionInfoSizeW(path, &ignored);
        if (bytes == 0)
            return true;

        std::wstring versionInfo(bytes, L'\0');
        if (!GetFileVersionInfoW(path, 0, bytes, &versionInfo[0]))
            return true;

        struct Translation { WORD language; WORD codePage; };
        Translation* translations = nullptr;
        UINT translationBytes = 0;
        if (!VerQueryValueW(&versionInfo[0], L"\\VarFileInfo\\Translation",
                reinterpret_cast<void**>(&translations), &translationBytes) ||
            translationBytes < sizeof(Translation))
            return true;

        wchar_t query[64] = {};
        swprintf_s(query, L"\\StringFileInfo\\%04x%04x\\CompanyName",
            translations[0].language, translations[0].codePage);
        wchar_t* company = nullptr;
        UINT companyLength = 0;
        if (!VerQueryValueW(&versionInfo[0], query,
                reinterpret_cast<void**>(&company), &companyLength) || !company)
            return true;

        return wcsstr(company, L"Wine") != nullptr;
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
        // Prefer an installed runtime. Wine can host the native Microsoft DLL
        // installed with Winetricks, but its builtin D3DX font path can fault
        // during title-font creation. Reject only that implementation, then
        // use BeatDrop's packaged native fallback.
        if (HMODULE library = ::LoadLibraryW(L"d3dx9_43.dll"))
        {
            if (!IsWineBuiltinD3DX9(library))
                return library;
            FreeLibrary(library);
        }

        if (HMODULE library = LoadFromPackage(L"d3dx9_43.dll"))
            return library;

        return nullptr;
    }

    bool PreloadD3DX9()
    {
        // d3d9.dll is a regular Windows dependency and is already loaded by
        // the executable.  Keep the one packaged DirectX helper BeatDrop uses
        // resident before sprites, textures, or text first call into D3DX.
        if (HMODULE library = GetModuleHandleW(L"d3dx9_43.dll"))
        {
            // A previously-loaded Wine builtin cannot be replaced safely in
            // process.  Reject it so initialization reports the missing
            // portable helper instead of reaching the buggy font code.
            return !IsWineBuiltinD3DX9(library);
        }

        return LoadD3DX9() != nullptr;
    }

    bool IsD3DX9Available()
    {
        return PreloadD3DX9();
    }
}
