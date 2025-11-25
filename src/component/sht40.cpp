#include "sht40.h"

void Sht40::InitSht40()
{
    Wire.begin(I2C_SDA, I2C_SCL, 100000);
    if (!sht40_.begin(&Wire)) 
    {
        Serial.printf("Couldn't find SHT40 sensor!\n");
    }
    Serial.printf("Found SHT40 sensor\n");

    sht40_.setPrecision(SHT4X_HIGH_PRECISION);
    sht40_.setHeater(SHT4X_HIGH_HEATER_1S);

    queue_ = xQueueCreate(1, sizeof(EnvParamsStruct));
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
        sht40_.getEvent(&env_params_.temperature, &env_params_.humidity);
        xQueueOverwrite(queue_, &env_params_);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

bool Sht40::Available()
{
    return xQueueReceive(queue_, &env_params_, 0) == pdTRUE;
}

Sht40::EnvParamsStruct Sht40::ReadEnvParams()
{
    return env_params_;
}