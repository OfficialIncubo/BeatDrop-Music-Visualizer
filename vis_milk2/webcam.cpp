#include "webcam.h"

#include <dshow.h>
#include <wrl/client.h>
#include <algorithm>
#include <atomic>
#include <climits>
#include <cmath>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <cwchar>
#include <mutex>
#include <limits>
#include <thread>
#include <vector>

// DirectShow works on the OldOS builds as well as current Windows. qedit.h is
// no longer shipped with the SDK and its old version conflicts with D3D9, so
// declare only the two documented Sample Grabber interfaces used here.
namespace
{
    using Microsoft::WRL::ComPtr;
    float ClampWebcamOpacity(float opacity, float fallback)
    {
        if (!std::isfinite(opacity)) return fallback;
        return (std::max)(0.0f, (std::min)(1.0f, opacity));
    }

    // Check the actual container and DirectShow sample-length constraints;
    // do not impose an application-specific camera resolution or memory cap.
    bool WebcamFrameSize(LONG width, LONG signedHeight, size_t& bytes)
    {
        if (width <= 0 || signedHeight == 0 || signedHeight == (std::numeric_limits<LONG>::min)())
            return false;
        const size_t height = static_cast<size_t>(std::abs(signedHeight));
        const size_t capacity = (std::min)(std::vector<BYTE>().max_size(),
            static_cast<size_t>((std::numeric_limits<long>::max)()));
        if (static_cast<size_t>(width) > capacity / sizeof(DWORD)) return false;
        const size_t stride = static_cast<size_t>(width) * sizeof(DWORD);
        if (height > capacity / stride) return false;
        bytes = stride * height;
        return true;
    }

    struct WebcamControlEvent
    {
        HANDLE handle = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        ~WebcamControlEvent() { if (handle) CloseHandle(handle); }
        void Signal() const { SetEvent(handle); }
    };

    MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
    WebcamSampleCallback : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE SampleCB(double, IMediaSample*) = 0;
        virtual HRESULT STDMETHODCALLTYPE BufferCB(double, BYTE*, long) = 0;
    };

    MIDL_INTERFACE("6B652FFF-11FE-4FCE-92AD-0266B5D7C78F")
    WebcamSampleGrabber : public IUnknown
    {
        virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE*) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long*, long*) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample**) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetCallback(WebcamSampleCallback*, long) = 0;
    };

    const CLSID kSampleGrabber =
        { 0xC1F400A0, 0x3F08, 0x11D3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
    const CLSID kNullRenderer =
        { 0xC1F400A4, 0x3F08, 0x11D3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

    void FreeMediaType(AM_MEDIA_TYPE& type)
    {
        CoTaskMemFree(type.pbFormat);
        if (type.pUnk)
            type.pUnk->Release();
        type = {};
    }

    struct ScopedWebcamMediaType : AM_MEDIA_TYPE
    {
        ScopedWebcamMediaType() : AM_MEDIA_TYPE{} {}
        ~ScopedWebcamMediaType() { FreeMediaType(*this); }
        ScopedWebcamMediaType(const ScopedWebcamMediaType&) = delete;
        ScopedWebcamMediaType& operator=(const ScopedWebcamMediaType&) = delete;
    };

    struct ScopedWebcamVariant : VARIANT
    {
        ScopedWebcamVariant() { VariantInit(this); }
        ~ScopedWebcamVariant() { VariantClear(this); }
    };

    struct CameraEntry
    {
        ComPtr<IMoniker> moniker;
        BeatDropWebcam::Device device;
    };

    // Shared by capture, keyboard selection and the menu. No camera is opened.
    HRESULT EnumerateCameras(std::vector<CameraEntry>& available)
    {
        ComPtr<ICreateDevEnum> devices;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(devices.GetAddressOf()));
        if (FAILED(hr)) return hr;
        ComPtr<IEnumMoniker> cameras;
        hr = devices->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, cameras.GetAddressOf(), 0);
        if (hr != S_OK) return hr;
        ComPtr<IMoniker> entry;
        while ((hr = cameras->Next(1, entry.ReleaseAndGetAddressOf(), nullptr)) == S_OK)
        {
            LPOLESTR rawId = nullptr;
            const HRESULT named = entry->GetDisplayName(nullptr, nullptr, &rawId);
            std::unique_ptr<OLECHAR, decltype(&CoTaskMemFree)> id(rawId, CoTaskMemFree);
            if (FAILED(named)) return named;
            CameraEntry camera;
            camera.moniker = entry;
            camera.device.id = id.get();
            camera.device.name = L"Camera " + std::to_wstring(available.size() + 1);
            ComPtr<IPropertyBag> properties;
            if (SUCCEEDED(entry->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(properties.GetAddressOf()))))
            {
                ScopedWebcamVariant name;
                if (SUCCEEDED(properties->Read(L"FriendlyName", &name, nullptr)) &&
                    name.vt == VT_BSTR && name.bstrVal)
                    camera.device.name = name.bstrVal;
            }
            available.push_back(std::move(camera));
        }
        return FAILED(hr) ? hr : S_OK;
    }

    struct WebcamStreamingCleanup
    {
        IMediaControl* control;
        WebcamSampleGrabber* grabber;
        ~WebcamStreamingCleanup()
        {
            // Also run during exception unwinding, before the callback and
            // capture buffers can be destroyed.
            control->Stop();
            grabber->SetCallback(nullptr, 1);
        }
    };

    struct WebcamVertex
    {
        float x, y, z, rhw;
        DWORD color;
        float u, v;
    };
}

