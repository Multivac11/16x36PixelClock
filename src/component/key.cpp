#include "key.h"

constexpr uint32_t SCAN_MS = 20;

StatusKey::StatusKey(uint8_t p1, uint8_t p2, uint8_t p3, uint32_t lm): _longMs(lm), _queue(nullptr)
{
    _key_pin[0] = p3;
    _key_pin[1] = p2;
    _key_pin[2] = p1;
}

void StatusKey::Begin()
{
     for (int i = 0; i < 3; ++i)
     {
        pinMode(_key_pin[i], INPUT_PULLUP);
     } 
     
    _queue = xQueueCreate(1, sizeof(Event));
    xTaskCreatePinnedToCore(GetKeyTask, "GetKeyTask", 2048, this, 2, nullptr, 1);
}

void StatusKey::GetKeyTask(void *pvParameters)
{
    static_cast<StatusKey*>(pvParameters)->KeyStatus();
}

void StatusKey::KeyStatus()
{
    while(true)
    {
        ScanKeys();
    }
}

void StatusKey::ScanKeys()
{
    uint32_t now = millis();

    for (int i = 0; i < 3; ++i) 
    {
        bool down = digitalRead(_key_pin[i]) == LOW;

        if (down) 
        {                            // 检测到按下
            if (!_buffer[i].act)
            {                      // 如果是刚按下
                _buffer[i].act = true;
                _buffer[i].t   = now;
            }
        } 
        else 
        {                            // 检测到未按下
            if (_buffer[i].act) 
            {                        // 检测到按下刚松开
                uint32_t dt = now - _buffer[i].t;
                _ev.key[i] = (dt >= _longMs) ? KEY_LONG : KEY_SHORT;
                _buffer[i].act = false;
            } 
            else
            {
                _ev.key[i] = KEY_NONE;
                _buffer[i].t   = now;
            }
        }
    }

    bool allZero = true;            // 判断当按键发生了变化时才写入队列
    for (int i = 0; i < 3; ++i)
    {
        if (_ev.key[i] != KEY_NONE) 
        { 
            allZero = false; 
            break; 
        }
    }

    if (!allZero) 
    {
        xQueueOverwrite(_queue, &_ev);
    }

    vTaskDelay(pdMS_TO_TICKS(SCAN_MS));
}

bool StatusKey::Available()
{
    return xQueueReceive(_queue, &_ev, 0) == pdTRUE;
}

StatusKey::Event StatusKey::Read()
{
    return _ev;
}