// loopback-capture.cpp

#include "common.h"
#include "audiodevicehandler.h"

extern bool GetCaptureMicFlag();

static AudioDeviceHandler* g_pAudioDeviceHandler = NULL;

HRESULT LoopbackCapture(
    IMMDevice *pMMDevice,
    HMMIO hFile,
    bool bInt16,
    HANDLE hStartedEvent,
    HANDLE hStopEvent,
    PUINT32 pnFrames
);

HRESULT WriteWaveHeader(HMMIO hFile, LPCWAVEFORMATEX pwfx, MMCKINFO *pckRIFF, MMCKINFO *pckData);
HRESULT FinishWaveFile(HMMIO hFile, MMCKINFO *pckRIFF, MMCKINFO *pckData);

DWORD WINAPI LoopbackCaptureThreadFunction(LPVOID pContext) {
    LoopbackCaptureThreadFunctionArguments *pArgs =
        (LoopbackCaptureThreadFunctionArguments*)pContext;

    pArgs->hr = CoInitialize(NULL);
    if (FAILED(pArgs->hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", pArgs->hr);
        return 0;
    }
    CoUninitializeOnExit cuoe;

    while (true) {
        pArgs->hr = LoopbackCapture(
            pArgs->pMMDevice,
            pArgs->hFile,
            pArgs->bInt16,
            pArgs->hStartedEvent,
            pArgs->hStopEvent,
            &pArgs->nFrames
        );

        // Only retry if device was invalidated
        if (pArgs->hr != AUDCLNT_E_DEVICE_INVALIDATED) {
            break; // Exit loop for any other error or normal completion
        }

        // Device was invalidated - try to recover
        if (g_pAudioDeviceHandler) {
            g_pAudioDeviceHandler->ResetToDefaultDevice();

            // Update the device in arguments for next retry
            if (pArgs->pMMDevice) {
                pArgs->pMMDevice->Release();
            }
            HRESULT hr;
            // Use microphone if enabled, otherwise use speaker loopback
            if (GetCaptureMicFlag()) {
                // Get default input device (microphone)
                hr = pArgs->pMMDevice->Release();
                hr = g_pAudioDeviceHandler->m_pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pArgs->pMMDevice);
            }
            else {
                // Get default output device (speaker)
                hr = g_pAudioDeviceHandler->CheckForDeviceChanges(&pArgs->pMMDevice);
            }
            if (FAILED(hr)) {
                ERR(L"Failed to get new audio device after invalidation: hr = 0x%08x", hr);
                break; // Can't recover if we can't get a new device
            }

            LOG(L"Audio device invalidated - retrying with new default device");
            continue; // Retry with the new device
        }
        else {
            ERR(L"Audio device invalidated but no device handler available");
            break; // Can't recover without device handler
        }
    }

    return 0;
}

