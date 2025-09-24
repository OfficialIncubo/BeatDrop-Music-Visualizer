// audiodevicehandler.h
#ifndef AUDIODEVICEHANDLER_H
#define AUDIODEVICEHANDLER_H

#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

class AudioDeviceHandler {
public:
    AudioDeviceHandler();
    ~AudioDeviceHandler();

    HRESULT Initialize();
    HRESULT CheckForDeviceChanges(IMMDevice** ppNewDevice);
    void ResetToDefaultDevice();

private:
    IMMDeviceEnumerator* m_pEnumerator;
    IMMDevice* m_pCurrentDevice;
    DWORD m_dwCurrentFormatTag;
    DWORD m_dwCurrentSampleRate;

    HRESULT GetDeviceFormat(IMMDevice* pDevice, DWORD* pdwFormatTag, DWORD* pdwSampleRate);
    HRESULT IsBluetoothDevice(IMMDevice* pDevice, bool* pbIsBluetooth);
};

#endif