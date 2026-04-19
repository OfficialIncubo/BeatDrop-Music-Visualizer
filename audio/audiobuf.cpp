// audiobuf.cpp

#include "audiobuf.h"

#define SAMPLE_SIZE_LPB 2304 // Max number of audio samples stored in circular buffer. Larger buffer enables better low-frequency FFT resolution (~5.4 Hz/bin). Should be no less than SAMPLE_SIZE. Expected sampling rate is 44100 Hz or 48000 Hz (samples per second).

float g_fAudioSensitivity = 1.0f;

std::mutex pcmLpbMutex;
unsigned char pcmLeftLpb[SAMPLE_SIZE_LPB]; // Circular buffer (left channel)
unsigned char pcmRightLpb[SAMPLE_SIZE_LPB]; // Circular buffer (right channel)
float pcmLeftFloatLpb[SAMPLE_SIZE_LPB];
float pcmRightFloatLpb[SAMPLE_SIZE_LPB];
bool pcmBufDrained = false; // Buffer drained by visualization thread and holds no new samples
signed int pcmLen = 0; // Actual number of samples the buffer holds. Can be less than SAMPLE_SIZE_LPB
signed int pcmPos = 0; // Position to write new data

void ResetAudioBuf() {
    std::unique_lock<std::mutex> lock(pcmLpbMutex);
    memset(pcmLeftLpb, 0, SAMPLE_SIZE_LPB);
    memset(pcmRightLpb, 0, SAMPLE_SIZE_LPB);
    memset(pcmLeftFloatLpb, 0, sizeof(pcmLeftFloatLpb));
    memset(pcmRightFloatLpb, 0, sizeof(pcmRightFloatLpb));
    pcmBufDrained = false;
    pcmLen = 0;
}

void GetAudioBuf(unsigned char *pWaveL, unsigned char *pWaveR, int SamplesCount) {
    std::unique_lock<std::mutex> lock(pcmLpbMutex);

    static int consecutiveReads = 0;
    static int lastPcmPos = pcmPos;

    if (pcmPos == lastPcmPos) {
        consecutiveReads++;
    }
    else {
        consecutiveReads = 0;
        lastPcmPos = pcmPos;
    }

    if ((pcmLen < SamplesCount) || (consecutiveReads > 3)) {
        // Buffer underrun. Insufficient new samples in circular buffer (pcmLeftLpb, pcmRightLpb)
        memset(pWaveL, 0, SamplesCount);
        memset(pWaveR, 0, SamplesCount);
        if (consecutiveReads > 3)
       	    pcmBufDrained = true; // Drain buffer to force underrun next time
    }
    else {
        // Circular buffer (pcmLeftLpb, pcmRightLpb) hold enough samples in it
        for (int i = 0; i < SamplesCount; i++) {
            // int8_t [-128 .. +127] stored into uint8_t [0..255]
            pWaveL[i % SamplesCount] = pcmLeftLpb[(pcmPos + i) % SAMPLE_SIZE_LPB];
            pWaveR[i % SamplesCount] = pcmRightLpb[(pcmPos + i) % SAMPLE_SIZE_LPB];
        }
        //pcmBufDrained = true;
    }
}

void GetAudioBufFloat(float* pWaveL, float* pWaveR, int SamplesCount) {
    std::unique_lock<std::mutex> lock(pcmLpbMutex);
    static int consecutiveReads = 0;
    static int lastPcmPos = pcmPos;

    if (pcmPos == lastPcmPos) {
        consecutiveReads++;
    }
    else {
        consecutiveReads = 0;
        lastPcmPos = pcmPos;
    }

    if ((pcmLen == 0) || (consecutiveReads > 3)) {
        memset(pWaveL, 0, SamplesCount * sizeof(float));
        memset(pWaveR, 0, SamplesCount * sizeof(float));
        if (consecutiveReads > 3)
            pcmBufDrained = true;
    }
    else {
        // Zero-fill beginning if buffer not yet full; valid audio at end.
        // This allows larger FFT windows to work during startup (zero-padded front
        // is naturally tapered by the FFT window function).
        int available = (pcmLen < SamplesCount) ? pcmLen : SamplesCount;
        int zeroPrefix = SamplesCount - available;
        if (zeroPrefix > 0) {
            memset(pWaveL, 0, zeroPrefix * sizeof(float));
            memset(pWaveR, 0, zeroPrefix * sizeof(float));
        }
        for (int i = 0; i < available; i++) {
            // Match the legacy waveform amplitude domain used by the FFT path:
            // old m_sound.fWaveform samples were roughly in [-128..127].
            pWaveL[zeroPrefix + i] = pcmLeftFloatLpb[(pcmPos + i) % SAMPLE_SIZE_LPB] * 128.0f;
            pWaveR[zeroPrefix + i] = pcmRightFloatLpb[(pcmPos + i) % SAMPLE_SIZE_LPB] * 128.0f;
        }
    }
}

int8_t FltToInt(float flt) {
    if (flt >= 1.0f) {
        return +127; // 0x7f
    }
    if (flt < -1.0f) {
        return -128; // 0x80
    }
    return (int8_t)(flt * 128);
};

// Union type for sample conversion
union u_type
{
    int32_t IntVar;
    float FltVar;
    uint8_t Bytes[4];
};

