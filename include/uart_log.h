#pragma once

#include <Arduino.h>
#include "uart_log.h"
#include "key.h"

class UartLog 
{
    public:
        enum SerialModuleEnum
        {
            SERIAL_NONE = 0,
            SERIAL_KEY = 1,
            SERIAL_LED = 2,
        };

        UartLog(uint32_t baudrate, SerialModuleEnum serial_module,StatusKey* k);
        ~UartLog() = default;

        void Begin();          // 启动任务

    private:
        static void SerialTask(void *); // 静态入口
        void SerialPrint();
        void PrintSerial(SerialModuleEnum module);
        void HelloWorld();
        void PrintKeyStatus();

    private:
        StatusKey* _key;
        SerialModuleEnum _serial_modlue;
};