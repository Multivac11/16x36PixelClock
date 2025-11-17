#include "status_led.h"

StatusLed::StatusLed()
{
    pinMode(LED_GPIO_1, OUTPUT);
    pinMode(LED_GPIO_2, OUTPUT);
    digitalWrite(LED_GPIO_1, LED_OFF);
    digitalWrite(LED_GPIO_2, LED_OFF);
}

void StatusLed::Begin()
{
    xTaskCreatePinnedToCore(Task, "LedStatus", 2048, this, 1, nullptr, 1);
}

void StatusLed::Task(void *pvParameters)
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
        LedOff();
    }
}

void StatusLed::LedNormal()
{
    LedOn();
    vTaskDelay(pdMS_TO_TICKS(200));
    LedOff();
    vTaskDelay(pdMS_TO_TICKS(200));
}

void StatusLed::LedOn()
{
    digitalWrite(LED_GPIO_1, LED_ON);
}

void StatusLed::LedOff()
{
    digitalWrite(LED_GPIO_1, LED_OFF);
}