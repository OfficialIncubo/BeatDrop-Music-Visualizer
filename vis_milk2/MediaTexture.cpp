/*
  Animated and live texture support for BeatDrop sprites and shader samplers.
*/

#include "MediaTexture.h"

#include "../spoutDX9/SpoutDX9.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <errno.h>
#include <excpt.h>
#include <mmsystem.h>
#include <propvarutil.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

// The June 2010 DirectX SDK dxgitype.h can be picked before the Windows SDK
// version. Newer wincodec.h declares JPEG interfaces with these structs.
#if defined(__dxgitype_h__) && defined(DXGI_USAGE_UNORDERED_ACCESS)
typedef struct DXGI_JPEG_DC_HUFFMAN_TABLE
{
    BYTE CodeCounts[12];
    BYTE CodeValues[12];
} DXGI_JPEG_DC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_AC_HUFFMAN_TABLE
{
    BYTE CodeCounts[16];
    BYTE CodeValues[162];
} DXGI_JPEG_AC_HUFFMAN_TABLE;

typedef struct DXGI_JPEG_QUANTIZATION_TABLE
{
    BYTE Elements[64];
} DXGI_JPEG_QUANTIZATION_TABLE;
#endif

#pragma push_macro("_WIN32_WINNT")
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#include <wincodec.h>
#pragma pop_macro("_WIN32_WINNT")

namespace
{
    template <class T>
    void SafeReleaseLocal(T*& p)
    {
        if (p)
        {
            p->Release();
            p = nullptr;
        }
    }

    bool EqualExt(const wchar_t* path, const wchar_t* ext)
    {
        const wchar_t* dot = wcsrchr(path, L'.');
        return dot && _wcsicmp(dot + 1, ext) == 0;
    }

    bool HasAnyExt(const wchar_t* path, const wchar_t* const* exts, int count)
    {
        for (int i = 0; i < count; i++)
        {
            if (EqualExt(path, exts[i]))
                return true;
        }
        return false;
    }

    bool StartsWithSpoutPrefix(const wchar_t* path)
    {
        return path &&
            (_wcsnicmp(path, L"Spout\\", 6) == 0 ||
             _wcsnicmp(path, L"Spout/", 6) == 0);
    }

    bool IsReasonableMediaSize(UINT width, UINT height)
    {
        const UINT maxDimension = 8192;
        const unsigned long long maxBytes = 512ULL * 1024ULL * 1024ULL;
        if (width == 0 || height == 0 || width > maxDimension || height > maxDimension)
            return false;

        return (unsigned long long)width * (unsigned long long)height * 4ULL <= maxBytes;
    }

    void UnbindTextureFromDevice(LPDIRECT3DDEVICE9EX device, LPDIRECT3DBASETEXTURE9 texture)
    {
        if (!device || !texture)
            return;

        for (DWORD stage = 0; stage < 16; stage++)
        {
            LPDIRECT3DBASETEXTURE9 bound = nullptr;
            if (SUCCEEDED(device->GetTexture(stage, &bound)) && bound)
            {
                if (bound == texture)
                    device->SetTexture(stage, nullptr);
                bound->Release();
            }
        }
    }

    bool ReadMetadataUInt(IWICMetadataQueryReader* reader, const wchar_t* name, UINT* value)
    {
        if (!reader || !value)
            return false;

        PROPVARIANT var;
        PropVariantInit(&var);
        HRESULT hr = reader->GetMetadataByName(name, &var);
        if (FAILED(hr))
        {
            PropVariantClear(&var);
            return false;
        }

        bool ok = true;
        switch (var.vt)
        {
        case VT_UI1: *value = var.bVal; break;
        case VT_UI2: *value = var.uiVal; break;
        case VT_UI4: *value = var.ulVal; break;
        case VT_UINT: *value = var.uintVal; break;
        default: ok = false; break;
        }

        PropVariantClear(&var);
        return ok;
    }