struct BeatDropWebcam::Capture
{
    std::thread worker;
    std::atomic<bool> stop{false};
    std::atomic<HRESULT> failure{S_OK};
    std::atomic<unsigned long long> requestedCamera{0};
    std::atomic<bool> notifyCameraChange{false};
    std::atomic<bool> finished{false};
    unsigned long long requestedIndex = 0; // protected by mutex
    std::wstring requestedDevice; // protected by mutex
    std::atomic<unsigned long long> activeCameraIndex{0};
    bool selectionOnly = false;
    UINT maxTextureWidth = 0;
    UINT maxTextureHeight = 0;
    std::mutex mutex;
    WebcamControlEvent controlEvent;
    std::vector<BYTE> pixels;
    unsigned long long frame = 0;
    int width = 0;
    int height = 0;
    bool bottomUp = false;
    bool haveFrame = false;
    unsigned long long streamFrames = 0;
    double firstSampleSeconds = 0.0;
    double frameIntervalSeconds = 0.0;
    std::chrono::steady_clock::time_point firstSampleClock;
    std::wstring notice;
    unsigned long long noticeSelection = 0;

    void PublishCameraChange(const std::wstring& name, unsigned long long selection)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (requestedCamera.load() == selection && notifyCameraChange.exchange(false))
        {
            notice = L"Changed webcam: " + name;
            noticeSelection = selection;
        }
    }

    class Callback final : public WebcamSampleCallback
    {
    public:
        explicit Callback(Capture& owner) : m_owner(owner) {}

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override
        {
            if (!object)
                return E_POINTER;
            *object = nullptr;
            if (iid != IID_IUnknown && iid != __uuidof(WebcamSampleCallback))
                return E_NOINTERFACE;
            *object = static_cast<WebcamSampleCallback*>(this);
            AddRef();
            return S_OK;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return ++m_references; }
        ULONG STDMETHODCALLTYPE Release() override
        {
            const ULONG remaining = --m_references;
            if (!remaining)
                delete this;
            return remaining;
        }
        HRESULT STDMETHODCALLTYPE SampleCB(double, IMediaSample*) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE BufferCB(double sampleSeconds, BYTE* data, long length) override
        {
            if (m_owner.stop.load())
                return S_OK;
            try
            {
                std::lock_guard<std::mutex> lock(m_owner.mutex);
                size_t size = 0;
                WebcamFrameSize(m_owner.width, m_owner.height, size);
                if (data && length > 0 && size && m_owner.pixels.size() >= size &&
                    static_cast<size_t>(length) >= size)
                {
                    // Retain only the newest frame; a slow renderer never queues
                    // seconds of camera footage or blocks capture on GPU work.
                    std::memcpy(m_owner.pixels.data(), data, size);
                    ++m_owner.frame;
                    m_owner.haveFrame = true;
                    const auto now = std::chrono::steady_clock::now();
                    if (++m_owner.streamFrames == 1)
                    {
                        m_owner.firstSampleSeconds = sampleSeconds;
                        m_owner.firstSampleClock = now;
                    }
                    else
                    {
                        double duration = sampleSeconds - m_owner.firstSampleSeconds;
                        if (!std::isfinite(duration) || duration <= 0.0)
                            duration = std::chrono::duration<double>(now - m_owner.firstSampleClock).count();
                        m_owner.frameIntervalSeconds = duration / (m_owner.streamFrames - 1);
                    }
                }
                return S_OK;
            }
            catch (...)
            {
                m_owner.failure.store(E_FAIL);
                m_owner.controlEvent.Signal();
                return E_FAIL; // Never let C++ exceptions escape into DirectShow.
            }
        }
    private:
        std::atomic<ULONG> m_references{1};
        Capture& m_owner;
    };

    HRESULT Run(unsigned long long selection)
    {
        ComPtr<IGraphBuilder> graph;
        HRESULT hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(graph.GetAddressOf()));
        if (FAILED(hr)) return hr;
        ComPtr<ICaptureGraphBuilder2> builder;
        hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(builder.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = builder->SetFiltergraph(graph.Get());
        if (FAILED(hr)) return hr;

        std::vector<CameraEntry> available;
        hr = EnumerateCameras(available);
        if (FAILED(hr)) return hr;
        if (available.empty()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        std::wstring selectedId;
        unsigned long long selectedIndex;
        {
            std::lock_guard<std::mutex> lock(mutex);
            selectedId = requestedDevice;
            selectedIndex = requestedIndex;
        }
        size_t index = selectedIndex % available.size();
        if (!selectedId.empty())
        {
            const auto found = std::find_if(available.begin(), available.end(),
                [&](const CameraEntry& entry) { return entry.device.id == selectedId; });
            if (found == available.end()) return HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED);
            index = static_cast<size_t>(found - available.begin());
        }
        activeCameraIndex.store(index);
        ComPtr<IMoniker> camera = available[index].moniker;
        const std::wstring& cameraName = available[index].device.name;
        if (stop.load() || requestedCamera.load() != selection) return S_OK;
        if (selectionOnly)
        {
            PublishCameraChange(cameraName, selection);
            return S_OK; // Device enumeration only; do not open the camera.
        }

        ComPtr<IBaseFilter> source;
        hr = camera->BindToObject(nullptr, nullptr, IID_PPV_ARGS(source.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = graph->AddFilter(source.Get(), L"BeatDrop webcam");
        if (FAILED(hr)) return hr;

        ComPtr<IBaseFilter> filter;
        hr = CoCreateInstance(kSampleGrabber, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(filter.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = graph->AddFilter(filter.Get(), L"Webcam frames");
        if (FAILED(hr)) return hr;
        ComPtr<WebcamSampleGrabber> grabber;
        hr = filter.As(&grabber);
        if (FAILED(hr)) return hr;
        AM_MEDIA_TYPE wanted = {};
        wanted.majortype = MEDIATYPE_Video;
        wanted.subtype = MEDIASUBTYPE_RGB32;
        wanted.formattype = FORMAT_VideoInfo;
        hr = grabber->SetMediaType(&wanted);
        if (FAILED(hr)) return hr;
        ComPtr<IBaseFilter> sink;
        hr = CoCreateInstance(kNullRenderer, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(sink.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = graph->AddFilter(sink.Get(), L"Webcam sink");
        if (FAILED(hr)) return hr;
        hr = builder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
            source.Get(), filter.Get(), sink.Get());
        if (FAILED(hr)) return hr;

        ScopedWebcamMediaType connected;
        hr = grabber->GetConnectedMediaType(&connected);
        if (SUCCEEDED(hr))
        {
            if (connected.formattype != FORMAT_VideoInfo ||
                connected.cbFormat < sizeof(VIDEOINFOHEADER) || !connected.pbFormat ||
                connected.subtype != MEDIASUBTYPE_RGB32)
                hr = VFW_E_INVALIDMEDIATYPE;
            else
            {
                const BITMAPINFOHEADER& bitmap =
                    reinterpret_cast<VIDEOINFOHEADER*>(connected.pbFormat)->bmiHeader;
                size_t frameBytes = 0;
                if (!WebcamFrameSize(bitmap.biWidth, bitmap.biHeight, frameBytes) ||
                    bitmap.biBitCount != sizeof(DWORD) * CHAR_BIT || bitmap.biCompression != BI_RGB ||
                    (maxTextureWidth && static_cast<UINT>(bitmap.biWidth) > maxTextureWidth) ||
                    (maxTextureHeight && static_cast<UINT>(std::abs(bitmap.biHeight)) > maxTextureHeight))
                    hr = VFW_E_INVALIDMEDIATYPE;
                else
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    width = bitmap.biWidth;
                    height = std::abs(bitmap.biHeight);
                    bottomUp = bitmap.biHeight > 0;
                    haveFrame = false;
                    streamFrames = 0;
                    frameIntervalSeconds = 0.0;
                    pixels.resize(frameBytes);
                }
            }
        }
        FreeMediaType(connected);
        if (FAILED(hr) || stop.load() || requestedCamera.load() != selection) return hr;

        ComPtr<Callback> callback;
        callback.Attach(new Callback(*this));
        hr = grabber->SetOneShot(FALSE);
        if (SUCCEEDED(hr)) hr = grabber->SetBufferSamples(FALSE);
        if (SUCCEEDED(hr)) hr = grabber->SetCallback(callback.Get(), 1);
        if (FAILED(hr)) return hr;
        ComPtr<IMediaControl> control;
        hr = graph.As(&control);
        if (FAILED(hr))
        {
            grabber->SetCallback(nullptr, 1);
            return hr;
        }
        WebcamStreamingCleanup streamingCleanup{control.Get(), grabber.Get()};
        ComPtr<IMediaEvent> events;
        hr = graph.As(&events);
        if (FAILED(hr)) return hr;
        OAEVENT graphEvent = 0;
        hr = events->GetEventHandle(&graphEvent);
        if (FAILED(hr)) return hr;
        // The graph owns its event handle; it stays valid until Run returns.
        const HANDLE waitHandles[] = {controlEvent.handle, reinterpret_cast<HANDLE>(graphEvent)};
        hr = stop.load() ? S_OK : control->Run();
        if (SUCCEEDED(hr))
        {
            PublishCameraChange(cameraName, selection);
            while (!stop.load() && requestedCamera.load() == selection && SUCCEEDED(failure.load()))
            {
                if (events)
                {
                    long code;
                    LONG_PTR first, second;
                    while (events->GetEvent(&code, &first, &second, 0) == S_OK)
                    {
                        if (code == EC_ERRORABORT || code == EC_DEVICE_LOST)
                            hr = code == EC_ERRORABORT && FAILED(static_cast<HRESULT>(first))
                                ? static_cast<HRESULT>(first) : HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED);
                        events->FreeEventParams(code, first, second);
                    }
                }
                if (FAILED(hr)) break;
                if (stop.load() || requestedCamera.load() != selection || FAILED(failure.load())) break;
                const DWORD result = WaitForMultipleObjects(_countof(waitHandles), waitHandles, FALSE, INFINITE);
                if (result == WAIT_FAILED)
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    break;
                }
            }
        }
        // streamingCleanup stops capture and detaches the callback before
        // the local COM pointers and frame storage are released.
        return hr;
    }

    void Main()
    {
        const HRESULT initialized = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        HRESULT hr = initialized;
        if (SUCCEEDED(initialized))
        {
            try
            {
                while (!stop.load())
                {
                    const auto selection = requestedCamera.load();
                    hr = Run(selection);
                    if (FAILED(hr) || requestedCamera.load() == selection) break;
                    // Run's cleanup has stopped the old device and detached
                    // its callback before the next camera is opened.
                }
            }
            catch (...) { hr = E_OUTOFMEMORY; }
            CoUninitialize();
        }
        if (!stop.load() && FAILED(hr))
            failure.store(hr);
        finished.store(true);
    }
};

BeatDropWebcam::BeatDropWebcam() = default;
BeatDropWebcam::~BeatDropWebcam() { Stop(); }

bool BeatDropWebcam::StartCapture(IDirect3DDevice9* device, bool selectionOnly, bool notify)
{
    if (m_capture) Stop();
    try
    {
        m_capture.reset(new Capture);
        if (!m_capture->controlEvent.handle)
        {
            Stop();
            return false;
        }
        m_capture->selectionOnly = selectionOnly;
        m_capture->notifyCameraChange.store(selectionOnly && notify);
        if (device)
        {
            D3DCAPS9 caps;
            if (FAILED(device->GetDeviceCaps(&caps)))
            {
                Stop();
                return false;
            }
            m_capture->maxTextureWidth = caps.MaxTextureWidth;
            m_capture->maxTextureHeight = caps.MaxTextureHeight;
        }
        m_capture->requestedDevice = m_selectedDevice;
        m_capture->requestedIndex = m_cameraSelection;
        m_capture->requestedCamera.store(m_selectionRevision);
        m_capture->worker = std::thread(&Capture::Main, m_capture.get());
        return true;
    }
    catch (...)
    {
        Stop();
        return false;
    }
}

const wchar_t* BeatDropWebcam::Cycle(IDirect3DDevice9* device)
{
    if (m_mode == Top)
    {
        Stop();
        return L"Webcam disabled";
    }
    if (m_mode == Off && !StartCapture(device))
        return L"Unable to start webcam capture.";
    m_mode = static_cast<Mode>(m_mode + 1);
    if (m_mode != Deep) ReleaseDeepResources();
    switch (m_mode)
    {
    case Deep: return L"Webcam: deep";
    case DeepBurn: return L"Webcam: deep (with burn)";
    default: return L"Webcam: top";
    }
}

const wchar_t* BeatDropWebcam::NextCamera()
{
    if (!m_selectedDevice.empty() && m_capture)
        m_cameraSelection = m_capture->activeCameraIndex.load();
    m_selectedDevice.clear();
    ++m_cameraSelection;
    ++m_selectionRevision;
    if (m_mode == Off)
    {
        if (!StartCapture(nullptr, true)) return L"Unable to select webcam.";
        return L"";
    }
    {
        std::lock_guard<std::mutex> lock(m_capture->mutex);
        m_capture->requestedDevice.clear();
        m_capture->requestedIndex = m_cameraSelection;
        m_capture->requestedCamera.store(m_selectionRevision);
        m_capture->notifyCameraChange.store(true);
        m_capture->notice.clear();
    }
    m_capture->controlEvent.Signal();
    return L""; // Report the device name after the new camera opens.
}

HRESULT BeatDropWebcam::ListDevices(std::vector<Device>& devices)
{
    devices.clear();
    const HRESULT initialized = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(initialized)) return initialized;
    HRESULT hr;
    try
    {
        std::vector<CameraEntry> available;
        hr = EnumerateCameras(available);
        if (SUCCEEDED(hr))
            for (auto& camera : available) devices.push_back(std::move(camera.device));
    }
    catch (...) { devices.clear(); hr = E_OUTOFMEMORY; }
    CoUninitialize();
    return hr;
}

