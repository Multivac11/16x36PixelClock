#include <Arduino.h>
#include "status_led.h"
#include "key.h"
#include "uart_log.h"

StatusLed led(19, 20);
StatusKey keys(39, 40, 41);
UartLog uart(115200, UartLog::SERIAL_KEY, &keys);

void setup() 
{
  led.Begin();
  keys.Begin();
  uart.Begin();
}

void loop() 
{

}