#include <Arduino.h>
#include "status_led.h"
#include "key.h"
#include "uart_log.h"
#include "sht40.h"

Sht40 sht40;
StatusLed led(19, 20);
StatusKey keys(39, 40, 41);
UartLog uart(115200, UartLog::SERIAL_ENV, &keys, &sht40);

void setup() 
{
  led.Begin();
  keys.Begin();
  sht40.Begin();
  uart.Begin();
}

void loop() 
{

}