    void CompositeBGRA(std::vector<unsigned char>& canvas, int canvasW, int canvasH,
        const unsigned char* src, int srcW, int srcH, int left, int top)
    {
        if (!src || canvas.empty())
            return;

        for (int y = 0; y < srcH; y++)
        {
            int dstY = top + y;
            if (dstY < 0 || dstY >= canvasH)
                continue;

            for (int x = 0; x < srcW; x++)
            {
                int dstX = left + x;
                if (dstX < 0 || dstX >= canvasW)
                    continue;

                const unsigned char* s = src + (y * srcW + x) * 4;
                unsigned char* d = canvas.data() + (dstY * canvasW + dstX) * 4;
                unsigned int a = s[3];

                if (a == 0)
                    continue;
                if (a == 255)
                {
                    d[0] = s[0];
                    d[1] = s[1];
                    d[2] = s[2];
                    d[3] = s[3];
                    continue;
                }

                unsigned int invA = 255 - a;
                d[0] = (unsigned char)(s[0] + (d[0] * invA + 127) / 255);
                d[1] = (unsigned char)(s[1] + (d[1] * invA + 127) / 255);
                d[2] = (unsigned char)(s[2] + (d[2] * invA + 127) / 255);
                unsigned int outA = a + (d[3] * invA + 127) / 255;
                d[3] = (unsigned char)(outA > 255 ? 255 : outA);
            }
        }
    }

    void ClearRect(std::vector<unsigned char>& canvas, int canvasW, int canvasH, int left, int top, int w, int h)
    {
        for (int y = 0; y < h; y++)
        {
            int dstY = top + y;
            if (dstY < 0 || dstY >= canvasH)
                continue;

            for (int x = 0; x < w; x++)
            {
                int dstX = left + x;
                if (dstX < 0 || dstX >= canvasW)
                    continue;

                unsigned char* d = canvas.data() + (dstY * canvasW + dstX) * 4;
                d[0] = d[1] = d[2] = d[3] = 0;
            }
        }
    }

    std::string WideToAnsi(const wchar_t* text)
    {
        if (!text)
            return std::string();

        int needed = WideCharToMultiByte(CP_ACP, 0, text, -1, nullptr, 0, nullptr, nullptr);
        if (needed <= 1)
            return std::string();

        std::string out;
        out.resize(needed - 1);
        WideCharToMultiByte(CP_ACP, 0, text, -1, &out[0], needed, nullptr, nullptr);
        return out;
    }

    std::string WideToUtf8(const wchar_t* text)
    {
        if (!text)
            return std::string();

        int needed = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
        if (needed <= 1)
            return std::string();

        std::string out;
        out.resize(needed - 1);
        WideCharToMultiByte(CP_UTF8, 0, text, -1, &out[0], needed, nullptr, nullptr);
        return out;
    }
}

MediaTexture::MediaTexture(LPDIRECT3DDEVICE9EX device)
    : m_device(device)
    , m_texture(nullptr)
    , m_kind(KIND_NONE)
    , m_width(0)
    , m_height(0)
    , m_comInitialized(false)
    , m_gifTotalSeconds(0.0)
    , m_gifFrameIndex(-1)
    , m_gifColorKey(0)
    , m_gifApplyColorKey(false)
    , m_formatContext(nullptr)
    , m_codecContext(nullptr)
    , m_videoFrame(nullptr)
    , m_videoPacket(nullptr)
    , m_swsContext(nullptr)
    , m_videoStreamIndex(-1)
    , m_videoSrcWidth(0)
    , m_videoSrcHeight(0)
    , m_videoSrcFormat(-1)
    , m_videoReachedEnd(false)
    , m_videoStopWorker(false)
    , m_videoPendingFrame(false)
    , m_videoPendingWidth(0)
    , m_videoPendingHeight(0)
    , m_videoFrameSeconds(1.0 / 30.0)
    , m_videoLastFrameTime(0)
    , m_videoHaveLastFrameTime(false)
    , m_videoColorKey(0)
    , m_videoApplyColorKey(false)
    , m_spoutReceiver(nullptr)
    , m_spoutWasConnected(false)
    , m_spoutColorKey(0)
    , m_spoutHasColorKey(false)
{
    if (m_device)
        m_device->AddRef();
}

MediaTexture::~MediaTexture()
{
    ReleaseVideo();
    ReleaseSpout(false);
    ReleaseTexture();

    if (m_comInitialized)
        CoUninitialize();

    SafeReleaseLocal(m_device);
}

bool MediaTexture::GetColorKey(unsigned int* outColorKey) const
{
    if (m_kind != KIND_SPOUT || !m_spoutHasColorKey)
        return false;

    if (outColorKey)
        *outColorKey = m_spoutColorKey;
    return true;
}

bool MediaTexture::IsGifFile(const wchar_t* path)
{
    return path && EqualExt(path, L"gif");
}

bool MediaTexture::IsVideoFile(const wchar_t* path)
{
    static const wchar_t* const videoExts[] =
    {
        L"3gp", L"3g2", L"mp4", L"wmv", L"mkv", L"avi", L"mxf",
        L"mov", L"webm", L"flv", L"mpg", L"mpeg"
    };
    return path && HasAnyExt(path, videoExts, sizeof(videoExts) / sizeof(videoExts[0]));
}

