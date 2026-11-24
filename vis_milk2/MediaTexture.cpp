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
#include <memory>
#include <mmsystem.h>
#include <propvarutil.h>
#include <random>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/rational.h>
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
    constexpr size_t kVideoQueueMaxFrames = 5;
    constexpr size_t kVideoQueueMinFrames = 2;
    constexpr uint64_t kVideoQueueBytes32 = 64ULL * 1024 * 1024;
    constexpr size_t kVideoStartupFrames = 3;
    constexpr uint64_t kDefaultVideoMemoryBudget = (sizeof(void*) == 4 ? 768ULL : 2048ULL) * 1024 * 1024;
    constexpr int64_t kRandomStartThresholdSeconds = 120;
    constexpr double kRandomStartEndMarginSeconds = 5.0;
    constexpr double kVideoTimestampToleranceSeconds = 0.0005;
    constexpr double kVideoStallSeconds = 0.25;
    constexpr auto kVideoRenderGap = std::chrono::milliseconds(500);
    std::mutex g_videoAdmissionMutex;
    size_t g_videoReservedBytes = 0;
    unsigned int g_videoExtraThreads = 0;

    int ReserveVideoDecoder(uint64_t frameBytes, uint64_t convertedFrameBytes, size_t extraConvertedFrames,
        unsigned int preferredThreads, size_t& reservedBytes, size_t& queueCapacity)
    {
        // Leave CPU capacity for the renderer and for other sprite workers.
        static const unsigned int extraThreadLimit = (std::min)(8u, std::thread::hardware_concurrency() / 2);
        uint64_t memoryBudget = kDefaultVideoMemoryBudget;
        uint64_t availableBytes = UINT64_MAX;
        MEMORYSTATUSEX memory = {};
        memory.dwLength = sizeof(memory);
        if (GlobalMemoryStatusEx(&memory))
        {
            // Allow more independent clips when the machine and process have room.
            const uint64_t capacity = (std::min)(memory.ullTotalVirtual / 3, memory.ullTotalPhys / 4);
            memoryBudget = (std::max)(memoryBudget, capacity);
            availableBytes = (std::min)(memory.ullAvailVirtual, memory.ullAvailPageFile);
        }

        std::lock_guard<std::mutex> lock(g_videoAdmissionMutex);
        const unsigned int extraAvailable = extraThreadLimit - g_videoExtraThreads;
        // A deep queue of native 4K frames can crowd out other clips on x86.
        const size_t maxQueueFrames = sizeof(void*) == 4 ?
            (std::clamp)(static_cast<size_t>(kVideoQueueBytes32 / convertedFrameBytes),
                kVideoQueueMinFrames, kVideoQueueMaxFrames) : kVideoQueueMaxFrames;
        for (unsigned int threads = (std::min)(preferredThreads, extraAvailable + 1); threads > 0; --threads)
        {
            // Each extra thread can hold another decoded frame.
            constexpr unsigned int baseDecoderFrames = 20;
            const uint64_t decoderBytes = frameBytes * (baseDecoderFrames + threads - 1);
            for (size_t queueFrames = maxQueueFrames; queueFrames >= kVideoQueueMinFrames; --queueFrames)
            {
                const uint64_t bytes = convertedFrameBytes * (queueFrames + extraConvertedFrames) + decoderBytes;
                if (bytes + g_videoReservedBytes > memoryBudget || bytes > availableBytes)
                    continue;

                queueCapacity = queueFrames;
                reservedBytes = static_cast<size_t>(bytes);
                g_videoReservedBytes += reservedBytes;
                g_videoExtraThreads += threads - 1;
                return static_cast<int>(threads);
            }
        }
        return 0;
    }

    int GetVideoFrameBytes(AVPixelFormat format, int width, int height)
    {
        return av_image_get_buffer_size(format, (width + 63) & ~63, (height + 63) & ~63, 64);
    }

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
        out.resize(needed);
        if (WideCharToMultiByte(CP_UTF8, 0, text, -1, &out[0], needed, nullptr, nullptr) != needed)
            return {};
        out.pop_back();
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
    , m_gifHavePlaybackStart(false)
    , m_formatContext(nullptr)
    , m_codecContext(nullptr)
    , m_videoFrame(nullptr)
    , m_videoPacket(nullptr)
    , m_swsContext(nullptr)
    , m_videoStreamIndex(-1)
    , m_videoTimeBaseSeconds(0.0)
    , m_videoFrameSeconds(1.0 / 30.0)
    , m_videoColorKey(0)
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
    UnbindTextureFromDevice(m_device, m_videoKeyedTexture);
    SafeReleaseLocal(m_videoKeyedTexture);
    ReleaseVideoReservation();

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
        // These containers are decoded by the bundled FFmpeg libraries. WebP
        // is included here deliberately so both static and animated WebP use
        // the same decoder path instead of the legacy D3DX image loader.
        L"webm", L"mkv", L"flv", L"vob", L"ogv", L"mng", L"avi",
        L"mts", L"m2ts", L"ts", L"mov", L"qt", L"wmv", L"yuv", L"rm",
        L"rmvb", L"viv", L"asf", L"amv", L"mp4", L"m4p", L"mp2", L"mpe",
        L"mpv", L"mpg", L"mpeg", L"m2v", L"m4v", L"svi", L"3gp", L"3g2",
        L"mxf", L"roq", L"nsv", L"f4v", L"f4p", L"f4a", L"f4b", L"webp"
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