const wchar_t* BeatDropWebcam::SelectCamera(const std::wstring& id, bool notify)
{
    m_selectedDevice = id;
    ++m_selectionRevision; // invalidate any in-flight selection notification
    if (m_mode == Off)
        return StartCapture(nullptr, true, notify) ? L"" : L"Unable to select webcam.";
    {
        std::lock_guard<std::mutex> lock(m_capture->mutex);
        m_capture->requestedDevice = id;
        m_capture->requestedIndex = m_cameraSelection;
        m_capture->requestedCamera.store(m_selectionRevision);
        m_capture->notifyCameraChange.store(notify);
        m_capture->notice.clear();
    }
    m_capture->controlEvent.Signal();
    return L"";
}

const wchar_t* BeatDropWebcam::SetMode(Mode mode, IDirect3DDevice9* device)
{
    if (mode == Off) { Stop(); return L"Webcam disabled"; }
    if (m_mode == Off && !StartCapture(device)) return L"Unable to start webcam capture.";
    m_mode = mode;
    if (mode != Deep) ReleaseDeepResources();
    switch (mode)
    {
    case Deep: return L"Webcam: deep (no burn)";
    case DeepBurn: return L"Webcam: deep (with burn)";
    default: return L"Webcam: top";
    }
}

void BeatDropWebcam::Stop()
{
    if (m_capture)
    {
        if (!m_selectedDevice.empty())
            m_cameraSelection = m_capture->activeCameraIndex.load();
        m_capture->stop.store(true);
        m_capture->controlEvent.Signal();
        if (m_capture->worker.joinable())
            m_capture->worker.join();
        m_capture.reset();
    }
    m_mode = Off;
    ReleaseDeviceResources();
}