bool MediaTexture::IsSupportedFile(const wchar_t* path)
{
    return IsGifFile(path) || IsVideoFile(path);
}

bool MediaTexture::IsSpoutPath(const wchar_t* path)
{
    return StartsWithSpoutPrefix(path);
}

MediaTexture* MediaTexture::Create(LPDIRECT3DDEVICE9EX device, const wchar_t* path, unsigned int colorKey, bool allowSpout)
{
    if (!device || !path || !path[0])
        return nullptr;

    if (device->TestCooperativeLevel() != D3D_OK)
        return nullptr;

    if (allowSpout && IsSpoutPath(path))
    {
        MediaTexture* media = new MediaTexture(device);
        if (media->InitSpout(path, colorKey))
            return media;
        delete media;
        return nullptr;
    }

    if (IsGifFile(path))
    {
        MediaTexture* media = new MediaTexture(device);
        if (media->InitGif(path, colorKey))
            return media;
        delete media;
        return nullptr;
    }

    if (IsVideoFile(path))
    {
        MediaTexture* media = new MediaTexture(device);
        if (media->InitVideo(path, colorKey))
            return media;
        delete media;
        return nullptr;
    }

    return nullptr;
}

bool MediaTexture::CreateDynamicTexture(int width, int height)
{
    if (!m_device || width <= 0 || height <= 0)
        return false;

    if (m_device->TestCooperativeLevel() != D3D_OK)
        return false;

    if (m_texture)
    {
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texture->GetLevelDesc(0, &desc)) &&
            (int)desc.Width == width &&
            (int)desc.Height == height)
        {
            return true;
        }
        ReleaseTexture();
    }

    HRESULT hr = m_device->CreateTexture(
        width,
        height,
        1,
        D3DUSAGE_DYNAMIC,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        &m_texture,
        nullptr);

    if (FAILED(hr))
        return false;

    m_width = width;
    m_height = height;
    return true;
}

void MediaTexture::ReleaseTexture()
{
    UnbindTextureFromDevice(m_device, m_texture);
    SafeReleaseLocal(m_texture);
}

void MediaTexture::ReleaseVideo()
{
    StopVideoWorker();

    if (m_swsContext)
    {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }

    if (m_videoPacket)
        av_packet_free(&m_videoPacket);

    if (m_videoFrame)
        av_frame_free(&m_videoFrame);

    if (m_codecContext)
        avcodec_free_context(&m_codecContext);

    if (m_formatContext)
        avformat_close_input(&m_formatContext);

    m_videoStreamIndex = -1;
    m_videoSrcWidth = 0;
    m_videoSrcHeight = 0;
    m_videoSrcFormat = -1;
    m_videoReachedEnd = false;
    m_videoPendingPixels.clear();
    m_videoPendingFrame = false;
    m_videoPendingWidth = 0;
    m_videoPendingHeight = 0;
    m_videoLastFrameTime = 0.0;
    m_videoHaveLastFrameTime = false;
}

void MediaTexture::ReleaseSpout(bool waitForSpout)
{
    if (!m_spoutReceiver)
        return;

    ReleaseTexture();
    m_spoutReceiver->ReleaseReceiver(waitForSpout);
    delete m_spoutReceiver;
    m_spoutReceiver = nullptr;
    m_spoutWasConnected = false;
}

