#include "network.h"

NetWork::NetWork() : _server(80)
{
        // 配置页面HTML内容
    _configPage = R"rawliteral(
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
            _configPage += MakeOptionList();
            _configPage += R"rawliteral(
                </select>
                <input type="password" name="password" placeholder="WiFi密码" required>
                <button type="submit">连接</button>
            </form>
        </body>
        </html>)rawliteral";

    // 连接成功页面
    _successPage = R"rawliteral(
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
    _failurePage = R"rawliteral(             
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
    _queue = xQueueCreate(1, sizeof(NetWorkInfoStruct));
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
        _net_work_info.status = WiFi.status();
        _net_work_info.mode = WiFi.getMode();
        xQueueOverwrite(_queue, &_net_work_info);
        
        if (_net_work_info.status == WL_CONNECTED)
        {

        } 
        else if (_net_work_info.status == WL_DISCONNECTED) 
        {
            if (!_scanning && !_apStarted) 
            {
                Serial.println("Wi-Fi 断线，重新进入周期扫描...");
                WiFi.mode(WIFI_OFF);
                vTaskDelay(pdMS_TO_TICKS(200));
                _scanning = true;
            }
            _server.handleClient();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void NetWork::StartConection()
{
    while (true) 
    {
        while(_scanning)
        {
            WiFi.mode(WIFI_STA);
            vTaskDelay(100);
            int scanRes = WiFi.scanNetworks();
            if(scanRes >= 0)
            {
                Serial.print("当前扫描到热点数量: ");
                Serial.println(scanRes);
                _connect_count ++;
                bool hit = false;
                for (int i = 0; i < scanRes; ++i)
                {
                    String seen = WiFi.SSID(i);
                    for (auto& r : _netList) 
                    {
                        if (seen == r.ssid) 
                        {
                            Serial.printf("发现已保存WiFi: %s，尝试连接...\n", seen.c_str());
                            _ssid = r.ssid;
                            _password = r.pass;
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
            
            if (_connect_count >= 4)
            {
                _apStarted = true;       
                _scanning = false;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        if(_apStarted && _connect_count != 0)
        {
            Serial.println("本次无匹配，打开配网 Web...");
            _connect_count = 0;
            StartAPMode();
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

bool NetWork::Available()
{
    return xQueueReceive(_queue, &_net_work_info, 0) == pdTRUE;
}

NetWork::NetWorkInfoStruct NetWork::ReadNetWorkInfo()
{
    return _net_work_info;
}

bool NetWork::LoadWiFiConfig()
{
    _netList.clear();
    _prefs.begin(NS, false);
    String json = _prefs.getString(KEY_LIST, "[]");
    _prefs.end();

    Serial.print("从NVS读出的原始json: ");
    Serial.println(json);

    JsonDocument doc;
    deserializeJson(doc, json);
    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject o : arr)
    {
        _netList.push_back({o["s"], o["p"]});
    }
    return !_netList.empty();
}

bool NetWork::SaveWiFiConfig()
{
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (auto& r : _netList)
    {
        JsonObject obj = arr.add<JsonObject>();
        obj["s"] = r.ssid;
        obj["p"] = r.pass;
    }

    String json;
    serializeJson(doc, json);
    _prefs.begin(NS, false);
    _prefs.putString(KEY_LIST, json);
    _prefs.end();
    return true;
}

void NetWork::AddWiFi(const String& ssid, const String& pass)
{
    for (auto& r : _netList)
    {
        if (r.ssid == ssid)
        { 
            r.pass = pass;
            SaveWiFiConfig();
            return; 
        }
    }
    
    if (_netList.size() >= MAX_NETS) 
    {
        _netList.pop_back();
    }
    _netList.push_back({ssid, pass});
    SaveWiFiConfig();
}

void NetWork::ConnectToWiFi()
{
    WiFi.begin(_ssid.c_str(), _password.c_str());
    Serial.printf("连接 %s ...\n", _ssid.c_str());
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
        _connect_count = 0;
        _scanning = false;
        _apStarted = false;
        AddWiFi(_ssid, _password);

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

    _server.on("/", [this]() { _server.send(200, "text/html", _configPage); });
    _server.on("/config", [this]() 
    {
        if (_server.hasArg("ssid") && _server.hasArg("password")) 
        {
            _ssid     = _server.arg("ssid");
            _password = _server.arg("password");
            Serial.printf("接收到WiFi配置 - SSID: %s, 密码: %s\n", _ssid.c_str(), _password.c_str());
            _server.send(200, "text/html", _successPage);
            vTaskDelay(pdMS_TO_TICKS(2000));
            ConnectToWiFi();
        } 
        else 
        {
            _server.send(400, "text/plain", "错误: 缺少参数");
        }
    });
    _server.begin();
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