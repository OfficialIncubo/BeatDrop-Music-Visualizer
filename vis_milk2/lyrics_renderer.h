#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#include <string>

// A self-contained, post-composite lyrics overlay.  It deliberately owns its
// texture and font so lyrics never overwrite or inherit the song-title/custom
// message renderer's state.
class BeatDropLyricsRenderer
{
public:
    BeatDropLyricsRenderer() = default;
    ~BeatDropLyricsRenderer();

    BeatDropLyricsRenderer(const BeatDropLyricsRenderer&) = delete;
    BeatDropLyricsRenderer& operator=(const BeatDropLyricsRenderer&) = delete;

    void SetLine(const std::wstring& line, float now);
    void Clear(float now) { SetLine(std::wstring(), now); }
    // The lyrics overlay has its own font settings so it never inherits the
    // song-title renderer's texture or typography.
    void SetFont(const wchar_t* face, bool bold, bool italic, int size,
        bool antiAliased, int red, int green, int blue);
    void Render(LPDIRECT3DDEVICE9 device, int width, int height, float now);

    // Called while the D3D device is being torn down.  Text is retained and
    // rendered again lazily after a resize, fullscreen switch, or device reset.
    void OnDeviceLost();
    void Reset();

private:
    bool EnsureResources(LPDIRECT3DDEVICE9 device);
    bool RenderTextToTexture(LPDIRECT3DDEVICE9 device, IDirect3DTexture9* texture,
        const std::wstring& text);
    void DrawTexture(LPDIRECT3DDEVICE9 device, IDirect3DTexture9* texture,
        int width, int height, float alpha, float scale) const;
    static float Clamp01(float value);

    IDirect3DTexture9* m_currentTexture = nullptr;
    IDirect3DTexture9* m_outgoingTexture = nullptr;
    ID3DXFont* m_font = nullptr;
    int m_textureWidth = 0;
    int m_textureHeight = 0;
    std::wstring m_currentLine;
    std::wstring m_outgoingLine;
    bool m_currentDirty = false;
    bool m_outgoingDirty = false;
    float m_transitionStart = -1.0f;
    std::wstring m_fontFace = L"Times New Roman";
    bool m_fontBold = false;
    bool m_fontItalic = true;
    bool m_fontAntiAliased = true;
    int m_fontSize = 48;
    int m_fontColorR = 180;
    int m_fontColorG = 0;
    int m_fontColorB = 255;
};
