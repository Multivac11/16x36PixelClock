#pragma once

#include <Arduino.h>
#include <Adafruit_NeoMatrix.h>
#include <memory>

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