void BeatDropWebcam::ReleaseDeviceResources()
{
    ReleaseDeepResources();
    if (m_texture)
        m_texture->Release();
    m_texture = nullptr;
    m_width = m_height = 0;
    m_uploadedFrame = 0;
    m_captureIntervalSeconds = 0.0;
    std::vector<unsigned char>().swap(m_pixels);
}

void BeatDropWebcam::Update(IDirect3DDevice9* device, std::wstring& error, std::wstring* notice)
{
    error.clear();
    if (notice) notice->clear();
    if (!m_capture || !device) return;
    const HRESULT hr = m_capture->failure.load();
    if (FAILED(hr))
    {
        wchar_t code[16];
        swprintf_s(code, L"0x%08lX", static_cast<unsigned long>(hr));
        error = L"Webcam unavailable (" + std::wstring(code) +
            L"). Check that a camera is connected, camera access is allowed, and it is not in use by another app. Press W to retry.";
        Stop();
        return;
    }

    if (m_capture->selectionOnly)
    {
        // Read completion before taking the notice: if the worker publishes
        // just after this read, leave it alive for the next render update.
        const bool finished = m_capture->finished.load();
        {
            std::lock_guard<std::mutex> lock(m_capture->mutex);
            if (notice && m_capture->noticeSelection == m_capture->requestedCamera.load())
                notice->swap(m_capture->notice);
        }
        if (finished) Stop();
        return;
    }

    int width, height;
    bool bottomUp;
    unsigned long long frame;
    try
    {
        std::lock_guard<std::mutex> lock(m_capture->mutex);
        if (notice && m_capture->noticeSelection == m_capture->requestedCamera.load())
            notice->swap(m_capture->notice);
        if (!m_capture->haveFrame || m_capture->frame == m_uploadedFrame) return;
        width = m_capture->width;
        height = m_capture->height;
        bottomUp = m_capture->bottomUp;
        frame = m_capture->frame;
        m_captureIntervalSeconds = m_capture->frameIntervalSeconds;
        m_pixels.assign(m_capture->pixels.begin(), m_capture->pixels.end());
    }
    catch (...)
    {
        error = L"Unable to allocate webcam frame memory. Webcam disabled.";
        Stop();
        return;
    }
    if (!m_texture || m_width != width || m_height != height)
    {
        if (m_texture) m_texture->Release();
        m_texture = nullptr;
        if (FAILED(device->CreateTexture(width, height, 1,
            D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_texture, nullptr)))
            return;
        m_width = width;
        m_height = height;
    }
    D3DLOCKED_RECT locked;
    if (SUCCEEDED(m_texture->LockRect(0, &locked, nullptr, D3DLOCK_DISCARD)))
    {
        const size_t stride = static_cast<size_t>(m_width) * sizeof(DWORD);
        for (int y = 0; y < m_height; ++y)
        {
            const int sourceY = bottomUp ? m_height - 1 - y : y;
            std::memcpy(static_cast<BYTE*>(locked.pBits) + y * locked.Pitch,
                m_pixels.data() + sourceY * stride, stride);
        }
        m_texture->UnlockRect(0);
        m_uploadedFrame = frame;
    }
}

