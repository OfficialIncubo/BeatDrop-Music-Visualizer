#pragma once

#include <d3dx9.h>
#include <algorithm>
#include <climits>
#include <cwchar>

// D3DX9's glyph-atlas DrawText path is unsafe for some shaped scripts. Let
// Windows shape the entire Unicode string, then upload an explicit alpha mask.
// No managed resources (D3D9Ex), per-character drawing, or DLL lifetime changes.
namespace BeatDropText
{
    template<class T> struct ComPtr
    {
        T* p = nullptr;
        ~ComPtr() { if (p) p->Release(); }
        ComPtr() = default;
        ComPtr(const ComPtr&) = delete;
        ComPtr& operator=(const ComPtr&) = delete;
    };

    class Canvas
    {
    public:
        HDC dc = nullptr;
        DWORD* pixels = nullptr;
        Canvas(ID3DXFont* source, int width = 0, int height = 0)
        {
            if (!source) return;
            LOGFONTW lf = {};
            HDC sourceDC = source->GetDC();
            if (!sourceDC || !GetObjectW(GetCurrentObject(sourceDC, OBJ_FONT), sizeof(lf), &lf))
                return;
            // ClearType is for opaque LCD backgrounds, not alpha textures.
            if (lf.lfQuality != NONANTIALIASED_QUALITY) lf.lfQuality = ANTIALIASED_QUALITY;
            font = CreateFontIndirectW(&lf);
            dc = CreateCompatibleDC(nullptr);
            if (!dc || !font) return;
            oldFont = SelectObject(dc, font);
            SetTextColor(dc, RGB(255, 255, 255));
            SetBkColor(dc, RGB(0, 0, 0));
            SetBkMode(dc, TRANSPARENT);
            if (width > 0 && height > 0)
            {
                BITMAPINFO info = {};
                info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                info.bmiHeader.biWidth = width;
                info.bmiHeader.biHeight = -height;
                info.bmiHeader.biPlanes = 1;
                info.bmiHeader.biBitCount = 32;
                info.bmiHeader.biCompression = BI_RGB;
                bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS,
                    reinterpret_cast<void**>(&pixels), nullptr, 0);
                if (bitmap)
                {
                    oldBitmap = SelectObject(dc, bitmap);
                    ZeroMemory(pixels, static_cast<size_t>(width) * height * sizeof(DWORD));
                }
            }
        }
        ~Canvas()
        {
            if (oldBitmap) SelectObject(dc, oldBitmap);
            if (oldFont) SelectObject(dc, oldFont);
            if (bitmap) DeleteObject(bitmap);
            if (font) DeleteObject(font);
            if (dc) DeleteDC(dc);
        }
        bool Valid() const { return dc && font && oldFont && oldFont != HGDI_ERROR; }
        Canvas(const Canvas&) = delete;
        Canvas& operator=(const Canvas&) = delete;
    private:
        HFONT font = nullptr;
        HBITMAP bitmap = nullptr;
        HGDIOBJ oldFont = nullptr, oldBitmap = nullptr;
    };

    inline int Measure(ID3DXFont* font, const wchar_t* text, int count, RECT* rect, DWORD flags)
    {
        Canvas canvas(font);
        if (!canvas.Valid() || !text || !rect) return 0;
        return ::DrawTextW(canvas.dc, text, count, rect,
            (flags & ~DT_MODIFYSTRING) | DT_CALCRECT | DT_NOPREFIX);
    }

    // Destination must be a DEFAULT-pool A8R8G8B8 texture, not a render target.
    // GDI leaves the DIB alpha byte undefined; derive it from grayscale coverage.
    inline int Upload(IDirect3DDevice9* device, IDirect3DTexture9* texture,
        ID3DXFont* font, const wchar_t* text, int count, RECT rect, DWORD flags, DWORD color)
    {
        D3DSURFACE_DESC desc = {};
        if (!device || !texture || FAILED(texture->GetLevelDesc(0, &desc)) ||
            desc.Format != D3DFMT_A8R8G8B8 || desc.Width > 8192 || desc.Height > 8192)
            return 0;
        Canvas canvas(font, desc.Width, desc.Height);
        if (!canvas.Valid() || !canvas.pixels) return 0;
        int height = ::DrawTextW(canvas.dc, text, count, &rect,
            (flags & ~(DT_CALCRECT | DT_MODIFYSTRING)) | DT_NOPREFIX);
        if (!height || !GdiFlush()) return 0;
        ComPtr<IDirect3DTexture9> staging;
        if (FAILED(device->CreateTexture(desc.Width, desc.Height, 1, 0,
            D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &staging.p, nullptr))) return 0;
        D3DLOCKED_RECT locked = {};
        if (FAILED(staging.p->LockRect(0, &locked, nullptr, 0))) return 0;
        for (UINT y = 0; y < desc.Height; ++y)
        {
            DWORD* row = reinterpret_cast<DWORD*>(static_cast<BYTE*>(locked.pBits) + y * locked.Pitch);
            for (UINT x = 0; x < desc.Width; ++x)
            {
                DWORD mask = canvas.pixels[y * desc.Width + x];
                DWORD coverage = (std::max)(mask & 255, (std::max)((mask >> 8) & 255, (mask >> 16) & 255));
                DWORD alpha = (coverage * (color >> 24) + 127) / 255;
                row[x] = (alpha << 24) | (color & 0x00ffffff);
            }
        }
        if (FAILED(staging.p->UnlockRect(0)) || FAILED(device->UpdateTexture(staging.p, texture)))
            return 0;
        return height;
    }

    inline void SetOverlayState(IDirect3DDevice9* device)
    {
        device->SetVertexShader(nullptr);
        device->SetPixelShader(nullptr);
        device->SetRenderState(D3DRS_ZENABLE, FALSE);
        device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        device->SetRenderState(D3DRS_FOGENABLE, FALSE);
        device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
        device->SetRenderState(D3DRS_COLORWRITEENABLE, 15);
        device->SetRenderState(D3DRS_WRAP0, 0);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
        device->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
        device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
        device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        device->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, FALSE);
        device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }

    // Matches the null-sprite D3DX DrawText calls used by song information,
    // title animations, messages and playlists.
    inline int Draw(ID3DXFont* font, const wchar_t* text, int count, RECT* rect,
        DWORD flags, DWORD color, bool fastAscii = false)
    {
        if (!font || !text || !rect || count == 0) return 0;
        if (count < 0)
        {
            size_t length = wcslen(text);
            if (length > INT_MAX) return 0;
            count = static_cast<int>(length);
        }
        if (fastAscii)
        {
            bool asciiOnly = true;
            for (int i = 0; i < count; ++i)
                asciiOnly = asciiOnly && text[i] <= 127;
            // The legacy atlas is safe for ASCII and avoids per-line GDI
            // uploads in a high-volume, constantly-changing diagnostics view.
            if (asciiOnly)
                return font->DrawTextW(nullptr, text, count, rect, flags, color);
        }
        // Use one Windows GDI shaping/rasterization path for every string.
        // D3DX9's separate ASCII glyph atlas has visibly clipped/haloed edges
        // on some drivers, whereas the GDI path preserves the same clean
        // grayscale coverage for Latin and complex scripts alike.
        if (flags & DT_CALCRECT) return Measure(font, text, count, rect, flags);
        ComPtr<IDirect3DDevice9> device;
        if (FAILED(font->GetDevice(&device.p))) return 0;
        D3DVIEWPORT9 viewport = {};
        if (FAILED(device.p->GetViewport(&viewport))) return 0;
        // Rasterize only the visible portion, while retaining the caller's
        // layout rectangle (including negative origins and RTL alignment).
        RECT area = *rect;
        if (flags & DT_NOCLIP)
            area = { static_cast<LONG>(viewport.X), static_cast<LONG>(viewport.Y),
                static_cast<LONG>(viewport.X + viewport.Width), static_cast<LONG>(viewport.Y + viewport.Height) };
        area.left = (std::max)(area.left, static_cast<LONG>(viewport.X));
        area.top = (std::max)(area.top, static_cast<LONG>(viewport.Y));
        area.right = (std::min)(area.right, static_cast<LONG>(viewport.X + viewport.Width));
        area.bottom = (std::min)(area.bottom, static_cast<LONG>(viewport.Y + viewport.Height));
        if (area.right <= area.left || area.bottom <= area.top) return 0;
        ComPtr<IDirect3DTexture9> texture;
        if (FAILED(D3DXCreateTexture(device.p, area.right - area.left, area.bottom - area.top,
            1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture.p))) return 0;
        RECT local = *rect;
        OffsetRect(&local, -area.left, -area.top);
        int height = Upload(device.p, texture.p, font, text, count, local, flags, color);
        if (!height) return 0;
        D3DSURFACE_DESC desc = {};
        texture.p->GetLevelDesc(0, &desc);
        ComPtr<IDirect3DStateBlock9> state;
        if (FAILED(device.p->CreateStateBlock(D3DSBT_ALL, &state.p))) return 0;
        if (FAILED(state.p->Capture())) return 0;
        SetOverlayState(device.p);
        struct Vertex { float x, y, z, rhw; DWORD color; float u, v; };
        float left = area.left - 0.5f, top = area.top - 0.5f;
        float right = area.right - 0.5f, bottom = area.bottom - 0.5f;
        float u = float(area.right - area.left) / desc.Width;
        float v = float(area.bottom - area.top) / desc.Height;
        Vertex vertices[] = { {left, top, 0, 1, 0xffffffff, 0, 0},
            {right, top, 0, 1, 0xffffffff, u, 0}, {left, bottom, 0, 1, 0xffffffff, 0, v},
            {right, bottom, 0, 1, 0xffffffff, u, v} };
        device.p->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        device.p->SetTexture(0, texture.p);
        HRESULT result = device.p->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));
        state.p->Apply();
        return SUCCEEDED(result) ? height : 0;
    }
}
