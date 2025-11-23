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
        if (_system_led_status == STATUS_SYSTEM_NORMAL)
        {
            SystemNormal();
        }
        else if (_system_led_status == STATUS_SYSTEM_ERROR)
        {
            SystemError();
        }
    }
}

void StatusLed::NetworkLedStatus()
{   
    while(true)
    {
        if (_network_led_status == STATUS_NETWOIRK_SCANING)
        {
            NetworkScanning();
        }
        else if (_network_led_status == STATUS_NETWOIRK_ONLINE)
        {
            NetworkOnline();
        }
        else if (_network_led_status == STATUS_NETWOIRK_OFFLINE)
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
                _network_led_status = STATUS_NETWOIRK_ONLINE;
            }
            else if ((net_work_info.status == WL_DISCONNECTED) || (net_work_info.status == WL_NO_SHIELD) && net_work_info.mode == WIFI_MODE_STA)
            {
                _network_led_status = STATUS_NETWOIRK_SCANING;
            }
            else if ((net_work_info.status == WL_DISCONNECTED) || (net_work_info.status == WL_NO_SHIELD) && net_work_info.mode == WIFI_MODE_AP)
            {
                _network_led_status = STATUS_NETWOIRK_OFFLINE;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void StatusLed::SystemNormal()
{
    LedOn(_led_pin[1]);
    vTaskDelay(pdMS_TO_TICKS(200));
    LedOff(_led_pin[1]);
    vTaskDelay(pdMS_TO_TICKS(200));
}

void StatusLed::SystemError()
{
    LedOn(_led_pin[1]);
    vTaskDelay(pdMS_TO_TICKS(50));
    LedOff(_led_pin[1]);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void StatusLed::NetworkScanning()
{
    LedOn(_led_pin[0]);
    vTaskDelay(pdMS_TO_TICKS(50));
    LedOff(_led_pin[0]);
    vTaskDelay(pdMS_TO_TICKS(50));
}

void StatusLed::NetworkAPmode()
{
    LedOn(_led_pin[0]);
    vTaskDelay(pdMS_TO_TICKS(150));
    LedOff(_led_pin[0]);
    vTaskDelay(pdMS_TO_TICKS(150));
}

void StatusLed::NetworkOnline()
{
    LedOn(_led_pin[0]);
    vTaskDelay(pdMS_TO_TICKS(1200));
    LedOff(_led_pin[0]);
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