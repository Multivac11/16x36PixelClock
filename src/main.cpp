#include <Arduino.h>
#include "status_led.h"
#include "key.h"
#include "uart_log.h"
#include "sht40.h"
#include "audio_capture.h"

Sht40 sht40;
AudioCapture audio_capture;
StatusLed led(19, 20);
StatusKey keys(39, 40, 41);
UartLog uart(115200, UartLog::SERIAL_KEY, &keys, &sht40);

void setup() 
{
  led.InitStatusLed();
  keys.InitKeys();
  sht40.InitSht40();
  audio_capture.InitAudioCapture();
  uart.InitUartLog();
}

void loop() 
{

}