#include <Arduino.h>
#include "status_led.h"
#include "key.h"

StatusLed led;
StatusKey keys;

TaskHandle_t SerialTaskHandle = NULL;

void SerialTask(void *pvParameters) 
{
    while (1) 
    {
      if (keys.Available())
      {
        auto ev = keys.Read();
           uint8_t s = ev.key[2] == StatusKey::KEY_SHORT   ? 1 :
                        ev.key[2] == StatusKey::KEY_LONG    ? 2  :
                        ev.key[2] == StatusKey::KEY_PRESSING? 3 : 4;
            if (s) Serial.printf("Key3:%d\n", s);
      }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup() 
{
  Serial.begin(115200);
  led.Begin();
  keys.Begin();

  // xTaskCreate(SerialTask, "SerialTask", 2048, NULL, 1, &SerialTaskHandle);
}

void loop() 
{
  // Serial.printf("Hello,world!\n");
}