bool MediaTexture::StartVideoWorker()
{
    if (m_videoThread.joinable())
        return true;

    {
        std::lock_guard<std::mutex> lock(m_videoMutex);
        m_videoStopWorker = false;
        m_videoPendingFrame = false;
    }

    try
    {
        m_videoThread = std::thread(&MediaTexture::VideoWorkerMain, this);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

void MediaTexture::StopVideoWorker()
{
    {
        std::lock_guard<std::mutex> lock(m_videoMutex);
        m_videoStopWorker = true;
    }
    m_videoCondition.notify_all();

    if (m_videoThread.joinable())
        m_videoThread.join();

    {
        std::lock_guard<std::mutex> lock(m_videoMutex);
        m_videoStopWorker = false;
        m_videoPendingFrame = false;
        m_videoPendingPixels.clear();
        m_videoPendingWidth = 0;
        m_videoPendingHeight = 0;
    }
}

void MediaTexture::VideoWorkerMain()
{
    std::vector<unsigned char> decodedPixels;

    for (;;)
    {
        {
            std::unique_lock<std::mutex> lock(m_videoMutex);
            m_videoCondition.wait(lock, [this]()
            {
                return m_videoStopWorker || !m_videoPendingFrame;
            });

            if (m_videoStopWorker)
                break;
        }

        int width = 0;
        int height = 0;
        decodedPixels.clear();
        if (!DecodeNextVideoFrame(decodedPixels, &width, &height, true))
        {
            Sleep(2);
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(m_videoMutex);
            if (m_videoStopWorker)
                break;

            m_videoPendingPixels.swap(decodedPixels);
            m_videoPendingWidth = width;
            m_videoPendingHeight = height;
            m_videoPendingFrame = true;
        }
        m_videoCondition.notify_all();
    }
}

bool MediaTexture::UploadPendingVideoFrame()
{
    std::vector<unsigned char> pixels;
    int width = 0;
    int height = 0;

    {
        std::lock_guard<std::mutex> lock(m_videoMutex);
        if (!m_videoPendingFrame)
            return false;

        pixels.swap(m_videoPendingPixels);
        width = m_videoPendingWidth;
        height = m_videoPendingHeight;
        m_videoPendingFrame = false;
        m_videoPendingWidth = 0;
        m_videoPendingHeight = 0;
    }
    m_videoCondition.notify_all();

    if (width <= 0 || height <= 0 || pixels.empty())
        return false;

    return UploadPixels(pixels.data(), width, height, m_videoColorKey, m_videoApplyColorKey, false);
}

bool MediaTexture::UploadPixels(const unsigned char* pixels, int width, int height, unsigned int colorKey, bool applyColorKey, bool forceOpaque)
{
    if (!pixels || !CreateDynamicTexture(width, height))
        return false;

    D3DLOCKED_RECT lr;
    HRESULT hr = m_texture->LockRect(0, &lr, nullptr, D3DLOCK_DISCARD);
    if (FAILED(hr))
        return false;

    if (!applyColorKey && !forceOpaque)
    {
        const int rowBytes = width * 4;
        for (int y = 0; y < height; y++)
        {
            const unsigned char* src = pixels + y * rowBytes;
            unsigned char* dst = (unsigned char*)lr.pBits + y * lr.Pitch;
            std::memcpy(dst, src, rowBytes);
        }

        m_texture->UnlockRect(0);
        return true;
    }

    // Colorkey/force-opaque path: still O(width*height) since every pixel has
    // to be checked, but operates a whole BGRA pixel at a time (one 32-bit
    // read/compare/write) instead of unpacking and repacking 4 separate
    // bytes per pixel - meaningfully cheaper per pixel than the old version
    // of this loop for sprites that do genuinely need a colorkey (this is
    // still slower than the plain memcpy path above, which is why video only
    // takes this path on frames where blendmode is actually 4 - see
    // wantColorKeyThisFrame in Update()/texmgr::UpdateTex - rather than on
    // every frame regardless of blendmode).
    // colorKey is 0x00RRGGBB. A raw little-endian read of 4 source bytes
    // stored in memory as B,G,R,A packs into an int as B | G<<8 | R<<16 |
    // A<<24 - i.e. masking off the top byte gives 0x00RRGGBB too, so
    // colorKey compares directly against a masked pixel with no repacking.
    const unsigned int keyMasked = colorKey & 0x00FFFFFFu;

    for (int y = 0; y < height; y++)
    {
        const unsigned char* srcRow = pixels + (size_t)y * width * 4;
        unsigned char* dstRow = (unsigned char*)lr.pBits + (size_t)y * lr.Pitch;

        for (int x = 0; x < width; x++)
        {
            unsigned int pixel;
            std::memcpy(&pixel, srcRow + (size_t)x * 4, 4);

            if (forceOpaque)
                pixel |= 0xFF000000u;
            if (applyColorKey && (pixel & 0x00FFFFFFu) == keyMasked)
                pixel &= 0x00FFFFFFu; // alpha = 0, color unchanged

            std::memcpy(dstRow + (size_t)x * 4, &pixel, 4);
        }
    }

    m_texture->UnlockRect(0);
    return true;
}

bool MediaTexture::InitGif(const wchar_t* path, unsigned int colorKey)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
        m_comInitialized = true;
    else if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return false;

    IWICImagingFactory* factory = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory));
    if (FAILED(hr))
        return false;

    hr = factory->CreateDecoderFromFilename(
        path,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder);
    if (FAILED(hr))
    {
        SafeReleaseLocal(factory);
        return false;
    }

    UINT frameCount = 0;
    decoder->GetFrameCount(&frameCount);
    if (frameCount == 0)
    {
        SafeReleaseLocal(decoder);
        SafeReleaseLocal(factory);
        return false;
    }

    UINT canvasW = 0;
    UINT canvasH = 0;
    IWICMetadataQueryReader* decoderMeta = nullptr;
    if (SUCCEEDED(decoder->GetMetadataQueryReader(&decoderMeta)))
    {
        ReadMetadataUInt(decoderMeta, L"/logscrdesc/Width", &canvasW);
        ReadMetadataUInt(decoderMeta, L"/logscrdesc/Height", &canvasH);
        SafeReleaseLocal(decoderMeta);
    }

    if (canvasW == 0 || canvasH == 0)
    {
        IWICBitmapFrameDecode* firstFrame = nullptr;
        if (FAILED(decoder->GetFrame(0, &firstFrame)) || FAILED(firstFrame->GetSize(&canvasW, &canvasH)))
        {
            SafeReleaseLocal(firstFrame);
            SafeReleaseLocal(decoder);
            SafeReleaseLocal(factory);
            return false;
        }
        SafeReleaseLocal(firstFrame);
    }

    if (!IsReasonableMediaSize(canvasW, canvasH))
    {
        SafeReleaseLocal(decoder);
        SafeReleaseLocal(factory);
        return false;
    }

    std::vector<unsigned char> canvas;
    try
    {
        canvas.assign((size_t)canvasW * (size_t)canvasH * 4, 0);
    }
    catch (...)
    {
        SafeReleaseLocal(decoder);
        SafeReleaseLocal(factory);
        return false;
    }

    std::vector<unsigned char> restoreCanvas;
    UINT prevDisposal = 0;
    UINT prevLeft = 0, prevTop = 0, prevW = 0, prevH = 0;
    bool haveRestoreCanvas = false;

    m_gifFrames.clear();
    m_gifTotalSeconds = 0.0;

    for (UINT i = 0; i < frameCount; i++)
    {
        if (prevDisposal == 2)
        {
            ClearRect(canvas, canvasW, canvasH, prevLeft, prevTop, prevW, prevH);
        }
        else if (prevDisposal == 3 && haveRestoreCanvas)
        {
            canvas = restoreCanvas;
            haveRestoreCanvas = false;
        }

        IWICBitmapFrameDecode* frame = nullptr;
        IWICFormatConverter* converter = nullptr;
        IWICMetadataQueryReader* frameMeta = nullptr;

        if (FAILED(decoder->GetFrame(i, &frame)))
            continue;

        UINT frameW = 0;
        UINT frameH = 0;
        frame->GetSize(&frameW, &frameH);
        if (!IsReasonableMediaSize(frameW, frameH))
        {
            SafeReleaseLocal(frame);
            continue;
        }

        UINT left = 0;
        UINT top = 0;
        UINT delay = 10;
        UINT disposal = 0;

        if (SUCCEEDED(frame->GetMetadataQueryReader(&frameMeta)))
        {
            ReadMetadataUInt(frameMeta, L"/imgdesc/Left", &left);
            ReadMetadataUInt(frameMeta, L"/imgdesc/Top", &top);
            ReadMetadataUInt(frameMeta, L"/grctlext/Delay", &delay);
            ReadMetadataUInt(frameMeta, L"/grctlext/Disposal", &disposal);
        }

        if (delay < 2)
            delay = 10;

        if (disposal == 3)
        {
            restoreCanvas = canvas;
            haveRestoreCanvas = true;
        }

        std::vector<unsigned char> framePixels;
        try
        {
            framePixels.assign((size_t)frameW * (size_t)frameH * 4, 0);
        }
        catch (...)
        {
            SafeReleaseLocal(frameMeta);
            SafeReleaseLocal(frame);
            continue;
        }

        hr = factory->CreateFormatConverter(&converter);
        if (SUCCEEDED(hr))
        {
            hr = converter->Initialize(
                frame,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0,
                WICBitmapPaletteTypeCustom);
        }

        if (SUCCEEDED(hr))
        {
            WICRect rc = { 0, 0, (INT)frameW, (INT)frameH };
            hr = converter->CopyPixels(&rc, frameW * 4, (UINT)framePixels.size(), framePixels.data());
        }

        if (SUCCEEDED(hr))
        {
            CompositeBGRA(canvas, canvasW, canvasH, framePixels.data(), frameW, frameH, left, top);

            GifFrame outFrame;
            outFrame.pixels = canvas;
            outFrame.delaySeconds = delay / 100.0;
            m_gifTotalSeconds += outFrame.delaySeconds;
            m_gifFrames.push_back(outFrame);
        }

        prevDisposal = disposal;
        prevLeft = left;
        prevTop = top;
        prevW = frameW;
        prevH = frameH;

        SafeReleaseLocal(frameMeta);
        SafeReleaseLocal(converter);
        SafeReleaseLocal(frame);
    }

    SafeReleaseLocal(decoder);
    SafeReleaseLocal(factory);

    if (m_gifFrames.empty())
        return false;

    m_kind = KIND_GIF;
    m_width = (int)canvasW;
    m_height = (int)canvasH;
    m_gifFrameIndex = 0;
    m_gifColorKey = colorKey;
    m_gifApplyColorKey = true;
    if (m_gifTotalSeconds <= 0.0)
        m_gifTotalSeconds = 0.1;

    return UploadPixels(m_gifFrames[0].pixels.data(), m_width, m_height, colorKey, true, false);
}

