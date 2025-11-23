#pragma once

#include <Arduino.h>
#include "status_led.h"
#include "network.h"

class StatusLed
{
    public:
        static StatusLed& GetInstance()
        {
            static StatusLed instance;
            return instance;
        }

        StatusLed(uint8_t led_pin1 = 19,uint8_t led_pin2 = 20);

        ~StatusLed() = default;

        enum NetworkLedStatusEnum
        {
            STATUS_NETWOIRK_OFFLINE = 0,
            STATUS_NETWOIRK_ONLINE = 1,
            STATUS_NETWOIRK_SCANING = 2,
        };

        enum SystemLedStatusEnum
        {
            STATUS_SYSTEM_NORMAL = 0,
            STATUS_SYSTEM_ERROR = 1,
        };

        void InitStatusLed();
 
    private:
        static void SetNetworkStatusTask(void *);

        static void SetSystemStatusTask(void *);

        static void GetStatusTask(void *);

        void NetworkLedStatus();

        void SystemLedStatus();

        void GetStatus();

        void LedOn(uint8_t led_pin);

        void LedOff(uint8_t led_pin);

        void SetLedStatus();

        void NetworkScanning();

        void NetworkAPmode();

        void NetworkOnline();

        void SystemNormal();

        void SystemError();

    private:
        uint8_t _led_pin[2];
        NetworkLedStatusEnum _network_led_status = STATUS_NETWOIRK_SCANING;
        SystemLedStatusEnum _system_led_status = STATUS_SYSTEM_NORMAL;
        
};