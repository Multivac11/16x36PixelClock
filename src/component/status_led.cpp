#include "status_led.h"

StatusLed::StatusLed(uint8_t led_gpio1, uint8_t led_gpio2)
{
    led_pin_[0] = led_gpio1;
    led_pin_[1] = led_gpio2;

    for(int i = 0; i < 2; i++)
    {
        pinMode(led_pin_[i], OUTPUT);
        digitalWrite(led_pin_[i], 0);
    }
}

void StatusLed::InitStatusLed()
{
    xTaskCreatePinnedToCore(GetStatusTask, "GetStatusTask", 2048, this, 1, nullptr, 1);
    xTaskCreatePinnedToCore(SetNetworkStatusTask, "SetStatusTask", 2048, this, 1, nullptr, 1);
    xTaskCreatePinnedToCore(SetSystemStatusTask, "SetSystemStatusTask", 2048, this, 1, nullptr, 1);
}

void StatusLed::SetNetworkStatusTask(void *pvParameters)
{
    static_cast<StatusLed*>(pvParameters)->NetworkLedStatus();
}

void StatusLed::SetSystemStatusTask(void *pvParameters)
{
    static_cast<StatusLed*>(pvParameters)->SystemLedStatus();
}

void StatusLed::GetStatusTask(void *pvParameters)
{
    static_cast<StatusLed*>(pvParameters)->GetStatus();
}

void StatusLed::SystemLedStatus()
{
    while(true)
    {
        if (system_led_status_ == STATUS_SYSTEM_NORMAL)
        {
            SystemNormal();
        }
        else if (system_led_status_ == STATUS_SYSTEM_ERROR)
        {
            SystemError();
        }
    }
}

void StatusLed::NetworkLedStatus()
{   
    while(true)
    {
        if (network_led_status_ == STATUS_NETWOIRK_SCANING)
        {
            NetworkScanning();
        }
        else if (network_led_status_ == STATUS_NETWOIRK_ONLINE)
        {
            NetworkOnline();
        }
        else if (network_led_status_ == STATUS_NETWOIRK_OFFLINE)
        {
            NetworkAPmode();
        }
    }
}

void StatusLed::GetStatus()
{
    while(true)
    {
        if(NetWork::GetInstance().Available())
        {
            auto net_work_info = NetWork::GetInstance().ReadNetWorkInfo();

            if (net_work_info.status == WL_CONNECTED)
            {
                network_led_status_ = STATUS_NETWOIRK_ONLINE;
            }
            else if ((net_work_info.status == WL_DISCONNECTED) || (net_work_info.status == WL_NO_SHIELD) && net_work_info.mode == WIFI_MODE_STA)
            {
                network_led_status_ = STATUS_NETWOIRK_SCANING;
            }
            else if ((net_work_info.status == WL_DISCONNECTED) || (net_work_info.status == WL_NO_SHIELD) && net_work_info.mode == WIFI_MODE_AP)
            {
                network_led_status_ = STATUS_NETWOIRK_OFFLINE;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void StatusLed::SystemNormal()
{
    LedOn(led_pin_[1]);
    vTaskDelay(pdMS_TO_TICKS(200));
    LedOff(led_pin_[1]);
    vTaskDelay(pdMS_TO_TICKS(200));
}

void StatusLed::SystemError()
{
    LedOn(led_pin_[1]);
    vTaskDelay(pdMS_TO_TICKS(50));
    LedOff(led_pin_[1]);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void StatusLed::NetworkScanning()
{
    LedOn(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(50));
    LedOff(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void StatusLed::NetworkAPmode()
{
    LedOn(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(150));
    LedOff(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(150));
}

void StatusLed::NetworkOnline()
{
    LedOn(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(1200));
    LedOff(led_pin_[0]);
    vTaskDelay(pdMS_TO_TICKS(1200));
}

void StatusLed::LedOn(uint8_t led_gpio)
{
    digitalWrite(led_gpio, 1);
}

void StatusLed::LedOff(uint8_t led_gpio)
{
    digitalWrite(led_gpio, 0);
}