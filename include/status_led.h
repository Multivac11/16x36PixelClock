#pragma once

#include <Arduino.h>
#include "status_led.h"

#define LED_GPIO_1 19
#define LED_GPIO_2 20
#define LED_ON 1
#define LED_OFF 0

class StatusLed
{
    public:
        StatusLed();
        ~StatusLed() = default;

        enum LedStatusEnum
        {
            STATUS_NORMAL = 0,
            STATUS_ERROR = 1,
        };

        void Begin();
 
    private:
        static void Task(void *);

        void LedStatus();

        void LedOn();

        void LedOff();

        void SetLedStatus(LedStatusEnum status);

        void LedNormal();

    public:

    private:
        LedStatusEnum _led_status = STATUS_NORMAL;
        
};