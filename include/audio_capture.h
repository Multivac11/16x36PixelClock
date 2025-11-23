#pragma once

#include "driver/i2s.h"
#include "audio_capture.h"
#include <Arduino.h>

#define SAMPLE_RATE (44100)

#define I2S_MIC_WS 6
#define I2S_MIC_SD 7
#define I2S_MIC_BCK 15
#define I2S_PORT I2S_NUM_0
#define bufferLen 64

class AudioCapture 
{
    public:
        static AudioCapture& GetInstance()
        {
            static AudioCapture instance;
            return instance;
        }

        AudioCapture() = default;
        ~AudioCapture() = default;

        void InitAudioCapture();

    private:
        static void AudioCaptureTask(void *);
        void I2sInstall();
        void I2sSetPin();
        void GetAudios();

    private:
        QueueHandle_t _queue;
        i2s_config_t _i2s_config;
        i2s_pin_config_t _pin_config;
        uint32_t _buffer[bufferLen];
        size_t _bytesIn = 0;
        esp_err_t _result;
};