MediaTexture* MediaTexture::Create(LPDIRECT3DDEVICE9EX device, const wchar_t* path, unsigned int colorKey,
    bool allowSpout, bool isVideoSprite, bool* outOfMemory)
{
    if (outOfMemory)
        *outOfMemory = false;
    if (!device || !path || !path[0] || device->TestCooperativeLevel() != D3D_OK)
        return nullptr;

    try
    {
        std::unique_ptr<MediaTexture> media(new MediaTexture(device));
        bool initialized = false;
        if (allowSpout && IsSpoutPath(path))
            initialized = media->InitSpout(path, colorKey);
        else if (IsGifFile(path))
            initialized = media->InitGif(path, colorKey);
        else if (IsVideoFile(path))
            initialized = media->InitVideo(path, colorKey, isVideoSprite, outOfMemory);

        return initialized ? media.release() : nullptr;
    }
    catch (const std::bad_alloc&)
    {
        if (outOfMemory)
            *outOfMemory = true;
        return nullptr;
    }
}

bool MediaTexture::CreateDynamicTexture(int width, int height, bool* outOfMemory)
{
    if (!m_device || width <= 0 || height <= 0)
        return false;

    if (m_texture && m_width == width && m_height == height)
        return true;

    if (m_device->TestCooperativeLevel() != D3D_OK)
        return false;

    ReleaseTexture();

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
    {
        if (outOfMemory)
            *outOfMemory = hr == E_OUTOFMEMORY || hr == D3DERR_OUTOFVIDEOMEMORY;
        return false;
    }

    m_width = width;
    m_height = height;
    return true;
}

void MediaTexture::ReleaseTexture()
{
    UnbindTextureFromDevice(m_device, m_texture);
    SafeReleaseLocal(m_texture);
}

void MediaTexture::ReleaseVideoReservation()
{
    if (m_videoReservedBytes == 0)
        return;

    std::lock_guard<std::mutex> lock(g_videoAdmissionMutex);
    g_videoReservedBytes -= m_videoReservedBytes;
    g_videoExtraThreads -= m_videoExtraThreads;
    m_videoReservedBytes = 0;
    m_videoExtraThreads = 0;
}

