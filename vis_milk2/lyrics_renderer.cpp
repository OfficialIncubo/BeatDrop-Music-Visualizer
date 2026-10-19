#include "lyrics_renderer.h"
#include "unicode_text.h"

#include <algorithm>

namespace
{
    constexpr float kTransitionSeconds = 0.32f;
    // Oversample glyphs before they are composited onto the MilkDrop frame.
    // This removes the visible stair-stepping produced by scaling a 1024px
    // text target up to a widescreen render surface.
    constexpr int kPreferredTextureWidth = 2048;
    constexpr int kPreferredTextureHeight = 384;

    struct LyricsVertex
    {
        float x;
        float y;
        float z;
        float rhw;
        DWORD diffuse;
        float u;
        float v;
    };

    constexpr DWORD kLyricsVertexFormat = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

    void Release(IUnknown*& object)
    {
        if (object)
        {
            object->Release();
            object = nullptr;
        }
    }
}

BeatDropLyricsRenderer::~BeatDropLyricsRenderer()
{
    Reset();
}

void BeatDropLyricsRenderer::SetFont(const wchar_t* face, bool bold, bool italic,
    int size, bool antiAliased, int red, int green, int blue)
{
    const std::wstring requestedFace = face && *face ? face : L"Times New Roman";
    const int requestedSize = (std::max)(12, (std::min)(128, size));
    const int requestedRed = (std::max)(0, (std::min)(255, red));
    const int requestedGreen = (std::max)(0, (std::min)(255, green));
    const int requestedBlue = (std::max)(0, (std::min)(255, blue));
    if (requestedFace == m_fontFace && bold == m_fontBold && italic == m_fontItalic &&
        antiAliased == m_fontAntiAliased && requestedSize == m_fontSize &&
        requestedRed == m_fontColorR && requestedGreen == m_fontColorG &&
        requestedBlue == m_fontColorB)
        return;

    m_fontFace = requestedFace;
    m_fontBold = bold;
    m_fontItalic = italic;
    m_fontAntiAliased = antiAliased;
    m_fontSize = requestedSize;
    m_fontColorR = requestedRed;
    m_fontColorG = requestedGreen;
    m_fontColorB = requestedBlue;
    // The next frame recreates the font and its private alpha textures.  Text
    // content is retained, so changing an INI setting cannot blank the line.
    OnDeviceLost();
}

void BeatDropLyricsRenderer::SetLine(const std::wstring& line, float now)
{
    if (line == m_currentLine)
        return;

    // Keep two independent cached glyph textures.  The outgoing line is never
    // redrawn into the song-title texture, which prevents title/message state
    // from leaking into lyrics during a rapid track seek.
    m_outgoingLine = m_currentLine;
    std::swap(m_currentTexture, m_outgoingTexture);
    m_outgoingDirty = !m_outgoingLine.empty();
    m_currentLine = line;
    m_currentDirty = !m_currentLine.empty();
    m_transitionStart = now;
}

void BeatDropLyricsRenderer::Render(LPDIRECT3DDEVICE9 device, int width,
    int height, float now)
{
    if (!device || width <= 0 || height <= 0 ||
        (m_currentLine.empty() && m_outgoingLine.empty()))
        return;
    if (!EnsureResources(device))
        return;

    // Isolate the overlay from preset shaders, clipping and blend state.
    IDirect3DStateBlock9* state = nullptr;
    if (FAILED(device->CreateStateBlock(D3DSBT_ALL, &state)) || !state)
        return;
    if (FAILED(state->Capture()))
    {
        state->Release();
        return;
    }

    if (m_outgoingDirty)
    {
        if (!RenderTextToTexture(device, m_outgoingTexture, m_outgoingLine))
        {
            state->Apply();
            state->Release();
            return;
        }
        m_outgoingDirty = false;
    }
    if (m_currentDirty)
    {
        if (!RenderTextToTexture(device, m_currentTexture, m_currentLine))
        {
            state->Apply();
            state->Release();
            return;
        }
        m_currentDirty = false;
    }

    if (now < m_transitionStart)
        m_transitionStart = now; // MilkDrop's clock may reset with a preset.
    const float progress = m_transitionStart < 0.0f ? 1.0f :
        Clamp01((now - m_transitionStart) / kTransitionSeconds);

    const bool hasOutgoingLine = !m_outgoingLine.empty();
    const float outgoingProgress = hasOutgoingLine ? Clamp01(progress * 2.0f) : 1.0f;
    const float incomingProgress = hasOutgoingLine ?
        Clamp01((progress - 0.5f) * 2.0f) : progress;
    if (hasOutgoingLine && outgoingProgress < 1.0f)
    {
        // First, the prior line fades out while shrinking about its own centre.
        DrawTexture(device, m_outgoingTexture, width, height,
            1.0f - outgoingProgress, 1.0f - 0.10f * outgoingProgress);
    }
    if (!m_currentLine.empty())
    {
        // Then the next line fades and zooms in from the same centre point.
        DrawTexture(device, m_currentTexture, width, height,
            incomingProgress, 0.90f + 0.10f * incomingProgress);
    }

    state->Apply();
    state->Release();

    if (progress >= 1.0f)
        m_outgoingLine.clear();
}

void BeatDropLyricsRenderer::OnDeviceLost()
{
    IUnknown* object = m_currentTexture;
    Release(object);
    m_currentTexture = nullptr;
    object = m_outgoingTexture;
    Release(object);
    m_outgoingTexture = nullptr;
    object = m_font;
    Release(object);
    m_font = nullptr;
    m_textureWidth = 0;
    m_textureHeight = 0;
    m_currentDirty = !m_currentLine.empty();
    m_outgoingDirty = !m_outgoingLine.empty();
}

