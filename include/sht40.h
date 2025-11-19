#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT4x.h"

#define I2C_SDA 48
#define I2C_SCL 47

class Sht40
{
    public:
        Sht40();
        ~Sht40() = default;

        struct EnvParamsStruct
        {
            sensors_event_t temperature;
            sensors_event_t humidity;
        };

        void Begin();
        bool Available();
        EnvParamsStruct ReadEnvParams();
    private:

        static void GetEnvParamsTask(void *);

        void GetEnvParams();

    private:
        Adafruit_SHT4x _sht40 = Adafruit_SHT4x();
        EnvParamsStruct _env_params;
        QueueHandle_t _queue;
};