void MediaTexture::ReleaseVideo()
{
    {
        std::lock_guard<std::mutex> lock(m_videoMutex);
        m_videoStopWorker = true;
    }
    m_videoCondition.notify_one();
    if (m_videoThread.joinable())
        m_videoThread.join();

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

    m_videoQueue.clear();
    std::vector<unsigned char>().swap(m_videoFreeBuffer);
    std::vector<unsigned char>().swap(m_videoDisplayedPixels);
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

void MediaTexture::VideoWorkerMain()
{
    try
    {
        for (;;)
        {
            VideoFrame frame;
            {
                std::unique_lock<std::mutex> lock(m_videoMutex);
                m_videoCondition.wait(lock, [this]()
                {
                    return m_videoStopWorker || m_videoQueue.size() < m_videoQueueCapacity;
                });

                if (m_videoStopWorker)
                    return;
                frame.pixels.swap(m_videoFreeBuffer);
            }

            if (!DecodeNextVideoFrame(frame))
                break;

            std::lock_guard<std::mutex> lock(m_videoMutex);
            if (m_videoStopWorker)
                return;
            m_videoQueue.push_back(std::move(frame));
        }
    }
    catch (const std::exception&)
    {
        // Allocation failures must stay on the worker side of the thread boundary.
    }

    std::lock_guard<std::mutex> lock(m_videoMutex);
    m_videoWorkerFailed = true;
}

bool MediaTexture::UpdateVideo(bool* shouldKill, bool applyColorKey)
{
    using Clock = std::chrono::steady_clock;
    const auto now = Clock::now();
    const auto renderGap = now - m_videoLastUpdateClock;
    m_videoLastUpdateClock = now;

    bool& textureCurrent = applyColorKey ? m_videoKeyedTextureCurrent : m_videoRawTextureCurrent;
    if (!m_videoHavePlaybackStart)
    {
        bool buffered;
        {
            std::lock_guard<std::mutex> lock(m_videoMutex);
            const size_t startupFrames = (std::min)(kVideoStartupFrames, m_videoQueueCapacity);
            buffered = m_videoWorkerFailed ? !m_videoQueue.empty() : m_videoQueue.size() >= startupFrames;
            if (m_videoWorkerFailed && !buffered && shouldKill)
                *shouldKill = true;
        }

        // Both variants must show the already decoded first frame while buffering.
        if (!textureCurrent)
            textureCurrent = UploadVideoPixels(m_videoDisplayedPixels.data(), applyColorKey);
        if (buffered && textureCurrent)
        {
            m_videoPlaybackStartClock = Clock::now();
            m_videoHavePlaybackStart = true;
        }
        return textureCurrent;
    }

    const double elapsed = std::chrono::duration<double>(now - m_videoPlaybackStartClock).count();
    double position = elapsed;
    if (renderGap > kVideoRenderGap)
        position = (std::min)(position, m_videoDisplayedEndSeconds);

    VideoFrame frame;
    {
        std::lock_guard<std::mutex> lock(m_videoMutex);
        while (!m_videoQueue.empty() &&
            m_videoQueue.front().presentationSeconds <= position + kVideoTimestampToleranceSeconds)
        {
            if (m_videoFreeBuffer.empty())
                m_videoFreeBuffer.swap(frame.pixels);

            frame = std::move(m_videoQueue.front());
            m_videoQueue.pop_front();
        }

        if (m_videoQueue.empty())
        {
            if (frame.pixels.empty() && m_videoWorkerFailed && shouldKill)
                *shouldKill = true;

            // Leave the clock alone on brief underruns. Rebasing on each
            // missed frame turns small decode delays into sustained slow motion.
            const double availableEnd = frame.pixels.empty() ? m_videoDisplayedEndSeconds :
                frame.presentationSeconds + frame.durationSeconds;
            if (position > availableEnd + kVideoStallSeconds)
                position = availableEnd;
        }
    }

    if (!frame.pixels.empty())
    {
        if (UploadVideoPixels(frame.pixels.data(), applyColorKey))
        {
            m_videoDisplayedEndSeconds = frame.presentationSeconds + frame.durationSeconds;
            m_videoRawTextureCurrent = !applyColorKey;
            m_videoKeyedTextureCurrent = applyColorKey;
            m_videoDisplayedPixels.swap(frame.pixels);
        }

        {
            std::lock_guard<std::mutex> lock(m_videoMutex);
            if (m_videoFreeBuffer.empty())
                m_videoFreeBuffer.swap(frame.pixels);
        }
        m_videoCondition.notify_one();
    }

    if (position != elapsed)
        m_videoPlaybackStartClock = now - std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(position));
    if (!textureCurrent)
        textureCurrent = UploadVideoPixels(m_videoDisplayedPixels.data(), applyColorKey);
    return textureCurrent;
}

