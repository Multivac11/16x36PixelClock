#include "status_led.h"

StatusLed::StatusLed(uint8_t led_gpio1, uint8_t led_gpio2)
{
    _led_pin[0] = led_gpio1;
    _led_pin[1] = led_gpio2;

    for(int i = 0; i < 2; i++)
    {
        pinMode(_led_pin[i], OUTPUT);
        digitalWrite(_led_pin[i], 0);
    }
}

void StatusLed::Begin()
{
    xTaskCreatePinnedToCore(SetStatusTask, "SetStatusTask", 2048, this, 1, nullptr, 1);
}

void StatusLed::SetStatusTask(void *pvParameters)
{
    static_cast<StatusLed*>(pvParameters)->LedStatus();
}

void StatusLed::LedStatus()
{   
    while(true)
    {
        SetLedStatus(_led_status);
    }
}

void StatusLed::SetLedStatus(LedStatusEnum status)
{
    _led_status = status;
    if (_led_status == STATUS_NORMAL)
    {
        LedNormal();
    }
    else if (_led_status == STATUS_ERROR)
    {
        LedOff(_led_pin[0]);
    }
}

void StatusLed::LedNormal()
{
    LedOn(_led_pin[0]);
    vTaskDelay(pdMS_TO_TICKS(200));
    LedOff(_led_pin[0]);
    vTaskDelay(pdMS_TO_TICKS(200));
}

void StatusLed::LedOn(uint8_t led_gpio)
{
    digitalWrite(led_gpio, 1);
}

void StatusLed::LedOff(uint8_t led_gpio)
{
    digitalWrite(led_gpio, 0);
}