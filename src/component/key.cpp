#include "key.h"

constexpr uint32_t SCAN_MS = 20;

StatusKey::StatusKey(uint8_t p1, uint8_t p2, uint8_t p3, uint32_t lm): longMs_(lm), queue_(nullptr)
{
    key_pin_[0] = p3;
    key_pin_[1] = p2;
    key_pin_[2] = p1;
}

void StatusKey::InitKeys()
{
     for (int i = 0; i < 3; ++i)
     {
        pinMode(key_pin_[i], INPUT_PULLUP);
     } 
     
    queue_ = xQueueCreate(1, sizeof(Event));
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
        bool down = digitalRead(key_pin_[i]) == LOW;

        if (down) 
        {                            // 检测到按下
            if (!buffer_[i].act)
            {                      // 如果是刚按下
                buffer_[i].act = true;
                buffer_[i].t   = now;
            }
        } 
        else 
        {                            // 检测到未按下
            if (buffer_[i].act) 
            {                        // 检测到按下刚松开
                uint32_t dt = now - buffer_[i].t;
                ev_.key[i] = (dt >= longMs_) ? KEY_LONG : KEY_SHORT;
                buffer_[i].act = false;
            } 
            else
            {
                ev_.key[i] = KEY_NONE;
                buffer_[i].t   = now;
            }
        }
    }

    bool allZero = true;            // 判断当按键发生了变化时才写入队列
    for (int i = 0; i < 3; ++i)
    {
        if (ev_.key[i] != KEY_NONE) 
        { 
            allZero = false; 
            break; 
        }
    }

    if (!allZero) 
    {
        xQueueOverwrite(queue_, &ev_);
    }

    vTaskDelay(pdMS_TO_TICKS(SCAN_MS));
}

bool StatusKey::Available()
{
    return xQueueReceive(queue_, &ev_, 0) == pdTRUE;
}

StatusKey::Event StatusKey::Read()
{
    return ev_;
}