void BeatDropWebcam::Draw(IDirect3DDevice9* device, int width, int height, bool behind, float opacity)
{
    opacity = ClampWebcamOpacity(opacity, behind ? DefaultDeepOpacity : DefaultTopOpacity);
    DrawTexture(device, m_texture, width, height, behind, true, opacity);
}

void BeatDropWebcam::DrawBurn(IDirect3DDevice9* device, int width, int height, float opacity, double elapsedSeconds)
{
    opacity = ClampWebcamOpacity(opacity, DefaultBurnOpacity);
    if (opacity <= 0.0f) return;
    // Keep the camera present on every render frame, including between camera
    // deliveries. Spread one camera interval's opacity across those renders:
    // (1-a1)*(1-a2) = 1-a for the same elapsed capture interval.
    if (std::isfinite(elapsedSeconds) && elapsedSeconds > 0.0 &&
        std::isfinite(m_captureIntervalSeconds) && m_captureIntervalSeconds > 0.0)
    {
        const double fraction = (std::min)(1.0, elapsedSeconds / m_captureIntervalSeconds);
        opacity = static_cast<float>(1.0 - std::pow(1.0 - opacity, fraction));
    }
    DrawTexture(device, m_texture, width, height, true, true, opacity);
}

void BeatDropWebcam::ReleaseDeepResources()
{
    if (m_deepInput) m_deepInput->Release();
    if (m_deepOutput) m_deepOutput->Release();
    if (m_deepBaseline) m_deepBaseline->Release();
    m_deepInput = m_deepOutput = m_deepBaseline = nullptr;
}

