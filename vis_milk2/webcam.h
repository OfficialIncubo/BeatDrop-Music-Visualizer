#ifndef BEATDROP_WEBCAM_H
#define BEATDROP_WEBCAM_H

#include <d3d9.h>
#include <memory>
#include <string>
#include <vector>

// Capture belongs to the plugin, independently of its replaceable D3D device.
class BeatDropWebcam
{
public:
    enum Mode { Off, Deep, DeepBurn, Top };
    static constexpr float DefaultBurnOpacity = 0.05f;
    static constexpr float DefaultDeepOpacity = 1.0f;
    static constexpr float DefaultTopOpacity = 0.5f;

    BeatDropWebcam();
    ~BeatDropWebcam();
    BeatDropWebcam(const BeatDropWebcam&) = delete;
    BeatDropWebcam& operator=(const BeatDropWebcam&) = delete;

    const wchar_t* Cycle(IDirect3DDevice9* device = nullptr);
    const wchar_t* NextCamera();
    struct Device { std::wstring id, name; };
    static HRESULT ListDevices(std::vector<Device>& devices);
    const wchar_t* SelectCamera(const std::wstring& id, bool notify = true);
    const wchar_t* SetMode(Mode mode, IDirect3DDevice9* device = nullptr);
    const std::wstring& SelectedDevice() const { return m_selectedDevice; }
    unsigned long long CameraSelection() const { return m_cameraSelection; }

    void Stop();
    // Called on the render thread. Reports a capture failure only once.
    void Update(IDirect3DDevice9* device, std::wstring& error, std::wstring* notice = nullptr);
    void ReleaseDeviceResources();
    Mode GetMode() const { return m_mode; }
    void Draw(IDirect3DDevice9* device, int width, int height, bool behind, float opacity = DefaultDeepOpacity);
    void DrawBurn(IDirect3DDevice9* device, int width, int height, float opacity, double elapsedSeconds = 0.0);
    // Separate camera-only warp targets keep Deep out of persistent feedback.
    bool PrepareDeepInput(IDirect3DDevice9* device, IDirect3DTexture9* reference, float opacity = DefaultDeepOpacity);
    IDirect3DTexture9* GetDeepInput() const { return m_deepInput; }
    IDirect3DTexture9* GetDeepOutput() const { return m_deepOutput; }
    IDirect3DTexture9* GetDeepBaseline() const { return m_deepBaseline; }
    void BlendDeepScene(IDirect3DDevice9* device, IDirect3DTexture9* scene);

private:
    struct Capture;
    bool StartCapture(IDirect3DDevice9* device = nullptr, bool selectionOnly = false, bool notify = true);
    void ReleaseDeepResources();
    static bool DrawTexture(IDirect3DDevice9* device, IDirect3DTexture9* texture,
        int width, int height, bool behind, bool crop, float opacity = 1.0f, bool subtract = false);
    std::unique_ptr<Capture> m_capture;
    Mode m_mode = Off;
    IDirect3DTexture9* m_texture = nullptr;
    IDirect3DTexture9* m_deepInput = nullptr;
    IDirect3DTexture9* m_deepOutput = nullptr;
    IDirect3DTexture9* m_deepBaseline = nullptr;
    std::wstring m_selectedDevice;
    unsigned long long m_cameraSelection = 0;
    unsigned long long m_selectionRevision = 0;
    unsigned long long m_uploadedFrame = 0;
    double m_captureIntervalSeconds = 0.0;
    int m_width = 0;
    int m_height = 0;
    std::vector<unsigned char> m_pixels;
};

#endif
