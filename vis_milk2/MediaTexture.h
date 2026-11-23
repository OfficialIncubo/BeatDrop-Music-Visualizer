/*
  Animated and live texture support for BeatDrop sprites and shader samplers.
*/

#ifndef BEATDROP_MEDIA_TEXTURE_H
#define BEATDROP_MEDIA_TEXTURE_H 1

#include <d3d9.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
class spoutDX9;

class MediaTexture
{
public:
    enum Kind
    {
        KIND_NONE = 0,
        KIND_GIF,
        KIND_VIDEO,
        KIND_SPOUT
    };

    static bool IsGifFile(const wchar_t* path);
    static bool IsVideoFile(const wchar_t* path);
    static bool IsSupportedFile(const wchar_t* path);
    static bool IsSpoutPath(const wchar_t* path);
    static MediaTexture* Create(LPDIRECT3DDEVICE9EX device, const wchar_t* path, unsigned int colorKey,
        bool allowSpout, bool isVideoSprite = false, bool* outOfMemory = nullptr);

    ~MediaTexture();

    bool Update(float now, bool* shouldKill, bool wantColorKeyThisFrame = false);
    LPDIRECT3DTEXTURE9 GetTexture(bool applyColorKey = false) const
    {
        return applyColorKey && m_kind == KIND_VIDEO ? m_videoKeyedTexture : m_texture;
    }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    Kind GetKind() const { return m_kind; }
    bool IsSpout() const { return m_kind == KIND_SPOUT; }
    bool CanShareVideo(unsigned int colorKey) const
    {
        return m_kind == KIND_VIDEO && (m_videoColorKey & 0xFFFFFFu) == (colorKey & 0xFFFFFFu);
    }
    const char* GetSpoutSenderName() const { return m_spoutSenderName.c_str(); }

    // Returns true and fills in *outColorKey if this texture has a colorkey to
    // apply for blendmode 4. GIF/Video already bake their colorkey directly into
    // the uploaded texture's alpha channel (see m_gifApplyColorKey/UploadPixels),
    // which the existing fixed-function blendmode-4 path already honors, so this
    // is currently only meaningful (and only returns true) for KIND_SPOUT: a live
    // Spout texture is a GPU resource with no per-frame CPU pixel access, so its
    // colorkey has to be applied in a pixel shader at draw time instead.
    bool GetColorKey(unsigned int* outColorKey) const;

private:
    struct GifFrame
    {
        std::vector<unsigned char> pixels;
        double delaySeconds;
    };

    explicit MediaTexture(LPDIRECT3DDEVICE9EX device);

    bool InitGif(const wchar_t* path, unsigned int colorKey);
    bool InitVideo(const wchar_t* path, unsigned int colorKey, bool isVideoSprite, bool* outOfMemory);
    bool InitSpout(const wchar_t* path, unsigned int colorKey);

    bool CreateDynamicTexture(int width, int height, bool* outOfMemory = nullptr);
    bool UploadPixels(LPDIRECT3DTEXTURE9 texture, const unsigned char* pixels, unsigned int colorKey, bool applyColorKey);
    bool UploadVideoPixels(const unsigned char* pixels, bool applyColorKey);
    struct VideoFrame;
    bool DecodeNextVideoFrame(VideoFrame& frame);
    bool SeekVideo(int64_t timestamp);
    void SeekToRandomVideoStart();
    void VideoWorkerMain();
    bool UpdateVideo(bool* shouldKill, bool applyColorKey);
    void ReleaseTexture();
    void ReleaseVideo();
    void ReleaseVideoReservation();
    void ReleaseSpout(bool waitForSpout);

    LPDIRECT3DDEVICE9EX m_device;
    LPDIRECT3DTEXTURE9 m_texture;
    Kind m_kind;
    int m_width;
    int m_height;
    bool m_comInitialized;

    std::vector<GifFrame> m_gifFrames;
    double m_gifTotalSeconds;
    int m_gifFrameIndex;
    unsigned int m_gifColorKey;
    bool m_gifApplyColorKey;
    std::chrono::steady_clock::time_point m_gifPlaybackStartClock;
    bool m_gifHavePlaybackStart;

    struct VideoFrame
    {
        std::vector<unsigned char> pixels;
        double presentationSeconds = 0.0;
        double durationSeconds = 0.0;
    };

    AVFormatContext* m_formatContext;
    AVCodecContext* m_codecContext;
    AVFrame* m_videoFrame;
    AVPacket* m_videoPacket;
    SwsContext* m_swsContext;
    int m_videoStreamIndex;
    bool m_videoReachedEnd = false;
    size_t m_videoReservedBytes = 0;
    size_t m_videoQueueCapacity = 0;
    unsigned int m_videoExtraThreads = 0;
    int m_videoMaxSourceFrameBytes = 0;
    int64_t m_videoSeekStart = 0;
    int64_t m_videoSeekRange = 0;
    int64_t m_videoPrerollEnd = INT64_MIN;
    std::thread m_videoThread;
    std::mutex m_videoMutex;
    std::condition_variable m_videoCondition;
    std::atomic<bool> m_videoStopWorker{false};
    bool m_videoWorkerFailed = false;
    std::deque<VideoFrame> m_videoQueue;
    std::vector<unsigned char> m_videoFreeBuffer;
    std::vector<unsigned char> m_videoDisplayedPixels;
    LPDIRECT3DTEXTURE9 m_videoKeyedTexture = nullptr;
    bool m_videoRawTextureCurrent = false;
    bool m_videoKeyedTextureCurrent = false;
    std::chrono::steady_clock::time_point m_videoPlaybackStartClock;
    std::chrono::steady_clock::time_point m_videoLastUpdateClock;
    bool m_videoHavePlaybackStart = false;
    double m_videoDisplayedEndSeconds = 0.0;
    double m_videoTimeBaseSeconds;
    double m_videoTimestampOffsetSeconds = 0.0;
    bool m_videoHaveTimestampOrigin = false;
    double m_videoLastPresentationSeconds = 0.0;
    double m_videoLastFrameDurationSeconds = 0.0;
    double m_videoFrameSeconds;
    unsigned int m_videoColorKey;

    spoutDX9* m_spoutReceiver;
    bool m_spoutWasConnected;
    std::string m_spoutSenderName;
    unsigned int m_spoutColorKey;
    bool m_spoutHasColorKey;
};

#endif
