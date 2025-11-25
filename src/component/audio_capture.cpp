#include "audio_capture.h"

void AudioCapture::InitAudioCapture()
{
    I2sInstall();
    I2sSetPin();
    xTaskCreatePinnedToCore(AudioCaptureTask, "AudioCaptureTask", 4096, this, 1, nullptr, 1);
}

void AudioCapture::I2sInstall()
{
    i2s_config_.mode                   = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
    i2s_config_.sample_rate            = SAMPLE_RATE;
    i2s_config_.bits_per_sample        = I2S_BITS_PER_SAMPLE_32BIT;
    i2s_config_.channel_format         = I2S_CHANNEL_FMT_ONLY_LEFT;
    i2s_config_.communication_format   = I2S_COMM_FORMAT_STAND_I2S;
    i2s_config_.intr_alloc_flags       = ESP_INTR_FLAG_LEVEL1;
    i2s_config_.dma_buf_count           = 8;
    i2s_config_.dma_buf_len          = bufferLen;
    i2s_config_.use_apll               = true;
    i2s_config_.fixed_mclk             = 0;
    i2s_config_.mclk_multiple          = I2S_MCLK_MULTIPLE_256;
    i2s_config_.bits_per_chan          = I2S_BITS_PER_CHAN_32BIT;

    if (ESP_OK != i2s_driver_install(I2S_PORT, &i2s_config_, 0, NULL))
    {
        Serial.printf("Install I2S driver failed\n");

        return;
    }

    Serial.printf("Install I2S driver success\n");
}

void AudioCapture::I2sSetPin()
{
    pin_config_ ={
        .bck_io_num = I2S_MIC_BCK,
        .ws_io_num = I2S_MIC_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_MIC_SD
    };

    if (ESP_OK != i2s_set_pin(I2S_PORT, &pin_config_))
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
        result_ = i2s_read(I2S_PORT, buffer_, sizeof(buffer_), &bytesIn_, portMAX_DELAY);
        if (result_ == ESP_OK && bytesIn_)
        {
            int samples_read = bytesIn_ / sizeof(uint32_t);
            if (samples_read)
            {
                int64_t sum = 0;
                for (int i = 0; i < samples_read; ++i)
                {
                    int32_t val = (int32_t)buffer_[i];
                    sum += val;
                }
                float mean = (float)sum / samples_read;
                // Serial.println(mean);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}