void BeatDropLyricsRenderer::Reset()
{
    OnDeviceLost();
    m_currentLine.clear();
    m_outgoingLine.clear();
    m_currentDirty = false;
    m_outgoingDirty = false;
    m_transitionStart = -1.0f;
}

bool BeatDropLyricsRenderer::EnsureResources(LPDIRECT3DDEVICE9 device)
{
    if (m_currentTexture && m_outgoingTexture && m_font)
        return true;

    OnDeviceLost();
    // Upload straight-alpha glyph pixels directly. No font render target or
    // dependency on D3DX's glyph-atlas alpha/shaping behavior is involved.
    const D3DFORMAT format = D3DFMT_A8R8G8B8;

    int textureWidth = kPreferredTextureWidth;
    int textureHeight = kPreferredTextureHeight;
    HRESULT result = E_FAIL;
    while (textureWidth >= 256 && textureHeight >= 48)
    {
        result = D3DXCreateTexture(device, textureWidth, textureHeight, 1,
            0, format, D3DPOOL_DEFAULT, &m_currentTexture);
        if (SUCCEEDED(result))
        {
            result = D3DXCreateTexture(device, textureWidth, textureHeight, 1,
                0, format, D3DPOOL_DEFAULT, &m_outgoingTexture);
        }
        if (SUCCEEDED(result))
            break;
        OnDeviceLost();
        textureWidth /= 2;
        textureHeight /= 2;
    }
    if (FAILED(result) || !m_currentTexture || !m_outgoingTexture)
    {
        OnDeviceLost();
        return false;
    }

    D3DSURFACE_DESC actual = {};
    if (FAILED(m_currentTexture->GetLevelDesc(0, &actual)))
    {
        OnDeviceLost();
        return false;
    }
    textureWidth = actual.Width;
    textureHeight = actual.Height;

    // Separate from the title renderer; the default is Times New Roman Italic
    // and the values can be changed through the sixth-font INI settings.
    const int scaledFontSize = m_fontSize * textureHeight / 192;
    const int height = (std::max)(12, (std::min)(textureHeight - 8, scaledFontSize));
    if (FAILED(D3DXCreateFontW(device, height, 0, m_fontBold ? FW_BOLD : FW_NORMAL,
        1, m_fontItalic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        m_fontAntiAliased ? CLEARTYPE_NATURAL_QUALITY : NONANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, m_fontFace.c_str(), &m_font)) || !m_font)
    {
        OnDeviceLost();
        return false;
    }

    m_textureWidth = textureWidth;
    m_textureHeight = textureHeight;
    m_currentDirty = !m_currentLine.empty();
    m_outgoingDirty = !m_outgoingLine.empty();
    return true;
}

bool BeatDropLyricsRenderer::RenderTextToTexture(LPDIRECT3DDEVICE9 device,
    IDirect3DTexture9* texture, const std::wstring& text)
{
    if (!device || !texture || !m_font || text.empty())
        return false;

    RECT bounds = {32, 0, m_textureWidth - 32, m_textureHeight};
    RECT measured = bounds;
    if (!BeatDropText::Measure(m_font, text.c_str(), -1, &measured, DT_CENTER | DT_WORDBREAK))
        return false;
    const int textHeight = (std::max)(1L, measured.bottom - measured.top);
    bounds.top = (std::max)(4, (m_textureHeight - textHeight) / 2);
    bounds.bottom = m_textureHeight - 4;
    return BeatDropText::Upload(device, texture, m_font, text.c_str(), -1,
        bounds, DT_CENTER | DT_WORDBREAK,
        D3DCOLOR_ARGB(255, m_fontColorR, m_fontColorG, m_fontColorB)) != 0;
}

void BeatDropLyricsRenderer::DrawTexture(LPDIRECT3DDEVICE9 device,
    IDirect3DTexture9* texture, int width, int height, float alpha, float scale) const
{
    if (!texture || alpha <= 0.0f || scale <= 0.0f)
        return;

    const float baseWidth = (std::min)(width * 0.86f, height * 4.8f);
    const float baseHeight = baseWidth * m_textureHeight /
        static_cast<float>(m_textureWidth);
    const float centreX = width * 0.5f;
    // Keep subtitles centred horizontally but anchored in the lower safe area.
    // The texture is still scaled about this point, so fade/zoom transitions
    // stay visually centred instead of drifting upward.
    const float centreY = height * 0.82f;
    const float halfWidth = baseWidth * scale * 0.5f;
    const float halfHeight = baseHeight * scale * 0.5f;
    const DWORD color = D3DCOLOR_ARGB(static_cast<int>(255.0f * Clamp01(alpha)),
        255, 255, 255);

    const LyricsVertex vertices[4] = {
        {centreX - halfWidth - 0.5f, centreY - halfHeight - 0.5f, 0.0f, 1.0f, color, 0.0f, 0.0f},
        {centreX + halfWidth - 0.5f, centreY - halfHeight - 0.5f, 0.0f, 1.0f, color, 1.0f, 0.0f},
        {centreX - halfWidth - 0.5f, centreY + halfHeight - 0.5f, 0.0f, 1.0f, color, 0.0f, 1.0f},
        {centreX + halfWidth - 0.5f, centreY + halfHeight - 0.5f, 0.0f, 1.0f, color, 1.0f, 1.0f}
    };

    BeatDropText::SetOverlayState(device);
    device->SetFVF(kLyricsVertexFormat);
    device->SetTexture(0, texture);
    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(LyricsVertex));
}

float BeatDropLyricsRenderer::Clamp01(float value)
{
    return (std::max)(0.0f, (std::min)(1.0f, value));
}
