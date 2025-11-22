#pragma once

#include <Arduino.h>
#include "status_led.h"

class StatusLed
{
    public:
        StatusLed(uint8_t led_pin1 = 19,uint8_t led_pin2 = 20);
        ~StatusLed() = default;

        enum LedStatusEnum
        {
            STATUS_NORMAL = 0,
            STATUS_ERROR = 1,
        };

        void InitStatusLed();
 
    private:
        static void SetStatusTask(void *);

        void LedStatus();

        void LedOn(uint8_t led_pin);

        void LedOff(uint8_t led_pin);

        void SetLedStatus(LedStatusEnum status);

        void LedNormal();

    private:
        uint8_t _led_pin[2];
        LedStatusEnum _led_status = STATUS_NORMAL;
        
};