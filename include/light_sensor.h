#pragma once

#include <Arduino.h>

#define CONVERSIONS_PER_PIN 8

class LightSensor 
{
    public:
        static LightSensor& GetInstance()
        {
            static LightSensor instance;
            return instance;
        }

        LightSensor() = default;
        ~LightSensor() = default;

        void InitLightSensor();

    private:
        static void LightSensorTask(void *);

        void GetLightValue();

    private:
        uint8_t _adc_pins[1] = {1};
        uint8_t _adc_pins_count = 1;
        volatile bool _adc_done = false;
        // adc_continuous_result_t *result = NULL;
};