int8_t GetChannelSample(const BYTE *pData, int BlockOffset, int ChannelOffset, const bool bInt16) {
    u_type sample;

    sample.IntVar = 0;
    sample.Bytes[0] = pData[BlockOffset + ChannelOffset + 0];
    sample.Bytes[1] = pData[BlockOffset + ChannelOffset + 1];
    if (!bInt16) {
        sample.Bytes[2] = pData[BlockOffset + ChannelOffset + 2];
        sample.Bytes[3] = pData[BlockOffset + ChannelOffset + 3];
    }

    if (!bInt16) {
        return FltToInt(sample.FltVar); //float [-1.0f .. +1.0f] range converted to int8_t [-128 .. +127] and later stored into uint8_t [0 .. 255]
    }
    else {
        float flt = ((int16_t)sample.IntVar) / 32768.0f;
        return FltToInt(flt); //int16_t [-32768 .. +32767] range converted to int8_t [-128 .. +127] and later stored into uint8_t [0..255]
    }
}

float GetChannelSampleFloat(const BYTE* pData, int BlockOffset, int ChannelOffset, const bool bInt16) {
    u_type sample;

    sample.IntVar = 0;
    sample.Bytes[0] = pData[BlockOffset + ChannelOffset + 0];
    sample.Bytes[1] = pData[BlockOffset + ChannelOffset + 1];
    if (!bInt16) {
        sample.Bytes[2] = pData[BlockOffset + ChannelOffset + 2];
        sample.Bytes[3] = pData[BlockOffset + ChannelOffset + 3];
        if (sample.FltVar >= 1.0f) return 1.0f;
        if (sample.FltVar <= -1.0f) return -1.0f;
        return sample.FltVar;
    }

    float v = (float)((int16_t)sample.IntVar) / 32768.0f;
    if (v >= 1.0f) return 1.0f;
    if (v <= -1.0f) return -1.0f;
    return v;
}

// Expecting pData holds:
//   signed 16-bit (2 bytes) PCM, Little Endian
//   or
//   32-bit float (4 bytes) PCM
// Supported audio formats:
//   pwfx->nChannels;          /* ANY number of channels (i.e. mono, stereo...) */
//   pwfx->nSamplesPerSec;     /* 44100 or 48000 sample rate */
//   pwfx->nBlockAlign;        /* ANY block size of data */
//   pwfx->wBitsPerSample;     /* 16 or 32 number of bits per sample of mono data */

void SetAudioBuf(const BYTE *pData, const UINT32 nNumFramesToRead, const WAVEFORMATEX *pwfx, const bool bInt16) {
    std::unique_lock<std::mutex> lock(pcmLpbMutex);
    //memset(pcmLeftLpb, 0, SAMPLE_SIZE_LPB);
    //memset(pcmRightLpb, 0, SAMPLE_SIZE_LPB);

    // Determine downsampling ratio (supporting 96kHz and 192kHz)
    int downsampleRatio = 1;
    if (pwfx->nSamplesPerSec > TARGET_SAMPLE_RATE) {
        downsampleRatio = pwfx->nSamplesPerSec / TARGET_SAMPLE_RATE;
    }

    // Calculate output samples after downsampling
    int outputSamples = nNumFramesToRead / downsampleRatio;

    // Adjust buffer writing parameters for downsampled data
    int n = 0;
    int start = 0;
    int len = outputSamples;
    if (outputSamples >= SAMPLE_SIZE_LPB) {
        n = 0;
        start = outputSamples - SAMPLE_SIZE_LPB;
        len = SAMPLE_SIZE_LPB;
    }
    else {
        n = SAMPLE_SIZE_LPB - outputSamples;
        start = 0;
        len = outputSamples;
    }

    for (int i = start; i < len; i++, n++) {
        float sumLeft = 0.0f;
        float sumRight = 0.0f;

        // Average samples for downsampling
        for (int j = 0; j < downsampleRatio; j++) {
            int inputIndex = i * downsampleRatio + j;
            if (inputIndex >= nNumFramesToRead) break;

            int blockOffset = inputIndex * pwfx->nBlockAlign;

            // Get left channel sample
            float sampleLeft = 0.0f;
            if (pData && pwfx->nChannels >= 1) {
                sampleLeft = GetChannelSampleFloat(pData, blockOffset, 0, bInt16);
            }
            sumLeft += sampleLeft;

            // Get right channel sample (use left if mono)
            float sampleRight = sampleLeft;
            if (pData && pwfx->nChannels >= 2) {
                sampleRight = GetChannelSampleFloat(pData, blockOffset, pwfx->wBitsPerSample / 8, bInt16);
            }
            sumRight += sampleRight;
        }

		/*
        Added support for 96000 Hz and 192000 Hz sample rates.
        Technically downsamples when it's above the target sample rate
        provided in audiobuf.h
        */

        // Ignoring data in all other audio channels (Quadraphonic 4.0, Surround 4.0, Surround 5.1, Surround 7.1, ...)

        // Saving audio data for visualizer
        // 8-bit signed integer in Two's Complement Representation stored in unsigned char array
        // int8_t[-128 .. + 127] stored into uint8_t[0 .. 255]

        // Store averaged/downsampled values
        pcmLeftFloatLpb[(pcmPos + n) % SAMPLE_SIZE_LPB] = sumLeft / downsampleRatio;
        pcmRightFloatLpb[(pcmPos + n) % SAMPLE_SIZE_LPB] = sumRight / downsampleRatio;
        pcmLeftLpb[(pcmPos + n) % SAMPLE_SIZE_LPB] = (uint8_t)FltToInt(sumLeft / downsampleRatio);
        pcmRightLpb[(pcmPos + n) % SAMPLE_SIZE_LPB] = (uint8_t)FltToInt(sumRight / downsampleRatio);
    }

    pcmBufDrained = false;
    pcmLen = (pcmLen + len <= SAMPLE_SIZE_LPB) ? (pcmLen + len) : (SAMPLE_SIZE_LPB);
    pcmPos = (pcmPos + len) % SAMPLE_SIZE_LPB;

}