bool BeatDropWebcam::PrepareDeepInput(IDirect3DDevice9* device, IDirect3DTexture9* reference, float opacity)
{
    opacity = ClampWebcamOpacity(opacity, DefaultDeepOpacity);
    if (opacity <= 0.0f) return false;
    if (m_mode != Deep || !device || !m_texture || !reference) return false;
    D3DSURFACE_DESC desc;
    if (FAILED(reference->GetLevelDesc(0, &desc))) return false;
    D3DSURFACE_DESC existing = {};
    if (m_deepInput) m_deepInput->GetLevelDesc(0, &existing);
    if (existing.Width != desc.Width || existing.Height != desc.Height || existing.Format != desc.Format)
        ReleaseDeepResources();
    if (!m_deepInput)
    {
        ComPtr<IDirect3DTexture9> input, output, baseline;
        if (FAILED(device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET,
                desc.Format, D3DPOOL_DEFAULT, input.GetAddressOf(), nullptr)) ||
            FAILED(device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET,
                desc.Format, D3DPOOL_DEFAULT, output.GetAddressOf(), nullptr)) ||
            FAILED(device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET,
                desc.Format, D3DPOOL_DEFAULT, baseline.GetAddressOf(), nullptr)))
            return false;
        m_deepInput = input.Detach();
        m_deepOutput = output.Detach();
        m_deepBaseline = baseline.Detach();
    }
    ComPtr<IDirect3DStateBlock9> state;
    ComPtr<IDirect3DSurface9> saved, input;
    if (FAILED(device->CreateStateBlock(D3DSBT_ALL, state.GetAddressOf())) ||
        FAILED(state->Capture()) || FAILED(device->GetRenderTarget(0, saved.GetAddressOf())) ||
        FAILED(m_deepInput->GetSurfaceLevel(0, input.GetAddressOf())))
        return false;
    for (DWORD stage = 0; stage < 16; ++stage) device->SetTexture(stage, nullptr);
    HRESULT hr = device->SetRenderTarget(0, input.Get());
    if (SUCCEEDED(hr))
    {
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        hr = device->Clear(0, nullptr, D3DCLEAR_TARGET, 0, 1, 0);
        if (SUCCEEDED(hr)) Draw(device, desc.Width, desc.Height, true, opacity);
    }
    device->SetRenderTarget(0, saved.Get());
    state->Apply();
    return SUCCEEDED(hr);
}

