#include <Arduino.h>
#include "status_led.h"
#include "key.h"
#include "uart_log.h"
#include "sht40.h"
#include "audio_capture.h"
#include "network.h"
#include "light_sensor.h"

UartLog uart(115200, UartLog::SERIAL_KEY);

void setup() 
{
  NetWork::GetInstance().InitNetWork();
  LightSensor::GetInstance().InitLightSensor();
  StatusLed::GetInstance().InitStatusLed();
  StatusKey::GetInstance().InitKeys();
  Sht40::GetInstance().InitSht40();
  AudioCapture::GetInstance().InitAudioCapture();
  uart.InitUartLog();
}

void loop()
{

}