#pragma once

#include <Arduino.h>
#include "key.h"

class StatusKey
{
    public:
        StatusKey(uint8_t key_pin1=41, uint8_t key_pin2=40, uint8_t key_pin3=39,uint32_t longMs = 800);
        ~StatusKey() = default;

        enum KeyStatusEnum
        {
            KEY_NONE = 0,
            KEY_SHORT = 1,
            KEY_LONG = 2,
            KEY_PRESSING = 3,
        };

        struct Event 
        {
            KeyStatusEnum key[3];
        };

        void InitKeys();          // 启动任务
        bool Available(); // 是否有新事件
        Event Read();          // 取出事件

    private:
        static void GetKeyTask(void *); // 静态入口
        void ScanKeys();
        void KeyStatus();

    private:
        uint8_t  _key_pin[3];
        uint32_t _longMs;
        QueueHandle_t _queue;
        Event _ev;               // 缓存
        struct Buf 
        { 
            uint32_t t; 
            bool act; 
        };
        Buf _buffer[3] = {};
};