bool MediaTexture::UploadVideoPixels(const unsigned char* pixels, bool applyColorKey)
{
    if (applyColorKey && !m_videoKeyedTexture &&
        FAILED(m_device->CreateTexture(m_width, m_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8,
            D3DPOOL_DEFAULT, &m_videoKeyedTexture, nullptr)))
        return false;

    return UploadPixels(GetTexture(applyColorKey), pixels, m_videoColorKey, applyColorKey);
}

bool MediaTexture::UploadPixels(LPDIRECT3DTEXTURE9 texture, const unsigned char* pixels,
    unsigned int colorKey, bool applyColorKey)
{
    if (!texture || !pixels)
        return false;

    D3DLOCKED_RECT lr;
    HRESULT hr = texture->LockRect(0, &lr, nullptr, D3DLOCK_DISCARD);
    if (FAILED(hr))
        return false;

    if (!applyColorKey)
    {
        const int rowBytes = m_width * 4;
        if (lr.Pitch == rowBytes)
            std::memcpy(lr.pBits, pixels, static_cast<size_t>(rowBytes) * m_height);
        else
        {
            for (int y = 0; y < m_height; ++y)
                std::memcpy(static_cast<unsigned char*>(lr.pBits) + y * lr.Pitch, pixels + y * rowBytes, rowBytes);
        }
        return SUCCEEDED(texture->UnlockRect(0));
    }

    // BGRA bytes form a 0xAARRGGBB word on Windows.
    const unsigned int keyMasked = colorKey & 0x00FFFFFFu;

    for (int y = 0; y < m_height; y++)
    {
        const unsigned char* srcRow = pixels + (size_t)y * m_width * 4;
        unsigned char* dstRow = (unsigned char*)lr.pBits + (size_t)y * lr.Pitch;

        for (int x = 0; x < m_width; x++)
        {
            unsigned int pixel;
            std::memcpy(&pixel, srcRow + (size_t)x * 4, 4);

            if ((pixel & 0x00FFFFFFu) == keyMasked)
                pixel &= 0x00FFFFFFu; // alpha = 0, color unchanged

            std::memcpy(dstRow + (size_t)x * 4, &pixel, 4);
        }
    }

    return SUCCEEDED(texture->UnlockRect(0));
}

bool MediaTexture::InitGif(const wchar_t* path, unsigned int colorKey)
{
    // BeatDrop.exe enables the Windows Runtime song-title poller, which runs
    // on this same render thread and uses an MTA. Requesting an STA here per
    // GIF therefore returns RPC_E_CHANGED_MODE. WIC is MTA-compatible, so
    // match the render thread instead and only balance a COM reference that
    // this MediaTexture actually acquired.
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
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

            try
            {
                GifFrame outFrame;
                outFrame.pixels = canvas;
                outFrame.delaySeconds = delay / 100.0;
                m_gifFrames.push_back(outFrame);
                m_gifTotalSeconds += outFrame.delaySeconds;
            }
            catch (...)
            {
                SafeReleaseLocal(frameMeta);
                SafeReleaseLocal(converter);
                SafeReleaseLocal(frame);
                continue;
            }
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

    return CreateDynamicTexture(m_width, m_height) &&
        UploadPixels(m_texture, m_gifFrames[0].pixels.data(), colorKey, true);
}

