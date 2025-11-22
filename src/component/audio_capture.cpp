#include "audio_capture.h"

void AudioCapture::InitAudioCapture()
{
    I2sInstall();
    I2sSetPin();
}

void AudioCapture::I2sInstall()
{
    _i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 16,
        .dma_buf_len = bufferLen,
        .use_apll = false 
    };

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