#include "ws_matrix.h"

void WsMatrix::InitWsMatrix() 
{
    Serial.println("WS Matrix Init");
    matrix_ = std::unique_ptr<Adafruit_NeoMatrix>(
                    new Adafruit_NeoMatrix(
                        kMatrixWidth, kMatrixHeight, LED_PIN,
                        NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                        NEO_GRB + NEO_KHZ800));

    matrix_->begin();
    matrix_->setBrightness(2);
    matrix_->clear();
    matrix_->fillScreen(matrix_->Color(255, 255, 255));
    matrix_->show();
    Serial.println("WS Matrix Init OK!");
    xTaskCreatePinnedToCore(WsMatrixTask, "WsMatrixTask", 2048, this, 1, nullptr, 1);
}

void WsMatrix::WsMatrixTask(void *pvParameters)
{
    static_cast<WsMatrix*>(pvParameters)->ShowMatrix();
}

void WsMatrix::ShowMatrix()
{
    while (true)
    {
        // Serial.println("WS Matrix");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}