bool MediaTexture::InitVideo(const wchar_t* path, unsigned int colorKey, bool isVideoSprite, bool* outOfMemory)
{
    std::string utf8Path = WideToUtf8(path);
    if (utf8Path.empty())
        return false;

    m_formatContext = avformat_alloc_context();
    if (!m_formatContext)
        return false;
    m_formatContext->interrupt_callback.callback = [](void* context) -> int
    {
        return static_cast<MediaTexture*>(context)->m_videoStopWorker.load() ? 1 : 0;
    };
    m_formatContext->interrupt_callback.opaque = this;

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

    const int sourceWidth = m_codecContext->width;
    const int sourceHeight = m_codecContext->height;
    if (!IsReasonableMediaSize(sourceWidth, sourceHeight))
        return false;

    double textureScale = 1.0;
    if (isVideoSprite)
    {
        D3DCAPS9 caps{};
        if (SUCCEEDED(m_device->GetDeviceCaps(&caps)))
        {
            textureScale = (std::min)({1.0,
                double(caps.MaxTextureWidth) / sourceWidth,
                double(caps.MaxTextureHeight) / sourceHeight});
        }
    }

    const int sourceBytes = GetVideoFrameBytes(m_codecContext->pix_fmt, sourceWidth, sourceHeight);
    m_videoMaxSourceFrameBytes = sourceBytes > 0 ? sourceBytes :
        GetVideoFrameBytes(AV_PIX_FMT_BGRA, sourceWidth, sourceHeight);

    unsigned int preferredThreads = 1;
    if (codec->capabilities & AV_CODEC_CAP_FRAME_THREADS)
    {
        const int sourcePixels = sourceWidth * sourceHeight;
        if (sourcePixels >= 3840 * 2160)
            preferredThreads = 4;
        else if (sourcePixels >= 1280 * 720)
            preferredThreads = 2;
    }
    // Include the displayed CPU frame and the optional keyed GPU texture.
    const size_t extraConvertedFrames = isVideoSprite ? 6 : 5;
    int threads;
    for (;;)
    {
        const int textureWidth = (std::max)(1, static_cast<int>(std::lround(sourceWidth * textureScale)));
        const int textureHeight = (std::max)(1, static_cast<int>(std::lround(sourceHeight * textureScale)));
        const uint64_t convertedFrameBytes = uint64_t(textureWidth) * textureHeight * 4;
        threads = ReserveVideoDecoder(m_videoMaxSourceFrameBytes, convertedFrameBytes,
            extraConvertedFrames, preferredThreads, m_videoReservedBytes, m_videoQueueCapacity);
        if (threads > 0)
        {
            m_videoExtraThreads = threads - 1;
            bool textureOutOfMemory = false;
            if (CreateDynamicTexture(textureWidth, textureHeight, &textureOutOfMemory))
                break;
            ReleaseVideoReservation();
            if (!textureOutOfMemory)
                return false;
        }

        if (!isVideoSprite || (textureWidth == 1 && textureHeight == 1))
        {
            if (outOfMemory)
                *outOfMemory = true;
            return false;
        }

        // The source still decodes at its encoded size. A smaller upload
        // texture reduces the cost of the converted frames under pressure.
        textureScale *= 0.5;
    }

    m_codecContext->thread_count = threads;
    m_codecContext->thread_type = FF_THREAD_FRAME;
    m_codecContext->max_pixels = int64_t((sourceWidth + 63) & ~63) * ((sourceHeight + 63) & ~63);
    const int openResult = avcodec_open2(m_codecContext, codec, nullptr);
    if (openResult < 0)
    {
        if (outOfMemory)
            *outOfMemory = openResult == AVERROR(ENOMEM);
        return false;
    }

    m_videoFrame = av_frame_alloc();
    m_videoPacket = av_packet_alloc();
    if (!m_videoFrame || !m_videoPacket)
        return false;

    m_videoStreamIndex = streamIndex;
    m_videoTimeBaseSeconds = av_q2d(stream->time_base);
    if (!std::isfinite(m_videoTimeBaseSeconds) || m_videoTimeBaseSeconds <= 0.0)
        m_videoTimeBaseSeconds = 0.0;

    // This is only a fallback for files without usable presentation
    // timestamps. It must not be used as the video clock: avg_frame_rate and
    // r_frame_rate describe a nominal cadence and are wrong for VFR content.
    AVRational fps = av_guess_frame_rate(m_formatContext, stream, nullptr);
    if (fps.num <= 0 || fps.den <= 0)
        fps = stream->avg_frame_rate;
    if (fps.num <= 0 || fps.den <= 0)
        fps = stream->r_frame_rate;
    if (fps.num > 0 && fps.den > 0)
    {
        double seconds = (double)fps.den / (double)fps.num;
        if (std::isfinite(seconds) && seconds > 0.0 && seconds <= 10.0)
            m_videoFrameSeconds = seconds;
    }

    m_kind = KIND_VIDEO;
    m_videoColorKey = colorKey;
    if (m_videoTimeBaseSeconds > 0.0)
    {
        if (stream->start_time != AV_NOPTS_VALUE)
            m_videoSeekStart = stream->start_time;
        else if (m_formatContext->start_time != AV_NOPTS_VALUE)
            m_videoSeekStart = av_rescale_q(m_formatContext->start_time, AV_TIME_BASE_Q, stream->time_base);

        if (m_formatContext->pb && (m_formatContext->pb->seekable & AVIO_SEEKABLE_NORMAL))
        {
            int64_t duration = stream->duration;
            if (duration <= 0 && m_formatContext->duration > 0)
                duration = av_rescale_q(m_formatContext->duration, AV_TIME_BASE_Q, stream->time_base);
            if (duration > 0 &&
                av_compare_ts(duration, stream->time_base, kRandomStartThresholdSeconds, AVRational{1, 1}) > 0 &&
                m_videoSeekStart <= INT64_MAX - duration)
            {
                m_videoSeekRange = duration - static_cast<int64_t>(kRandomStartEndMarginSeconds / m_videoTimeBaseSeconds);
            }
        }
    }

    // Expose a real frame before either a sprite or a shader can sample the texture.
    SeekToRandomVideoStart();
    VideoFrame firstFrame;
    if (!DecodeNextVideoFrame(firstFrame))
    {
        if (m_videoPrerollEnd == AV_NOPTS_VALUE || !SeekVideo(m_videoSeekStart) || !DecodeNextVideoFrame(firstFrame))
            return false;
    }
    if (!UploadVideoPixels(firstFrame.pixels.data(), false))
        return false;
    m_videoDisplayedPixels.swap(firstFrame.pixels);
    m_videoDisplayedEndSeconds = firstFrame.presentationSeconds + firstFrame.durationSeconds;
    m_videoRawTextureCurrent = true;

    try
    {
        m_videoThread = std::thread(&MediaTexture::VideoWorkerMain, this);
    }
    catch (const std::system_error&)
    {
        return false;
    }
    return true;
}