void BeatDropWebcam::BlendDeepScene(IDirect3DDevice9* device, IDirect3DTexture9* scene)
{
    D3DSURFACE_DESC desc;
    if (!m_deepOutput || FAILED(m_deepOutput->GetLevelDesc(0, &desc))) return;
    // Remove the shader-generated baseline before screen blending the clean scene.
    // A shader that ignores the camera input then contributes exactly zero.
    DrawTexture(device, m_deepBaseline, desc.Width, desc.Height, true, false, 1.0f, true);
    DrawTexture(device, scene, desc.Width, desc.Height, true, false);
}

bool BeatDropWebcam::DrawTexture(IDirect3DDevice9* device, IDirect3DTexture9* texture,
    int width, int height, bool behind, bool crop, float opacity, bool subtract)
{
    if (!device || !texture || width <= 0 || height <= 0) return false;
    D3DSURFACE_DESC desc;
    if (FAILED(texture->GetLevelDesc(0, &desc)) || !desc.Height) return false;
    ComPtr<IDirect3DStateBlock9> state;
    if (FAILED(device->CreateStateBlock(D3DSBT_ALL, state.GetAddressOf())) ||
        FAILED(state->Capture())) return false;

    // Fill the target without distorting the camera's aspect ratio.
    float u = 0.0f, v = 0.0f;
    const float cameraAspect = static_cast<float>(desc.Width) / desc.Height;
    const float targetAspect = static_cast<float>(width) / height;
    if (crop && cameraAspect > targetAspect) u = (1.0f - targetAspect / cameraAspect) * 0.5f;
    else if (crop) v = (1.0f - cameraAspect / targetAspect) * 0.5f;
    // Screen blending uses RGB, not SRCALPHA. Scale the source color to
    // apply opacity while preserving black transparency and bright details.
    const int strength = behind ? static_cast<int>(opacity * 255.0f + 0.5f) : 255;
    const int alpha = behind ? 255 : static_cast<int>(opacity * 255.0f + 0.5f);
    const DWORD color = D3DCOLOR_ARGB(alpha, strength, strength, strength);
    const WebcamVertex vertices[] = {
        { -0.5f, -0.5f, 0, 1, color, u, v },
        { width - 0.5f, -0.5f, 0, 1, color, 1-u, v },
        { -0.5f, height - 0.5f, 0, 1, color, u, 1-v },
        { width - 0.5f, height - 0.5f, 0, 1, color, 1-u, 1-v }
    };
    D3DVIEWPORT9 viewport = {0, 0, static_cast<DWORD>(width), static_cast<DWORD>(height), 0, 1};
    device->SetViewport(&viewport);
    device->SetVertexShader(nullptr);
    device->SetPixelShader(nullptr);
    device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    device->SetTexture(0, texture);
    device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    device->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT);
    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_WRAP0, 0);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
    device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
    device->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, FALSE);
    device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_BLENDOP, subtract ? D3DBLENDOP_REVSUBTRACT : D3DBLENDOP_ADD);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    // Both deep layers retain bright preset detail and replace its black
    // background with camera imagery. Top uses its configurable alpha.
    device->SetRenderState(D3DRS_SRCBLEND, subtract ? D3DBLEND_ONE : (behind ? D3DBLEND_INVDESTCOLOR : D3DBLEND_SRCALPHA));
    device->SetRenderState(D3DRS_DESTBLEND, behind ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);
    const HRESULT drawn = device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(WebcamVertex));
    state->Apply();
    return SUCCEEDED(drawn);
}
