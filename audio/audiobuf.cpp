// audiobuf.cpp

#include "audiobuf.h"

#define SAMPLE_SIZE_LPB 576 // Max number of audio samples stored in circular buffer. Should be no less than SAMPLE_SIZE. Expected sampling rate is 44100 Hz or 48000 Hz (samples per second).

std::mutex pcmLpbMutex;
unsigned char pcmLeftLpb[SAMPLE_SIZE_LPB]; // Circular buffer (left channel)
unsigned char pcmRightLpb[SAMPLE_SIZE_LPB]; // Circular buffer (right channel)
bool pcmBufDrained = false; // Buffer drained by visualization thread and holds no new samples
signed int pcmLen = 0; // Actual number of samples the buffer holds. Can be less than SAMPLE_SIZE_LPB
signed int pcmPos = 0; // Position to write new data

void ResetAudioBuf() {
    std::unique_lock<std::mutex> lock(pcmLpbMutex);
    memset(pcmLeftLpb, 0, SAMPLE_SIZE_LPB);
    memset(pcmRightLpb, 0, SAMPLE_SIZE_LPB);
    pcmBufDrained = false;
    pcmLen = 0;
}

void GetAudioBuf(unsigned char *pWaveL, unsigned char *pWaveR, int SamplesCount) {
    std::unique_lock<std::mutex> lock(pcmLpbMutex);
    if ((pcmLen < SamplesCount) || (pcmBufDrained)) {
        // Buffer underrun. Insufficient new samples in circular buffer (pcmLeftLpb, pcmRightLpb)
        memset(pWaveL, 0, SamplesCount);
        memset(pWaveR, 0, SamplesCount);
    }
    else {
        // Circular buffer (pcmLeftLpb, pcmRightLpb) hold enough samples in it
        for (int i = 0; i < SAMPLE_SIZE_LPB; i++) {
            // int8_t [-128 .. +127] stored into uint8_t [0..255]
            pWaveL[i % SamplesCount] = pcmLeftLpb[(pcmPos + i) % SAMPLE_SIZE_LPB];
            pWaveR[i % SamplesCount] = pcmRightLpb[(pcmPos + i) % SAMPLE_SIZE_LPB];
        }
        pcmBufDrained = true;
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
        return (signed char)(sample.IntVar / 256); //int16_t [-32768 .. +32767] range converted to int8_t [-128 .. +127] and later stored into uint8_t [0..255]
    }
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
    int BlockOffset;

    int8_t LeftSample8;
    int8_t RightSample8;


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
        int32_t sumLeft = 0;
        int32_t sumRight = 0;

        // Average samples for downsampling
        for (int j = 0; j < downsampleRatio; j++) {
            int inputIndex = i * downsampleRatio + j;
            if (inputIndex >= nNumFramesToRead) break;

            int blockOffset = inputIndex * pwfx->nBlockAlign;

            // Get left channel sample
            int8_t sampleLeft = 0;
            if (pwfx->nChannels >= 1) {
                sampleLeft = GetChannelSample(pData, blockOffset, 0, bInt16);
            }
            sumLeft += sampleLeft;

            // Get right channel sample (use left if mono)
            int8_t sampleRight = sampleLeft;
            if (pwfx->nChannels >= 2) {
                sampleRight = GetChannelSample(pData, blockOffset, pwfx->wBitsPerSample / 8, bInt16);
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
        pcmLeftLpb[(pcmPos + n) % SAMPLE_SIZE_LPB] = sumLeft / downsampleRatio;
        pcmRightLpb[(pcmPos + n) % SAMPLE_SIZE_LPB] = sumRight / downsampleRatio;
    }

    pcmBufDrained = false;
    pcmLen = (pcmLen + len <= SAMPLE_SIZE_LPB) ? (pcmLen + len) : (SAMPLE_SIZE_LPB);
    pcmPos = (pcmPos + len) % SAMPLE_SIZE_LPB;

}
