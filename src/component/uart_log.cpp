#include "uart_log.h"

UartLog::UartLog(uint32_t baudrate, SerialModuleEnum module,StatusKey* k)
{
    Serial.begin(baudrate);
    _serial_modlue = module;
    _key = k;
}

void UartLog::Begin()
{
    xTaskCreatePinnedToCore(SerialTask, "SerialTask", 2048, this, 1, nullptr, 1);
}

void UartLog::SerialTask(void *pvParameters)
{
    static_cast<UartLog*>(pvParameters)->SerialPrint();
}

void UartLog::SerialPrint()
{
    while(true)
    {
        PrintSerial(_serial_modlue);
    }
}

void UartLog::PrintSerial(SerialModuleEnum module)
{
    if(module == SERIAL_NONE)
    {
        HelloWorld();
    }
    else if(module == SERIAL_KEY)
    {
        PrintKeyStatus();
    }
}

void UartLog::HelloWorld()
{
    Serial.println("Hello World!\n");
    vTaskDelay(pdMS_TO_TICKS(500));
}

void UartLog::PrintKeyStatus()
{
    if (_key->Available())
    {
        auto ev = _key->Read();
        uint8_t s = ev.key[2] == StatusKey::KEY_SHORT   ? 1 :
                        ev.key[2] == StatusKey::KEY_LONG    ? 2  :
                        ev.key[2] == StatusKey::KEY_PRESSING? 3 : 4;
        if (s) 
        {
            Serial.printf("Key3:%d\n", s);
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}