bool MediaTexture::InitVideo(const wchar_t* path, unsigned int colorKey)
{
    std::string utf8Path = WideToUtf8(path);
    if (utf8Path.empty())
        return false;

    if (avformat_open_input(&m_formatContext, utf8Path.c_str(), nullptr, nullptr) < 0)
        return false;

    if (avformat_find_stream_info(m_formatContext, nullptr) < 0)
        return false;

    int streamIndex = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (streamIndex < 0)
        return false;

    AVStream* stream = m_formatContext->streams[streamIndex];
    if (!stream || !stream->codecpar)
        return false;

    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
        return false;

    m_codecContext = avcodec_alloc_context3(codec);
    if (!m_codecContext)
        return false;

    if (avcodec_parameters_to_context(m_codecContext, stream->codecpar) < 0)
        return false;

    if (avcodec_open2(m_codecContext, codec, nullptr) < 0)
        return false;

    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();
    if (!frame || !packet)
    {
        if (frame)
            av_frame_free(&frame);
        if (packet)
            av_packet_free(&packet);
        return false;
    }

    m_videoFrame = frame;
    m_videoPacket = packet;
    m_videoStreamIndex = streamIndex;

    AVRational fps = av_guess_frame_rate(m_formatContext, stream, nullptr);
    if (fps.num <= 0 || fps.den <= 0)
        fps = stream->avg_frame_rate;
    if (fps.num <= 0 || fps.den <= 0)
        fps = stream->r_frame_rate;
    if (fps.num > 0 && fps.den > 0)
    {
        double seconds = (double)fps.den / (double)fps.num;
        if (seconds > 0.0 && seconds < 1.0)
            m_videoFrameSeconds = seconds;
    }

    m_kind = KIND_VIDEO;
    m_width = m_codecContext->width > 0 ? m_codecContext->width : stream->codecpar->width;
    m_height = m_codecContext->height > 0 ? m_codecContext->height : stream->codecpar->height;
    m_videoColorKey = colorKey;
    // Whether colorkey actually gets applied is now decided fresh every
    // frame in Update(), based on whether this sprite's blendmode is
    // *currently* 4 (see wantColorKeyThisFrame there, and the comment on
    // Update() in MediaTexture.h for the full reasoning). m_videoApplyColorKey
    // just needs a safe starting value here; Update() overwrites it every
    // call before it's used.
    m_videoApplyColorKey = false;
    m_videoReachedEnd = false;

    if (m_width > 0 && m_height > 0 && !CreateDynamicTexture(m_width, m_height))
        return false;

    std::vector<unsigned char> firstFrame;
    int firstWidth = 0;
    int firstHeight = 0;
    if (!DecodeNextVideoFrame(firstFrame, &firstWidth, &firstHeight, false))
        return false;

    if (!UploadPixels(firstFrame.data(), firstWidth, firstHeight, colorKey, m_videoApplyColorKey, false))
        return false;

    // Note: the frame-timing baseline (m_videoLastFrameTime) is intentionally left
    // unset here; it's established on the first Update(now, ...) call using the
    // visualizer's own high-resolution clock rather than the OS wall clock, so
    // video playback stays in sync with the app's animation timeline (same
    // approach GIF playback already uses).
    StartVideoWorker();
    return true;
}

