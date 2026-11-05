#pragma once
#include <d3d9.h>
#include <memory>
#include <string>
struct HdrSettings {
    float paperWhite = 0; // nits; 0 follows Windows
    float peak = 0;       // nits; 0 follows the display
    float highlights = 1;
    float compression = 1;
    float titleBrightness = 1.5f; // linear RGB multiplier
    float lyricsBrightness = 1.5f;
};
// Optional DXGI presenter. No modern graphics DLL is imported at process startup.
class HdrPresenter {
public:
    HdrPresenter();
    ~HdrPresenter();
    void Reset();
    bool Prepare(IDirect3DDevice9* source, HWND window, std::wstring& reason);
    bool BeginFrame(IDirect3DDevice9* source);
    bool EndFrame(IDirect3DDevice9* source);
    bool Present(IDirect3DDevice9* source, std::wstring& reason);
    bool Active() const;
    bool BeginLegacyComposite(IDirect3DDevice9* source);
    bool EndLegacyComposite(IDirect3DDevice9* source, bool expandHighlights = true);
    bool ExpandPresetHighlights(IDirect3DDevice9* source); // inside scene, before overlays
    bool CaptureSdrExport(IDirect3DDevice9* source); // inside the scene, after overlays
    bool GetSdrExport(IDirect3DSurface9** surface) const; // caller releases
    HdrSettings settings;
    // Display text only; refuses feedback/title burn-in render targets.
    bool BindTextShader(IDirect3DDevice9* source, bool brighten = true,
        bool screenSpaceVertices = true);
private:
    struct Impl;
    std::unique_ptr<Impl> p;
};

// Draw-based format conversion; valid between scene boundaries.
bool CopyHdrTexture9(IDirect3DDevice9* device, IDirect3DTexture9* texture,
    IDirect3DSurface9* destination, IDirect3DPixelShader9* copyShader = nullptr,
    bool sceneOpen = false);