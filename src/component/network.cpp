#include "network.h"

NetWork::NetWork() 
{
    networkMutex_ = nullptr;
    queue_ = nullptr;
}

NetWork::~NetWork() 
{
    if (networkMutex_ != nullptr) 
    {
        vSemaphoreDelete(networkMutex_);
    }
    if (queue_ != nullptr) 
    {
        vQueueDelete(queue_);
    }
}

void NetWork::InitNetWork()
{
    Serial.println("初始化网络...");
    
    networkMutex_ = xSemaphoreCreateMutex();
    if (networkMutex_ == nullptr) 
    {
        Serial.println("错误：无法创建网络互斥锁");
        return;
    }
    
    server_ = std::unique_ptr<WebServer>(new WebServer(80));
    
    LoadWiFiConfig();
    
    queue_ = xQueueCreate(1, sizeof(NetWorkInfoStruct));
    if (queue_ == nullptr) 
    {
        Serial.println("错误：无法创建网络状态队列");
        return;
    }
    
    xTaskCreatePinnedToCore(NetWorkTask, "NetWorkTask", 8192, this, 1, nullptr, 0);
    Serial.println("创建NetWorkTask");
    
    xTaskCreatePinnedToCore(ConnectWorkTask, "ConnectWorkTask", 8192, this, 1, nullptr, 0);
    Serial.println("创建ConnectWorkTask");
    
    Serial.println("网络初始化完成");
}

void NetWork::SetScanning(bool value) 
{
    if (xSemaphoreTake(networkMutex_, portMAX_DELAY) == pdTRUE) 
    {
        scanning_ = value;
        xSemaphoreGive(networkMutex_);
    }
}

void NetWork::SetApStarted(bool value) 
{
    if (xSemaphoreTake(networkMutex_, portMAX_DELAY) == pdTRUE) 
    {
        apStarted_ = value;
        xSemaphoreGive(networkMutex_);
    }
}

bool NetWork::GetScanning() 
{
    bool value = false;
    if (xSemaphoreTake(networkMutex_, portMAX_DELAY) == pdTRUE) 
    {
        value = scanning_;
        xSemaphoreGive(networkMutex_);
    }
    return value;
}

bool NetWork::GetApStarted() 
{
    bool value = false;
    if (xSemaphoreTake(networkMutex_, portMAX_DELAY) == pdTRUE) 
    {
        value = apStarted_;
        xSemaphoreGive(networkMutex_);
    }
    return value;
}

void NetWork::IncrementConnectCount() 
{
    if (xSemaphoreTake(networkMutex_, portMAX_DELAY) == pdTRUE) 
    {
        connect_count_++;
        xSemaphoreGive(networkMutex_);
    }
}

void NetWork::ResetConnectCount() 
{
    if (xSemaphoreTake(networkMutex_, portMAX_DELAY) == pdTRUE) 
    {
        connect_count_ = 0;
        xSemaphoreGive(networkMutex_);
    }
}

int NetWork::GetConnectCount() 
{
    int value = 0;
    if (xSemaphoreTake(networkMutex_, portMAX_DELAY) == pdTRUE) 
    {
        value = connect_count_;
        xSemaphoreGive(networkMutex_);
    }
    return value;
}

void NetWork::NetWorkTask(void *pvParameters)
{
    static_cast<NetWork*>(pvParameters)->GetNetWorkInfo();
}

void NetWork::ConnectWorkTask(void *pvParameters)
{
    static_cast<NetWork*>(pvParameters)->StartConection();
}

void NetWork::DelayedAPCloseTask(void *pvParameters)
{
    NetWork* network = static_cast<NetWork*>(pvParameters);
    
    Serial.println("延迟AP关闭任务启动，等待10秒...");
    vTaskDelay(pdMS_TO_TICKS(10000));
    
    network->SetApStarted(false);

    WiFi.softAPdisconnect(true);
    Serial.println("AP模式已关闭");
    
    vTaskDelete(nullptr);
}