bool MediaTexture::SeekVideoToStart()
{
    if (!m_formatContext || !m_codecContext || m_videoStreamIndex < 0)
        return false;

    int64_t timestamp = 0;
    if (m_videoStreamIndex < (int)m_formatContext->nb_streams)
    {
        AVStream* stream = m_formatContext->streams[m_videoStreamIndex];
        if (stream && stream->start_time != AV_NOPTS_VALUE)
            timestamp = stream->start_time;
    }

    int ret = av_seek_frame(m_formatContext, m_videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
    if (ret < 0)
        ret = av_seek_frame(m_formatContext, -1, 0, AVSEEK_FLAG_BACKWARD);
    if (ret < 0)
        return false;

    avcodec_flush_buffers(m_codecContext);
    m_videoReachedEnd = false;
    return true;
}

bool MediaTexture::DecodeNextVideoFrame(std::vector<unsigned char>& pixels, int* width, int* height, bool loop)
{
    if (!m_formatContext || !m_codecContext || !m_videoFrame || !m_videoPacket || m_videoStreamIndex < 0)
        return false;

    if (width)
        *width = 0;
    if (height)
        *height = 0;

    for (int attempt = 0; attempt < 4096; attempt++)
    {
        int ret = avcodec_receive_frame(m_codecContext, m_videoFrame);
        if (ret == 0)
        {
            int srcW = m_videoFrame->width;
            int srcH = m_videoFrame->height;
            int srcFormat = m_videoFrame->format;
            if (srcW <= 0 || srcH <= 0 || srcFormat < 0)
            {
                av_frame_unref(m_videoFrame);
                return false;
            }

            if (!m_swsContext ||
                m_videoSrcWidth != srcW ||
                m_videoSrcHeight != srcH ||
                m_videoSrcFormat != srcFormat)
            {
                m_swsContext = sws_getCachedContext(
                    m_swsContext,
                    srcW,
                    srcH,
                    (AVPixelFormat)srcFormat,
                    srcW,
                    srcH,
                    AV_PIX_FMT_BGRA,
                    SWS_FAST_BILINEAR,
                    nullptr,
                    nullptr,
                    nullptr);
                if (!m_swsContext)
                {
                    av_frame_unref(m_videoFrame);
                    return false;
                }

                m_videoSrcWidth = srcW;
                m_videoSrcHeight = srcH;
                m_videoSrcFormat = srcFormat;
            }

            size_t pixelBytes = (size_t)srcW * (size_t)srcH * 4;
            if (pixelBytes == 0)
            {
                av_frame_unref(m_videoFrame);
                return false;
            }

            pixels.resize(pixelBytes);
            uint8_t* dstData[4] = { pixels.data(), nullptr, nullptr, nullptr };
            int dstStride[4] = { srcW * 4, 0, 0, 0 };
            int convertedRows = sws_scale(
                m_swsContext,
                m_videoFrame->data,
                m_videoFrame->linesize,
                0,
                srcH,
                dstData,
                dstStride);

            av_frame_unref(m_videoFrame);
            if (convertedRows != srcH)
                return false;

            m_width = srcW;
            m_height = srcH;
            if (width)
                *width = srcW;
            if (height)
                *height = srcH;
            return true;
        }

        if (ret == AVERROR_EOF)
        {
            if (!loop || !SeekVideoToStart())
                return false;
            continue;
        }

        if (ret != AVERROR(EAGAIN))
            return false;

        if (m_videoReachedEnd)
        {
            if (!loop || !SeekVideoToStart())
                return false;
            continue;
        }

        ret = av_read_frame(m_formatContext, m_videoPacket);
        if (ret < 0)
        {
            avcodec_send_packet(m_codecContext, nullptr);
            m_videoReachedEnd = true;
            continue;
        }

        if (m_videoPacket->stream_index == m_videoStreamIndex)
        {
            ret = avcodec_send_packet(m_codecContext, m_videoPacket);
            av_packet_unref(m_videoPacket);
            if (ret < 0 && ret != AVERROR(EAGAIN))
                return false;
        }
        else
        {
            av_packet_unref(m_videoPacket);
        }
    }

    return false;
}

bool MediaTexture::InitSpout(const wchar_t* path, unsigned int colorKey)
{
    if (!m_device || !StartsWithSpoutPrefix(path))
        return false;

    const wchar_t* senderNameW = path + 6;
    m_spoutSenderName = WideToAnsi(senderNameW);
    if (m_spoutSenderName.empty())
        return false;

    // Stored for GetColorKey(); a live Spout texture can't have its colorkey
    // baked into its alpha channel on the CPU like GIF/Video do (see the class
    // comment on GetColorKey), so the renderer applies it via a pixel shader
    // instead when blendmode 4 is selected on a Spout sprite.
    m_spoutColorKey = colorKey;
    m_spoutHasColorKey = true;

    m_spoutReceiver = new spoutDX9();
    m_spoutReceiver->SetDX9device(m_device);
    m_spoutReceiver->SetReceiverName(m_spoutSenderName.c_str());

    unsigned int senderW = 0;
    unsigned int senderH = 0;
    HANDLE shareHandle = nullptr;
    DWORD senderFormat = 0;
    if (!m_spoutReceiver->GetSenderInfo(m_spoutSenderName.c_str(), senderW, senderH, shareHandle, senderFormat))
        return false;

    m_width = (int)senderW;
    m_height = (int)senderH;
    m_kind = KIND_SPOUT;

    bool received = m_spoutReceiver->ReceiveDX9Texture(m_texture);
    m_spoutWasConnected = received || m_spoutReceiver->IsConnected();

    if (m_texture)
    {
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texture->GetLevelDesc(0, &desc)))
        {
            m_width = (int)desc.Width;
            m_height = (int)desc.Height;
        }
    }

    return m_texture != nullptr;
}

