// audiodevicehandler.cpp
#include "audiodevicehandler.h"
#include "common.h"
extern bool GetCaptureMicFlag();

AudioDeviceHandler::AudioDeviceHandler()
    : m_pEnumerator(NULL)
    , m_pCurrentDevice(NULL)
    , m_dwCurrentFormatTag(0)
    , m_dwCurrentSampleRate(0) {
}

AudioDeviceHandler::~AudioDeviceHandler() {
    if (m_pCurrentDevice) {
        m_pCurrentDevice->Release();
    }
    if (m_pEnumerator) {
        m_pEnumerator->Release();
    }
}

HRESULT AudioDeviceHandler::Initialize() {
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        (void**)&m_pEnumerator);
    if (FAILED(hr)) {
        ERR(L"Failed to create device enumerator: hr = 0x%08x", hr);
        return hr;
    }

    // Get initial default device
    if (GetCaptureMicFlag()) {
        hr = m_pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &m_pCurrentDevice);
    }
    else {
        hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_pCurrentDevice);
    }
    if (FAILED(hr)) {
        ERR(L"Failed to get default audio endpoint: hr = 0x%08x", hr);
        return hr;
    }

    // Get initial format info
    return GetDeviceFormat(m_pCurrentDevice, &m_dwCurrentFormatTag, &m_dwCurrentSampleRate);
}

HRESULT AudioDeviceHandler::CheckForDeviceChanges(IMMDevice** ppNewDevice) {
    if (!m_pEnumerator || !ppNewDevice) {
        return E_INVALIDARG;
    }

    *ppNewDevice = NULL;

    // 1. Check if default device changed
    IMMDevice* pNewDefaultDevice = NULL;
    HRESULT hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pNewDefaultDevice);
    if (FAILED(hr)) {
        ERR(L"Failed to get default audio endpoint: hr = 0x%08x", hr);
        return hr;
    }

    // Compare device IDs to see if default device changed
    LPWSTR szCurrentId = NULL;
    LPWSTR szNewId = NULL;

    BOOL bDeviceChanged = FALSE;

    if (SUCCEEDED(m_pCurrentDevice->GetId(&szCurrentId))) {
        if (SUCCEEDED(pNewDefaultDevice->GetId(&szNewId))) {
            if (wcscmp(szCurrentId, szNewId) != 0) {
                LOG(L"Audio device changed: %ls -> %ls", szCurrentId, szNewId);
                bDeviceChanged = TRUE;
            }
            CoTaskMemFree(szNewId);
        }
        CoTaskMemFree(szCurrentId);
    }

    // 2. Check if current device format changed
    DWORD dwNewFormatTag, dwNewSampleRate;
    if (SUCCEEDED(GetDeviceFormat(m_pCurrentDevice, &dwNewFormatTag, &dwNewSampleRate))) {
        if (dwNewFormatTag != m_dwCurrentFormatTag || dwNewSampleRate != m_dwCurrentSampleRate) {
            LOG(L"Audio format changed: Format %u->%u, SampleRate %u->%u",
                m_dwCurrentFormatTag, dwNewFormatTag, m_dwCurrentSampleRate, dwNewSampleRate);
            bDeviceChanged = TRUE;
        }
    }

    // 3. Check if Bluetooth device disconnected
    bool bIsBluetooth = false;
    if (SUCCEEDED(IsBluetoothDevice(m_pCurrentDevice, &bIsBluetooth)) && bIsBluetooth) {
        // Check if Bluetooth device is still available
        DWORD dwState;
        if (SUCCEEDED(m_pCurrentDevice->GetState(&dwState))) {
            if (dwState != DEVICE_STATE_ACTIVE) {
                LOG(L"Bluetooth device disconnected or inactive");
                bDeviceChanged = TRUE;
            }
        }
    }

    if (bDeviceChanged) {
        // Release old device
        if (m_pCurrentDevice) {
            m_pCurrentDevice->Release();
            m_pCurrentDevice = NULL;
        }

        // Use the new default device
        m_pCurrentDevice = pNewDefaultDevice;
        m_pCurrentDevice->AddRef(); // AddRef since we're keeping it

        // Update format info
        GetDeviceFormat(m_pCurrentDevice, &m_dwCurrentFormatTag, &m_dwCurrentSampleRate);

        *ppNewDevice = pNewDefaultDevice;
        pNewDefaultDevice->AddRef(); // Caller will release this

        LOG(L"Switched to new default audio device");
    }
    else {
        // No change, return current device with AddRef
        m_pCurrentDevice->AddRef();
        *ppNewDevice = m_pCurrentDevice;
    }

    pNewDefaultDevice->Release(); // Release our local reference
    return S_OK;
}

void AudioDeviceHandler::ResetToDefaultDevice() {
    if (m_pEnumerator) {
        IMMDevice* pNewDevice = NULL;
        HRESULT hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pNewDevice);
        if (SUCCEEDED(hr)) {
            if (m_pCurrentDevice) {
                m_pCurrentDevice->Release();
            }
            m_pCurrentDevice = pNewDevice;
            GetDeviceFormat(m_pCurrentDevice, &m_dwCurrentFormatTag, &m_dwCurrentSampleRate);
            LOG(L"Reset to default audio device");
        }
    }
}

HRESULT AudioDeviceHandler::GetDeviceFormat(IMMDevice* pDevice, DWORD* pdwFormatTag, DWORD* pdwSampleRate) {
    if (!pDevice || !pdwFormatTag || !pdwSampleRate) {
        return E_INVALIDARG;
    }

    IPropertyStore* pProps = NULL;
    PROPVARIANT var;
    PropVariantInit(&var);

    HRESULT hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (SUCCEEDED(hr)) {
        hr = pProps->GetValue(PKEY_AudioEngine_DeviceFormat, &var);
        if (SUCCEEDED(hr) && var.blob.pBlobData) {
            WAVEFORMATEX* pWfx = (WAVEFORMATEX*)var.blob.pBlobData;
            *pdwFormatTag = pWfx->wFormatTag;
            *pdwSampleRate = pWfx->nSamplesPerSec;
        }
        pProps->Release();
    }

    PropVariantClear(&var);
    return hr;
}

HRESULT AudioDeviceHandler::IsBluetoothDevice(IMMDevice* pDevice, bool* pbIsBluetooth) {
    if (!pDevice || !pbIsBluetooth) {
        return E_INVALIDARG;
    }

    *pbIsBluetooth = false;

    IPropertyStore* pProps = NULL;
    PROPVARIANT var;
    PropVariantInit(&var);

    HRESULT hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (SUCCEEDED(hr)) {
        hr = pProps->GetValue(PKEY_Device_Class, &var);
        if (SUCCEEDED(hr) && var.pwszVal) {
            // Check if device class indicates Bluetooth
            if (wcsstr(var.pwszVal, L"Bluetooth") != NULL) {
                *pbIsBluetooth = true;
            }
        }
        pProps->Release();
    }

    PropVariantClear(&var);
    return hr;
}