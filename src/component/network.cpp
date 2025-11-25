#include "network.h"

NetWork::NetWork() : server_(80)
{
        // 配置页面HTML内容
    configPage_ = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
            <title>ESP32 WiFi配置</title>
            <style>
                body { font-family: Arial; max-width:400px; margin:auto; padding:20px; text-align:center; }
                form { background:#f9f9f9; border:1px solid #ddd; padding:20px; border-radius:5px; }
                select, input[type=password], button { width:100%; padding:10px; margin:8px 0; box-sizing:border-box; }
                button { background:#4CAF50; color:white; border:none; border-radius:4px; cursor:pointer; }
                button:hover { background:#45a049; }
            </style>
        </head>
        <body>
            <h1>ESP32 WiFi配置</h1>
            <form action="/config" method="get">
                <select name="ssid" required>
                    <option value="">-- 请选择WiFi --</option>)rawliteral";
            configPage_ += MakeOptionList();
            configPage_ += R"rawliteral(
                </select>
                <input type="password" name="password" placeholder="WiFi密码" required>
                <button type="submit">连接</button>
            </form>
        </body>
        </html>)rawliteral";

    // 连接成功页面
    successPage_ = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
            <title>连接成功</title>
            <style>
                body { font-family: Arial, sans-serif; max-width: 400px; margin: 0 auto; padding: 20px; text-align: center; }
                .success { color: green; }
            </style>
        </head>
        <body>
            <h1 class="success">连接成功</h1>
            <p>ESP32已成功连接到WiFi网络。</p>
            <p>AP将在5秒后关闭...</p>
        </body>
        </html>
    )rawliteral";

    // 连接失败页面
    failurePage_ = R"rawliteral(             
        <!DOCTYPE html>
        <html>
        <head>
            <title>连接失败</title>
            <style>
                body { font-family: Arial, sans-serif; max-width: 400px; margin: 0 auto; padding: 20px; text-align: center; }
                .error { color: red; }
            </style>
        </head>
        <body>
            <h1 class="error">连接失败</h1>
            <p>无法连接到指定的WiFi网络，请重新配置。</p>
            <a href="/">返回配置页面</a>
        </body>
        </html>
    )rawliteral";
}

void NetWork::InitNetWork()
{
    Serial.println("初始化网络...");
    LoadWiFiConfig();
    queue_ = xQueueCreate(1, sizeof(NetWorkInfoStruct));
    xTaskCreatePinnedToCore(NetWorkTask, "NetWorkTask", 4096, this, 1, nullptr, 1);
    xTaskCreatePinnedToCore(ConnectWorkTask, "ConnectWorkTask", 4096 , this, 1, nullptr, 1);
}

void NetWork::NetWorkTask(void *pvParameters)
{
    static_cast<NetWork*>(pvParameters)->GetNetWorkInfo();
}

void NetWork::ConnectWorkTask(void *pvParameters)
{
    static_cast<NetWork*>(pvParameters)->StartConection();
}

void NetWork::GetNetWorkInfo()
{
     while (true) 
     {
        net_work_info_.status = WiFi.status();
        net_work_info_.mode = WiFi.getMode();
        xQueueOverwrite(queue_, &net_work_info_);
        
        if (net_work_info_.status == WL_CONNECTED)
        {

        } 
        else if (net_work_info_.status == WL_DISCONNECTED) 
        {
            if (!scanning_ && !apStarted_) 
            {
                Serial.println("Wi-Fi 断线，重新进入周期扫描...");
                WiFi.mode(WIFI_OFF);
                vTaskDelay(pdMS_TO_TICKS(200));
                scanning_ = true;
            }
            server_.handleClient();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void NetWork::StartConection()
{
    while (true) 
    {
        while(scanning_)
        {
            WiFi.mode(WIFI_STA);
            vTaskDelay(100);
            int scanRes = WiFi.scanNetworks();
            if(scanRes >= 0)
            {
                Serial.print("当前扫描到热点数量: ");
                Serial.println(scanRes);
                connect_count_ ++;
                bool hit = false;
                for (int i = 0; i < scanRes; ++i)
                {
                    String seen = WiFi.SSID(i);
                    for (auto& r : netList_) 
                    {
                        if (seen == r.ssid) 
                        {
                            Serial.printf("发现已保存WiFi: %s，尝试连接...\n", seen.c_str());
                            ssid_ = r.ssid;
                            password_ = r.pass;
                            ConnectToWiFi();
                            hit = true;
                            break;
                        }
                    }
                    if (hit) 
                    {
                        break;
                    }
                }
            }
            else if(scanRes == -2)
            {
                Serial.println("扫描失败！");
            }
            
            if (connect_count_ >= 4)
            {
                apStarted_ = true;       
                scanning_ = false;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        if(apStarted_ && connect_count_ != 0)
        {
            Serial.println("本次无匹配，打开配网 Web...");
            connect_count_ = 0;
            StartAPMode();
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

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

    Serial.print("从NVS读出的原始json: ");
    Serial.println(json);

    JsonDocument doc;
    deserializeJson(doc, json);
    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject o : arr)
    {
        netList_.push_back({o["s"], o["p"]});
    }
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
        netList_.pop_back();
    }
    netList_.push_back({ssid, pass});
    SaveWiFiConfig();
}

void NetWork::ConnectToWiFi()
{
    WiFi.begin(ssid_.c_str(), password_.c_str());
    Serial.printf("连接 %s ...\n", ssid_.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) 
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print('.');
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) 
    {
        Serial.println("\n连接成功！IP: " + WiFi.localIP().toString());
        connect_count_ = 0;
        scanning_ = false;
        apStarted_ = false;
        AddWiFi(ssid_, password_);

    }
    else 
    {
        Serial.println("\n连接失败，继续周期扫描...");
    }
}

void NetWork::StartAPMode()
{
    Serial.println("启动AP模式...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_Config", "12345678");
    Serial.print("AP IP地址: ");
    Serial.println(WiFi.softAPIP());

    server_.on("/", [this]() { server_.send(200, "text/html", configPage_); });
    server_.on("/config", [this]() 
    {
        if (server_.hasArg("ssid") && server_.hasArg("password")) 
        {
            ssid_     = server_.arg("ssid");
            password_ = server_.arg("password");
            Serial.printf("接收到WiFi配置 - SSID: %s, 密码: %s\n", ssid_.c_str(), password_.c_str());
            server_.send(200, "text/html", successPage_);
            vTaskDelay(pdMS_TO_TICKS(2000));
            ConnectToWiFi();
        } 
        else 
        {
            server_.send(400, "text/plain", "错误: 缺少参数");
        }
    });
    server_.begin();
    Serial.println("Web服务器已启动");
}

String NetWork::MakeOptionList()
{
    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks();
    String opt;
    for (int i = 0; i < n; ++i) {
        String ssid = String(WiFi.SSID(i));
        ssid.replace("&", "&amp;");
        ssid.replace("<", "&lt;");
        ssid.replace(">", "&gt;");
        opt += "<option value=\"" + ssid + "\">" + ssid + "</option>";
    }
    return opt;
}