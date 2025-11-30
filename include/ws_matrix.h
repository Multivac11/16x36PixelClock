#pragma once

#include <Arduino.h>
#include <Adafruit_NeoMatrix.h>
#include <memory>

#define kMatrixWidth   2             //宽度
#define kMatrixHeight  2              //高度
#define BRIGHTNESS     50            //默认亮度 0-255
#define BRIGHTNESS_INTERVAL 30        //亮度调节间隔
#define LED_PIN        16              //像素阵列引脚

class WsMatrix 
{
    public:
        static WsMatrix& GetInstance()
        {
            static WsMatrix instance;
            return instance;
        }

        WsMatrix() = default;

        ~WsMatrix() = default;

        void InitWsMatrix();
    
    private:
        static void WsMatrixTask(void *);
        void ShowMatrix();
        void SetMatrix();

    private:
        QueueHandle_t queue_;
        std::unique_ptr<Adafruit_NeoMatrix> matrix_;
};