bool MediaTexture::Update(float now, bool* shouldKill, bool wantColorKeyThisFrame)
{
    if (shouldKill)
        *shouldKill = false;

    switch (m_kind)
    {
    case KIND_GIF:
    {
        if (m_gifFrames.empty())
            return m_texture != nullptr;

        double t = fmod((double)now, m_gifTotalSeconds);
        double cursor = 0.0;
        int frameIndex = 0;
        for (int i = 0; i < (int)m_gifFrames.size(); i++)
        {
            cursor += m_gifFrames[i].delaySeconds;
            if (t < cursor)
            {
                frameIndex = i;
                break;
            }
        }

        if (frameIndex != m_gifFrameIndex)
        {
            m_gifFrameIndex = frameIndex;
            return UploadPixels(m_gifFrames[frameIndex].pixels.data(), m_width, m_height, m_gifColorKey, m_gifApplyColorKey, false);
        }
        return true;
    }
    case KIND_VIDEO:
    {
        // Colorkey only has an effect when THIS frame's blendmode is
        // actually 4 (matches the documented MilkDrop behavior classic
        // images already have, and GIF's own colorkey handling) - decided
        // fresh every call rather than once at load time. See the comment
        // on Update()'s declaration in MediaTexture.h.
        m_videoApplyColorKey = wantColorKeyThisFrame;

        // Use the visualizer's own high-resolution animation clock ('now', a
        // damped/smoothed real-time seconds value - see CPluginShell::GetTime())
        // instead of the OS wall clock (timeGetTime()). This keeps video frame
        // presentation synced to the video's own native FPS (m_videoFrameSeconds,
        // read from the container's stream metadata in InitVideo) rather than
        // drifting or feeling locked to a fixed rate, and is consistent with how
        // GIF playback already times itself off of 'now'.
        double elapsedSeconds = m_videoHaveLastFrameTime ? ((double)now - m_videoLastFrameTime) : m_videoFrameSeconds;
        double frameSeconds = m_videoFrameSeconds > 0.0 ? m_videoFrameSeconds : (1.0 / 30.0);

        // Guard against a negative/garbage delta (e.g. 'now' wrapped or was reset)
        // by treating it the same as "time for a new frame" rather than stalling.
        if (!m_videoHaveLastFrameTime || elapsedSeconds < 0.0 || elapsedSeconds >= frameSeconds)
        {
            if (UploadPendingVideoFrame())
            {
                m_videoLastFrameTime = (double)now;
                m_videoHaveLastFrameTime = true;
            }
        }
        return m_texture != nullptr;
    }
    case KIND_SPOUT:
    {
        if (!m_spoutReceiver)
            return false;

        bool received = m_spoutReceiver->ReceiveDX9Texture(m_texture);
        bool connected = received || m_spoutReceiver->IsConnected();

        if (connected)
        {
            m_spoutWasConnected = true;
            if (m_texture)
            {
                D3DSURFACE_DESC desc;
                if (SUCCEEDED(m_texture->GetLevelDesc(0, &desc)))
                {
                    m_width = (int)desc.Width;
                    m_height = (int)desc.Height;
                }
            }
            return m_texture != nullptr;
        }

        if (m_spoutWasConnected && shouldKill)
            *shouldKill = true;
        return false;
    }
    default:
        return m_texture != nullptr;
    }
}