HRESULT LoopbackCapture(
    IMMDevice *pMMDevice,
    HMMIO hFile,
    bool bInt16,
    HANDLE hStartedEvent,
    HANDLE hStopEvent,
    PUINT32 pnFrames
) {
    HRESULT hr;

    // Initialize device handler if not already done
    if (!g_pAudioDeviceHandler) {
        g_pAudioDeviceHandler = new AudioDeviceHandler();
        hr = g_pAudioDeviceHandler->Initialize();
        if (FAILED(hr)) {
            ERR(L"Failed to initialize audio device handler: hr = 0x%08x", hr);
            // Continue without device change detection
        }
    }

    // activate an IAudioClient
    IAudioClient *pAudioClient;
    hr = pMMDevice->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL, NULL,
        (void**)&pAudioClient
    );
    if (FAILED(hr)) {
        ERR(L"IMMDevice::Activate(IAudioClient) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseAudioClient(pAudioClient);
    
    // get the default device periodicity
    REFERENCE_TIME hnsDefaultDevicePeriod;
    hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, NULL);
    if (FAILED(hr)) {
        ERR(L"IAudioClient::GetDevicePeriod failed: hr = 0x%08x", hr);
        return hr;
    }

    // get the default device format
    WAVEFORMATEX *pwfx;
    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        ERR(L"IAudioClient::GetMixFormat failed: hr = 0x%08x", hr);
        return hr;
    }
    CoTaskMemFreeOnExit freeMixFormat(pwfx);

    if (bInt16) {
        // coerce int-16 wave format
        // can do this in-place since we're not changing the size of the format
        // also, the engine will auto-convert from float to int for us
        switch (pwfx->wFormatTag) {
            case WAVE_FORMAT_IEEE_FLOAT:
                pwfx->wFormatTag = WAVE_FORMAT_PCM;
                pwfx->wBitsPerSample = 16;
                pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
                pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
                break;

            case WAVE_FORMAT_EXTENSIBLE:
                {
                    // naked scope for case-local variable
                    PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
                    if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
                        pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                        pEx->Samples.wValidBitsPerSample = 16;
                        pwfx->wBitsPerSample = 16;
                        pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
                        pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
                    } else {
                        ERR(L"%s", L"Don't know how to coerce mix format to int-16");
                        return E_UNEXPECTED;
                    }
                }
                break;

            default:
                ERR(L"Don't know how to coerce WAVEFORMATEX with wFormatTag = 0x%08x to int-16", pwfx->wFormatTag);
                return E_UNEXPECTED;
        }
    }

    MMCKINFO ckRIFF = { 0 };
    MMCKINFO ckData = { 0 };

    if (NULL != hFile) {
        hr = WriteWaveHeader(hFile, pwfx, &ckRIFF, &ckData);
        if (FAILED(hr)) {
            // WriteWaveHeader does its own logging
            return hr;
        }
    }

    // create a periodic waitable timer
    HANDLE hWakeUp = CreateWaitableTimer(NULL, FALSE, NULL);
    if (NULL == hWakeUp) {
        DWORD dwErr = GetLastError();
        ERR(L"CreateWaitableTimer failed: last error = %u", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }
    CloseHandleOnExit closeWakeUp(hWakeUp);

    UINT32 nBlockAlign = pwfx->nBlockAlign;
    *pnFrames = 0;
    
    // call IAudioClient::Initialize
    // note that AUDCLNT_STREAMFLAGS_LOOPBACK and AUDCLNT_STREAMFLAGS_EVENTCALLBACK
    // do not work together...
    // the "data ready" event never gets set
    // so we're going to do a timer-driven loop
    DWORD streamFlags;
    if (GetCaptureMicFlag()) {
        // For microphone - regular capture
        streamFlags = 0;
    }
    else {
        // For speaker - loopback capture  
        streamFlags = AUDCLNT_STREAMFLAGS_LOOPBACK;
    }

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        streamFlags,
        0, 0, pwfx, 0
    );
    if (FAILED(hr)) {
        ERR(L"IAudioClient::Initialize failed: hr = 0x%08x", hr);
        return hr;
    }

    // activate an IAudioCaptureClient
    IAudioCaptureClient *pAudioCaptureClient;
    hr = pAudioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&pAudioCaptureClient
    );
    if (FAILED(hr)) {
        ERR(L"IAudioClient::GetService(IAudioCaptureClient) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseAudioCaptureClient(pAudioCaptureClient);
    /*
    // register with MMCSS
    DWORD nTaskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(L"Audio", &nTaskIndex);
    if (NULL == hTask) {
        DWORD dwErr = GetLastError();
        ERR(L"AvSetMmThreadCharacteristics failed: last error = %u", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }
    AvRevertMmThreadCharacteristicsOnExit unregisterMmcss(hTask);
    */
    // set the waitable timer
    LARGE_INTEGER liFirstFire;
    liFirstFire.QuadPart = -hnsDefaultDevicePeriod / 2; // negative means relative time
    LONG lTimeBetweenFires = (LONG)hnsDefaultDevicePeriod / 2 / (10 * 1000); // convert to milliseconds
    BOOL bOK = SetWaitableTimer(
        hWakeUp,
        &liFirstFire,
        lTimeBetweenFires,
        NULL, NULL, FALSE
    );
    if (!bOK) {
        DWORD dwErr = GetLastError();
        ERR(L"SetWaitableTimer failed: last error = %u", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }
    CancelWaitableTimerOnExit cancelWakeUp(hWakeUp);
    
    // call IAudioClient::Start
    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        ERR(L"IAudioClient::Start failed: hr = 0x%08x", hr);
        return hr;
    }
    AudioClientStopOnExit stopAudioClient(pAudioClient);

    SetEvent(hStartedEvent);
    
    // loopback capture loop
    HANDLE waitArray[2] = { hStopEvent, hWakeUp };
    DWORD dwWaitResult;

    bool bDone = false;
    bool bFirstPacket = true;

    bool bErrorInAudioData = false;

    for (UINT32 nPasses = 0; !bDone; nPasses++) {

        if (g_pAudioDeviceHandler && (nPasses % 100 == 0)) {
            IMMDevice* pNewDevice = NULL;
            HRESULT deviceCheckHr;

            if (GetCaptureMicFlag()) {
                // For microphone - get default input device
                deviceCheckHr = g_pAudioDeviceHandler->m_pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pNewDevice);
            }
            else {
                // For speaker - check for device changes
                deviceCheckHr = g_pAudioDeviceHandler->CheckForDeviceChanges(&pNewDevice);
            }

            if (SUCCEEDED(deviceCheckHr)) {
                // Compare device IDs to see if device changed
                LPWSTR szCurrentId = NULL;
                LPWSTR szNewId = NULL;
                BOOL bDeviceChanged = FALSE;

                if (SUCCEEDED(pMMDevice->GetId(&szCurrentId))) {
                    if (SUCCEEDED(pNewDevice->GetId(&szNewId))) {
                        if (wcscmp(szCurrentId, szNewId) != 0) {
                            LOG(L"Audio device changed: %ls -> %ls", szCurrentId, szNewId);
                            bDeviceChanged = TRUE;
                        }
                        CoTaskMemFree(szNewId);
                    }
                    CoTaskMemFree(szCurrentId);
                }

                if (bDeviceChanged) {
                    LOG(L"Audio device change detected, stopping capture");
                    bDone = true;
                    hr = AUDCLNT_E_DEVICE_INVALIDATED; // Signal device change
                    if (pNewDevice) {
                        pNewDevice->Release();
                    }
                    continue;
                }
                if (pNewDevice) {
                    pNewDevice->Release();
                }
            }
        }

        // drain data while it is available
        UINT32 nNextPacketSize;
        for (
            hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize);
            SUCCEEDED(hr) && nNextPacketSize > 0;
            hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize)
        ) {
            // get the captured data
            BYTE *pData;
            UINT32 nNumFramesToRead;
            DWORD dwFlags;

            hr = pAudioCaptureClient->GetBuffer(
                &pData,
                &nNumFramesToRead,
                &dwFlags,
                NULL,
                NULL
                );

            bErrorInAudioData = false;

            if (FAILED(hr)) {
                bErrorInAudioData = true;
                ERR(L"IAudioCaptureClient::GetBuffer failed on pass %u after %u frames: hr = 0x%08x", nPasses, *pnFrames, hr);
                return hr;
            }

            if (bFirstPacket && AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY == dwFlags) {
                //bErrorInAudioData = true;
                LOG(L"%s", L"Probably spurious glitch reported on first packet");
            } else if (0 != dwFlags) {
                bErrorInAudioData = true;
                LOG(L"IAudioCaptureClient::GetBuffer set flags to 0x%08x on pass %u after %u frames", dwFlags, nPasses, *pnFrames);
                //return E_UNEXPECTED;
            }

            if (0 == nNumFramesToRead) {
                bErrorInAudioData = true;
                ERR(L"IAudioCaptureClient::GetBuffer said to read 0 frames on pass %u after %u frames", nPasses, *pnFrames);
                //return E_UNEXPECTED;
            }

            if (bErrorInAudioData) {
                // Glitch in audio detected so we reset audio buffer and avoid writing to the output .wav file
                ResetAudioBuf();
            }
            else
            {
                // Saving audio data for visualizer
                SetAudioBuf(pData, nNumFramesToRead, pwfx, bInt16);
                
                if (NULL != hFile) {
                    // Writing the buffer captured to the output .wav file
                    LONG lBytesToWrite = nNumFramesToRead * nBlockAlign;
#pragma prefast(suppress: __WARNING_INCORRECT_ANNOTATION, "IAudioCaptureClient::GetBuffer SAL annotation implies a 1-byte buffer")
                    LONG lBytesWritten = mmioWrite(hFile, reinterpret_cast<PCHAR>(pData), lBytesToWrite);
                    if (lBytesToWrite != lBytesWritten) {
                        ERR(L"mmioWrite wrote %u bytes on pass %u after %u frames: expected %u bytes", lBytesWritten, nPasses, *pnFrames, lBytesToWrite);
                        return E_UNEXPECTED;
                    }
                }
                *pnFrames += nNumFramesToRead;
            }

            hr = pAudioCaptureClient->ReleaseBuffer(nNumFramesToRead);
            if (FAILED(hr)) {
                ERR(L"IAudioCaptureClient::ReleaseBuffer failed on pass %u after %u frames: hr = 0x%08x", nPasses, *pnFrames, hr);
                return hr;
            }

            bFirstPacket = false;
        }

        if (FAILED(hr)) {
            ERR(L"IAudioCaptureClient::GetNextPacketSize failed on pass %u after %u frames: hr = 0x%08x", nPasses, *pnFrames, hr);
            return hr;
        }

        dwWaitResult = WaitForMultipleObjects(
            ARRAYSIZE(waitArray), waitArray,
            FALSE, INFINITE
        );

        if (WAIT_OBJECT_0 == dwWaitResult) {
            LOG(L"Received stop event after %u passes and %u frames", nPasses, *pnFrames);
            bDone = true;
            continue; // exits loop
        }

        if (WAIT_OBJECT_0 + 1 != dwWaitResult) {
            ERR(L"Unexpected WaitForMultipleObjects return value %u on pass %u after %u frames", dwWaitResult, nPasses, *pnFrames);
            return E_UNEXPECTED;
        }
    } // capture loop

    if (NULL != hFile) {
        hr = FinishWaveFile(hFile, &ckData, &ckRIFF);
        if (FAILED(hr)) {
            // FinishWaveFile does it's own logging
            return hr;
        }
    }

    return hr;
}