bool MediaTexture::SeekVideo(int64_t timestamp)
{
    int result = av_seek_frame(m_formatContext, m_videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
    if (result < 0 && timestamp == m_videoSeekStart)
    {
        const int64_t start = m_formatContext->start_time == AV_NOPTS_VALUE ? 0 : m_formatContext->start_time;
        result = av_seek_frame(m_formatContext, -1, start, AVSEEK_FLAG_BACKWARD);
    }
    if (result < 0)
        return false;

    avcodec_flush_buffers(m_codecContext);
    m_videoReachedEnd = false;
    m_videoHaveTimestampOrigin = false;
    m_videoPrerollEnd = AV_NOPTS_VALUE;
    return true;
}

void MediaTexture::SeekToRandomVideoStart()
{
    if (m_videoSeekRange <= 0)
        return;

    const auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937_64 random(seed);
    std::uniform_int_distribution<int64_t> offset(0, m_videoSeekRange);
    const int64_t target = m_videoSeekStart + offset(random);
    if (SeekVideo(target))
        m_videoPrerollEnd = target;
    else
        SeekVideo(m_videoSeekStart);
}

bool MediaTexture::DecodeNextVideoFrame(VideoFrame& frame)
{
    // Bound retries for damaged files and empty streams, including EOF/seek loops.
    for (int attempt = 0; attempt < 4096 && !m_videoStopWorker; ++attempt)
    {
        int result = avcodec_receive_frame(m_codecContext, m_videoFrame);
        if (result == 0)
        {
            const int64_t timestamp = m_videoFrame->best_effort_timestamp;
            if (m_videoPrerollEnd != AV_NOPTS_VALUE && timestamp != AV_NOPTS_VALUE && timestamp < m_videoPrerollEnd)
            {
                av_frame_unref(m_videoFrame);
                continue;
            }
            m_videoPrerollEnd = AV_NOPTS_VALUE;

            const int width = m_videoFrame->width;
            const int height = m_videoFrame->height;
            const auto format = static_cast<AVPixelFormat>(m_videoFrame->format);
            const int bytes = IsReasonableMediaSize(width, height) ? GetVideoFrameBytes(format, width, height) : 0;
            if (bytes <= 0 || bytes > m_videoMaxSourceFrameBytes)
                return false;

            m_swsContext = sws_getCachedContext(m_swsContext, width, height, format,
                m_width, m_height, AV_PIX_FMT_BGRA, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
            if (!m_swsContext)
                return false;

            const int stride = m_width * 4;
            frame.pixels.resize(static_cast<size_t>(stride) * m_height + AV_INPUT_BUFFER_PADDING_SIZE);
            uint8_t* destination[] = {frame.pixels.data(), nullptr, nullptr, nullptr};
            const int strides[] = {stride, 0, 0, 0};
            const int rows = sws_scale(m_swsContext, m_videoFrame->data, m_videoFrame->linesize,
                0, height, destination, strides);
            const double duration = m_videoFrame->duration * m_videoTimeBaseSeconds;
            av_frame_unref(m_videoFrame);
            if (rows != m_height)
                return false;

            const double next = m_videoLastPresentationSeconds + m_videoLastFrameDurationSeconds;
            frame.presentationSeconds = next;
            frame.durationSeconds = duration > 0.0 && std::isfinite(duration) ? duration : m_videoFrameSeconds;
            if (timestamp != AV_NOPTS_VALUE && m_videoTimeBaseSeconds > 0.0)
            {
                const double sourceSeconds = timestamp * m_videoTimeBaseSeconds;
                if (!m_videoHaveTimestampOrigin)
                {
                    m_videoTimestampOffsetSeconds = next - sourceSeconds;
                    m_videoHaveTimestampOrigin = true;
                }
                const double presentation = sourceSeconds + m_videoTimestampOffsetSeconds;
                if (presentation > m_videoLastPresentationSeconds)
                    frame.presentationSeconds = presentation;
            }
            m_videoLastPresentationSeconds = frame.presentationSeconds;
            m_videoLastFrameDurationSeconds = frame.durationSeconds;
            return true;
        }

        if (result == AVERROR_EOF || (result == AVERROR(EAGAIN) && m_videoReachedEnd))
        {
            if (!SeekVideo(m_videoSeekStart))
                return false;
            continue;
        }
        if (result != AVERROR(EAGAIN))
            return false;

        result = av_read_frame(m_formatContext, m_videoPacket);
        if (result == AVERROR_EOF)
        {
            result = avcodec_send_packet(m_codecContext, nullptr);
            if (result < 0 && result != AVERROR_EOF)
                return false;
            m_videoReachedEnd = true;
            continue;
        }
        if (result < 0)
            return false;

        if (m_videoPacket->stream_index == m_videoStreamIndex)
            result = avcodec_send_packet(m_codecContext, m_videoPacket);
        av_packet_unref(m_videoPacket);
        if (result < 0 && result != AVERROR_INVALIDDATA)
            return false;
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

    (void)now;

    switch (m_kind)
    {
    case KIND_GIF:
    {
        if (m_gifFrames.empty())
            return m_texture != nullptr;

        if (!m_gifHavePlaybackStart)
        {
            m_gifPlaybackStartClock = std::chrono::steady_clock::now();
            m_gifHavePlaybackStart = true;
        }
        double elapsedSeconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - m_gifPlaybackStartClock).count();
        if (elapsedSeconds < 0.0)
            elapsedSeconds = 0.0;

        double t = fmod(elapsedSeconds, m_gifTotalSeconds);
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
            return UploadPixels(m_texture, m_gifFrames[frameIndex].pixels.data(), m_gifColorKey, m_gifApplyColorKey);
        }
        return true;
    }
    case KIND_VIDEO:
        return UpdateVideo(shouldKill, wantColorKeyThisFrame);
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
