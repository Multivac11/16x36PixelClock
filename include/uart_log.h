#pragma once

#include <Arduino.h>
#include "uart_log.h"
#include "key.h"
#include "sht40.h"

class UartLog 
{
    public:
        enum SerialModuleEnum
        {
            SERIAL_NONE = 0,
            SERIAL_KEY = 1,
            SERIAL_LED = 2,
            SERIAL_ENV = 3,
        };

        UartLog(uint32_t baudrate, SerialModuleEnum serial_module,StatusKey* k,Sht40* sht40);
        ~UartLog() = default;

        void InitUartLog();          // 启动任务

    private:
        static void SerialTask(void *); // 静态入口
        void SerialPrint();
        void PrintSerial(SerialModuleEnum module);
        void HelloWorld();
        void PrintKeyStatus();
        void PrintEnvParams();

    private:
        StatusKey* _key;
        Sht40* _sht40;
        SerialModuleEnum _serial_modlue;
};