#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT4x.h"

#define I2C_SDA 48
#define I2C_SCL 47

class Sht40
{
    public:
        static Sht40& GetInstance()
        {
            static Sht40 instance;
            return instance;
        }

        Sht40() = default;
        ~Sht40() = default;

        struct EnvParamsStruct
        {
            sensors_event_t temperature;
            sensors_event_t humidity;
        };

        void InitSht40();
        bool Available();
        EnvParamsStruct ReadEnvParams();
    private:

        static void GetEnvParamsTask(void *);

        void GetEnvParams();

    private:
        Adafruit_SHT4x sht40_ = Adafruit_SHT4x();
        EnvParamsStruct env_params_;
        QueueHandle_t queue_;
};