HRESULT WriteWaveHeader(HMMIO hFile, LPCWAVEFORMATEX pwfx, MMCKINFO *pckRIFF, MMCKINFO *pckData) {
    MMRESULT result;

    // make a RIFF/WAVE chunk
    pckRIFF->ckid = MAKEFOURCC('R', 'I', 'F', 'F');
    pckRIFF->fccType = MAKEFOURCC('W', 'A', 'V', 'E');

    result = mmioCreateChunk(hFile, pckRIFF, MMIO_CREATERIFF);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioCreateChunk(\"RIFF/WAVE\") failed: MMRESULT = 0x%08x", result);
        return E_FAIL;
    }
    
    // make a 'fmt ' chunk (within the RIFF/WAVE chunk)
    MMCKINFO chunk;
    chunk.ckid = MAKEFOURCC('f', 'm', 't', ' ');
    result = mmioCreateChunk(hFile, &chunk, 0);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioCreateChunk(\"fmt \") failed: MMRESULT = 0x%08x", result);
        return E_FAIL;
    }

    // write the WAVEFORMATEX data to it
    LONG lBytesInWfx = sizeof(WAVEFORMATEX) + pwfx->cbSize;
    LONG lBytesWritten =
        mmioWrite(
            hFile,
            reinterpret_cast<PCHAR>(const_cast<LPWAVEFORMATEX>(pwfx)),
            lBytesInWfx
        );
    if (lBytesWritten != lBytesInWfx) {
        ERR(L"mmioWrite(fmt data) wrote %u bytes; expected %u bytes", lBytesWritten, lBytesInWfx);
        return E_FAIL;
    }

    // ascend from the 'fmt ' chunk
    result = mmioAscend(hFile, &chunk, 0);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioAscend(\"fmt \" failed: MMRESULT = 0x%08x", result);
        return E_FAIL;
    }
    
    // make a 'fact' chunk whose data is (DWORD)0
    chunk.ckid = MAKEFOURCC('f', 'a', 'c', 't');
    result = mmioCreateChunk(hFile, &chunk, 0);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioCreateChunk(\"fmt \") failed: MMRESULT = 0x%08x", result);
        return E_FAIL;
    }

    // write (DWORD)0 to it
    // this is cleaned up later
    DWORD frames = 0;
    lBytesWritten = mmioWrite(hFile, reinterpret_cast<PCHAR>(&frames), sizeof(frames));
    if (lBytesWritten != sizeof(frames)) {
        ERR(L"mmioWrite(fact data) wrote %u bytes; expected %u bytes", lBytesWritten, (UINT32)sizeof(frames));
        return E_FAIL;
    }

    // ascend from the 'fact' chunk
    result = mmioAscend(hFile, &chunk, 0);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioAscend(\"fact\" failed: MMRESULT = 0x%08x", result);
        return E_FAIL;
    }

    // make a 'data' chunk and leave the data pointer there
    pckData->ckid = MAKEFOURCC('d', 'a', 't', 'a');
    result = mmioCreateChunk(hFile, pckData, 0);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioCreateChunk(\"data\") failed: MMRESULT = 0x%08x", result);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT FinishWaveFile(HMMIO hFile, MMCKINFO *pckRIFF, MMCKINFO *pckData) {
    MMRESULT result;

    result = mmioAscend(hFile, pckData, 0);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioAscend(\"data\" failed: MMRESULT = 0x%08x", result);
        return E_FAIL;
    }

    result = mmioAscend(hFile, pckRIFF, 0);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioAscend(\"RIFF/WAVE\" failed: MMRESULT = 0x%08x", result);
        return E_FAIL;
    }

    return S_OK;    
}
