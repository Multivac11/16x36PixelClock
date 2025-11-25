#include "light_sensor.h"

void LightSensor::InitLightSensor()
{
    analogReadResolution(12);
    queue_ = xQueueCreate(1, sizeof(uint16_t));
    xTaskCreatePinnedToCore(LightSensorTask, "LightSensorTask", 2048, this, 1, nullptr, 1);
}

void LightSensor::LightSensorTask(void *pvParameters)
{
    static_cast<LightSensor*>(pvParameters)->GetLightValue();
}

void LightSensor::GetLightValue()
{
    const int N = 8;
    static int buf[N] = {0};
    static int sum = 0;
    static uint8_t idx = 0;

    while (true)
    {
        // 滑动平均滤波
        int raw = analogRead(1);
        sum += raw - buf[idx];
        buf[idx] = raw;
        idx = (idx + 1) % N;
        light_value_ = sum / N;
        // Serial.println(light_value_);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

bool LightSensor::Available()
{
    return xQueueReceive(queue_, &light_value_, 0) == pdTRUE;
}

uint16_t LightSensor::ReadLightValue()
{
    return light_value_;
}