void NetWork::GetNetWorkInfo()
{
    Serial.println("开始获取网络信息...");
    
    while (true) 
    {
        wl_status_t currentStatus = WiFi.status();
        
        if (xSemaphoreTake(networkMutex_, portMAX_DELAY) == pdTRUE) 
        {
            net_work_info_.status = currentStatus;
            net_work_info_.mode = WiFi.getMode();
            xQueueOverwrite(queue_, &net_work_info_);
            xSemaphoreGive(networkMutex_);
        }
        
        SafeHandleWebClient();
        
        if (currentStatus == WL_CONNECTED) 
        {
            SetScanning(false);
        } 
        else 
        {
            if (!GetScanning() && !GetApStarted()) 
            {
                Serial.println("WiFi断开连接，准备重新扫描...");
                SafeWiFiReset();
                SetScanning(true);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void NetWork::SafeHandleWebClient()
{
    if ((GetApStarted() || WiFi.status() == WL_CONNECTED) && server_ != nullptr) 
    {
        server_->handleClient();
    }
}

void NetWork::SafeWiFiReset()
{
    Serial.println("执行WiFi重置...");
    
    if (server_ != nullptr) 
    {
        server_->stop();
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    

    WiFi.disconnect();
    vTaskDelay(pdMS_TO_TICKS(200));
    WiFi.mode(WIFI_OFF);
    vTaskDelay(pdMS_TO_TICKS(200));
    
    Serial.println("WiFi重置完成");
}

void NetWork::StartConection()
{
    Serial.println("开始连接管理...");
    SetScanning(true);
    while (true) 
    {
        if (GetScanning()) 
        {
            PerformSafeScan();
        }
        
        if (GetApStarted() && GetConnectCount() != 0) 
        {
            StartSafeAPMode();
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void NetWork::PerformSafeScan()
{
    Serial.println("执行WiFi扫描...");
    
    WiFi.disconnect();
    vTaskDelay(pdMS_TO_TICKS(100));
    WiFi.mode(WIFI_STA);
    vTaskDelay(pdMS_TO_TICKS(200));
    
    WiFi.scanDelete();
    vTaskDelay(pdMS_TO_TICKS(100));
    
    int scanResult = WiFi.scanNetworks();
    Serial.printf("扫描结果: %d 个网络\n", scanResult);
    
    if (scanResult > 0 && scanResult < 100) 
    {
        bool hit = false;
        
        for (int i = 0; i < scanResult && i < 50; ++i) 
        {
            String seen = WiFi.SSID(i);
            int rssi = WiFi.RSSI(i);
            
            if (rssi > -80) 
            {
                for (auto& r : netList_) 
                {
                    if (seen == r.ssid) 
                    {
                        Serial.printf("发现已保存WiFi: %s (信号: %d dBm)\n", seen.c_str(), rssi);
                        ssid_ = r.ssid;
                        password_ = r.pass;
                        bool connected = ConnectToWiFi();
                        if (connected) 
                        {
                            hit = true;
                        }
                        break;
                    }
                }
            }
            if (hit) break;
        }
        
        if (!hit) 
        {
            Serial.println("未找到已保存的WiFi网络");
            IncrementConnectCount();
        }
    } 
    else 
    {
        Serial.println("扫描失败或结果异常");
        IncrementConnectCount();
    }
    
    WiFi.scanDelete();
    
    if (GetConnectCount() >= 3) 
    {
        SetApStarted(true);
        SetScanning(false);
        Serial.println("达到扫描次数上限，启动AP模式");
    }
    
    vTaskDelay(pdMS_TO_TICKS(500));
}

void NetWork::StartSafeAPMode()
{
    Serial.println("启动AP模式...");
    
    ResetConnectCount();
    
    WiFi.disconnect();
    vTaskDelay(pdMS_TO_TICKS(100));
    WiFi.mode(WIFI_AP);
    vTaskDelay(pdMS_TO_TICKS(200));
    
    if (!WiFi.softAP("PixelClock_Config", "12345678")) 
    {
        Serial.println("AP启动失败！");
        SetApStarted(false);
        SetScanning(true);
        return;
    }
    
    Serial.print("AP IP地址: ");
    Serial.println(WiFi.softAPIP());
    
    // 设置Web服务器路由
    server_->on("/", HTTP_GET, [this]() { HandleRoot(); });
    server_->on("/config", HTTP_GET, [this]() { HandleConfig(); });
    server_->on("/scan", HTTP_GET, [this]() { HandleScan(); });
    server_->on("/success", HTTP_GET, [this]() { HandleSuccess(); });
    server_->on("/failure", HTTP_GET, [this]() { HandleFailure(); });
    server_->on("/status", HTTP_GET, [this]() { HandleStatus(); });
    
    server_->begin();
    Serial.println("Web服务器已启动");
    
    // AP模式维护循环
    uint32_t apStartTime = millis();
    while (GetApStarted()) 
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 5分钟超时保护
        if (millis() - apStartTime > 300000) 
        {
            Serial.println("AP模式超时，自动关闭");
            SetApStarted(false);
            SetScanning(true);
            break;
        }
    }
}

// 连接WiFi
bool NetWork::ConnectToWiFi()
{
    Serial.printf("尝试连接 WiFi: %s\n", ssid_.c_str());
    
    WiFi.begin(ssid_.c_str(), password_.c_str());
    Serial.printf("连接 %s ...\n", ssid_.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 4) 
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        Serial.print('.');
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) 
    {
        Serial.println("\n连接成功！");
        Serial.println("IP地址: " + WiFi.localIP().toString());
        
        // 更新状态
        SetScanning(false);
        ResetConnectCount();
        
        // 保存WiFi配置
        AddWiFi(ssid_, password_);
        
        // 延迟关闭AP，让用户看到成功页面
        Serial.println("启动延迟AP关闭任务...");
        xTaskCreatePinnedToCore(DelayedAPCloseTask, "DelayedAPClose", 2048, this, 1, nullptr, 0);
        
        return true;
    } 
    else 
    {
        Serial.println("\n连接失败");
        return false;
    }
}

// Web处理函数
void NetWork::HandleRoot()
{
    server_->send_P(200, "text/html", WebPages::CONFIG_PAGE);
}

void NetWork::HandleConfig()
{
    if (server_->hasArg("ssid") && server_->hasArg("password")) 
    {
        ssid_ = server_->arg("ssid");
        password_ = server_->arg("password");
        
        Serial.printf("接收到WiFi配置 - SSID: %s\n", ssid_.c_str());
        
        bool connectSuccess = ConnectToWiFi();
        
        if (connectSuccess) 
        {
            server_->send(200, "application/json", "{\"status\":\"success\"}");
            
        } 
        else 
        {
            server_->send(200, "application/json", "{\"status\":\"failure\"}");
        }
    } 
    else 
    {
        server_->send(400, "application/json", "{\"error\":\"缺少SSID或密码\"}");
    }
}

void NetWork::HandleScan()
{
    // 扫描WiFi网络
    int networksFound = WiFi.scanNetworks();
    if (networksFound < 0) 
    {
        server_->send(500, "application/json", "[]");
        return;
    }
    
    String json = "[";
    for (int i = 0; i < networksFound; ++i) 
    {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + EscapeJsonString(WiFi.SSID(i)) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"encryption\":" + String(WiFi.encryptionType(i));
        json += "}";
    }
    json += "]";
    
    server_->send(200, "application/json", json);
    
    WiFi.scanDelete();
}

void NetWork::HandleSuccess()
{
    String successPage = FPSTR(WebPages::SUCCESS_PAGE);
    successPage.replace("%s", WiFi.localIP().toString());
    server_->send(200, "text/html", successPage);
}

void NetWork::HandleFailure()
{
    server_->send_P(200, "text/html", WebPages::FAILURE_PAGE);
    vTaskDelay(pdMS_TO_TICKS(200));
    WiFi.mode(WIFI_AP);
}

void NetWork::HandleStatus()
{
    String json = "{";
    json += "\"connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"ssid\":\"" + EscapeJsonString(WiFi.SSID()) + "\"";
    json += "}";
    
    server_->send(200, "application/json", json);
}

// 工具函数
bool NetWork::Available()
{
    return xQueueReceive(queue_, &net_work_info_, 0) == pdTRUE;
}

NetWork::NetWorkInfoStruct NetWork::ReadNetWorkInfo()
{
    return net_work_info_;
}

bool NetWork::LoadWiFiConfig()
{
    netList_.clear();
    prefs_.begin(NS, false);
    String json = prefs_.getString(KEY_LIST, "[]");
    prefs_.end();

    Serial.print("从NVS读取的WiFi配置: ");
    Serial.println(json);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) 
    {
        Serial.print("JSON解析错误: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject o : arr) 
    {
        netList_.push_back({o["s"], o["p"]});
    }
    
    Serial.printf("加载了 %d 个WiFi配置\n", netList_.size());
    return !netList_.empty();
}

bool NetWork::SaveWiFiConfig()
{
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (auto& r : netList_) 
    {
        JsonObject obj = arr.add<JsonObject>();
        obj["s"] = r.ssid;
        obj["p"] = r.pass;
    }

    String json;
    serializeJson(doc, json);
    
    prefs_.begin(NS, false);
    prefs_.putString(KEY_LIST, json);
    prefs_.end();
    
    Serial.println("WiFi配置已保存");
    return true;
}

void NetWork::AddWiFi(const String& ssid, const String& pass)
{
    for (auto& r : netList_) 
    {
        if (r.ssid == ssid) 
        { 
            r.pass = pass;
            SaveWiFiConfig();
            return; 
        }
    }
    
    if (netList_.size() >= MAX_NETS) 
    {
        netList_.erase(netList_.begin());
    }
    
    netList_.push_back({ssid, pass});
    SaveWiFiConfig();
    
    Serial.printf("已添加WiFi: %s\n", ssid.c_str());
}

String NetWork::EscapeJsonString(const String& input)
{
    String output;
    output.reserve(input.length());
    
    for (size_t i = 0; i < input.length(); ++i) 
    {
        char c = input.charAt(i);
        switch (c) 
        {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: 
                if (c >= 32 && c <= 126) 
                {
                    output += c;
                }
                break;
        }
    }
    
    return output;
}