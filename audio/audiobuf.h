// audiobuf.h

#include <mutex>
#include <windows.h>

#define TARGET_SAMPLE_RATE 44100

extern float g_fAudioSensitivity;

// Reset audio buffer discarding stored audio data
void ResetAudioBuf();

// Return previously saved audio data for visualizer
void GetAudioBuf(unsigned char *pWaveL, unsigned char *pWaveR, int SamplesCount);

// Return previously saved audio data as normalized float samples in [-1..1]
void GetAudioBufFloat(float* pWaveL, float* pWaveR, int SamplesCount);

// Save audio data for visualizer
void SetAudioBuf(const BYTE *pData, const UINT32 nNumFramesToRead, const WAVEFORMATEX *pwfx, const bool bInt16);
