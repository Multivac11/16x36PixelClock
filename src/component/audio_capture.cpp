#include "audio_capture.h"

void AudioCapture::InitAudioCapture()
{
    I2sInstall();
    I2sSetPin();
    xTaskCreatePinnedToCore(AudioCaptureTask, "AudioCaptureTask", 4096, this, 1, nullptr, 1);
}

void AudioCapture::I2sInstall()
{
    _i2s_config.mode                   = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
    _i2s_config.sample_rate            = SAMPLE_RATE;
    _i2s_config.bits_per_sample        = I2S_BITS_PER_SAMPLE_32BIT;
    _i2s_config.channel_format         = I2S_CHANNEL_FMT_ONLY_LEFT;
    _i2s_config.communication_format   = I2S_COMM_FORMAT_STAND_I2S;
    _i2s_config.intr_alloc_flags       = ESP_INTR_FLAG_LEVEL1;
    _i2s_config.dma_buf_count          = 8;
    _i2s_config.dma_buf_len            = bufferLen;
    _i2s_config.use_apll               = true;
    _i2s_config.fixed_mclk             = 0;
    _i2s_config.mclk_multiple          = I2S_MCLK_MULTIPLE_256;
    _i2s_config.bits_per_chan          = I2S_BITS_PER_CHAN_32BIT;

    if (ESP_OK != i2s_driver_install(I2S_PORT, &_i2s_config, 0, NULL))
    {
        Serial.printf("Install I2S driver failed\n");

        return;
    }

    Serial.printf("Install I2S driver success\n");
}

void AudioCapture::I2sSetPin()
{
    _pin_config ={
        .bck_io_num = I2S_MIC_BCK,
        .ws_io_num = I2S_MIC_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_MIC_SD
    };

    if (ESP_OK != i2s_set_pin(I2S_PORT, &_pin_config))
    {
        Serial.printf("I2S set pin failed\n");
        return;
    }
    Serial.printf("I2S set pin success\n");
}

void AudioCapture::AudioCaptureTask(void *pvParameters)
{
    static_cast<AudioCapture*>(pvParameters)->GetAudios();
}

void AudioCapture::GetAudios()
{
    while (true)
    {
        _result = i2s_read(I2S_PORT, _buffer, sizeof(_buffer), &_bytesIn, portMAX_DELAY);
        if (_result == ESP_OK && _bytesIn)
        {
            int samples_read = _bytesIn / sizeof(uint32_t);
            if (samples_read)
            {
                int64_t sum = 0;
                for (int i = 0; i < samples_read; ++i)
                {
                    int32_t val = (int32_t)_buffer[i];
                    sum += val;
                }
                float mean = (float)sum / samples_read;
                Serial.println(mean);
            }
            // Serial.println(samples_read);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}