#include "sht40.h"

void Sht40::InitSht40()
{
    Wire.begin(I2C_SDA, I2C_SCL, 100000);
    if (!_sht40.begin(&Wire)) 
    {
        Serial.printf("Couldn't find SHT40 sensor!\n");
    }
    Serial.printf("Found SHT40 sensor\n");

    _sht40.setPrecision(SHT4X_HIGH_PRECISION);
    _sht40.setHeater(SHT4X_HIGH_HEATER_1S);

    _queue = xQueueCreate(1, sizeof(EnvParamsStruct));
    xTaskCreatePinnedToCore(GetEnvParamsTask, "GetEnvParamsTask", 4096, this, 1, nullptr, 1);
}

void Sht40::GetEnvParamsTask(void *pvParameters)
{
    static_cast<Sht40*>(pvParameters)->GetEnvParams();
}

void Sht40::GetEnvParams()
{
    while(true)
    {
        _sht40.getEvent(&_env_params.temperature, &_env_params.humidity);
        xQueueOverwrite(_queue, &_env_params);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

bool Sht40::Available()
{
    return xQueueReceive(_queue, &_env_params, 0) == pdTRUE;
}

Sht40::EnvParamsStruct Sht40::ReadEnvParams()
{
    return _env_params;
}