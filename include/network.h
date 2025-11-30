// #pragma once

// #include <Arduino.h>
// #include <WiFi.h>
// #include <WiFiClient.h>
// #include <WebServer.h>
// #include <EEPROM.h>
// #include <Preferences.h>
// #include <ArduinoJson.h>
// #include <nvs_flash.h>
// #include <memory>
// #include "web_pages.h"

// class NetWork
// {
//     public:

//         static NetWork& GetInstance()
//         {
//             static NetWork instance;
//             return instance;
//         }

//         NetWork() = default;
        
//         ~NetWork() = default;

//         struct NetWorkInfoStruct
//         {
//             wl_status_t status;
//             wifi_mode_t mode;
//         };

//         void InitNetWork();

//         bool Available();

//         NetWorkInfoStruct ReadNetWorkInfo();
    
//     private:
//         static void NetWorkTask(void *);

//         static void ConnectWorkTask(void *);

//         void GetNetWorkInfo();

//         bool LoadWiFiConfig();

//         bool SaveWiFiConfig();

//         bool ConnectToWiFi();

//         void StartAPMode();

//         void AddWiFi(const String& ssid, const String& pass);

//         void StartConection();

//         void HandleRoot();

//         void HandleConfig();

//         void HandleScan();

//         void HandleSuccess();

//         void HandleFailure();

//         void HandleStatus();
        
//         String EscapeJsonString(const String& input);

//     private:

//         struct NetRecord {          // 一条记录
//             String ssid;
//             String pass;
//         };
//         uint8_t scanRes_ = 0;
//         uint8_t connect_count_ = 0;
//         bool scanning_ = true;
//         bool apStarted_ = false;
//         std::vector<NetRecord> netList_;
//         QueueHandle_t queue_;      
//         std::unique_ptr<WebServer> server_;
//         Preferences prefs_;
//         static constexpr const char* NS       = "wifi";
//         static constexpr const char* KEY_LIST = "list";
//         static constexpr const uint8_t MAX_NETS = 10;
//         NetWorkInfoStruct net_work_info_;
//         String ssid_;
//         String password_;
// };

#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <memory>
#include <vector>

// 网络配置常量
#define MAX_NETS 5
#define NS "network"
#define KEY_LIST "wifilist"

// 网页文件
#include "web_pages.h"

struct WiFiCredential {
    String ssid;
    String pass;
};

class NetWork {
public:
    struct NetWorkInfoStruct {
        uint8_t status;
        uint8_t mode;
    };

    static NetWork& GetInstance()
    {
        static NetWork instance;
        return instance;
    }

    NetWork();
    ~NetWork();

    void InitNetWork();
    bool Available();
    NetWorkInfoStruct ReadNetWorkInfo();

private:
    // 任务函数
    static void NetWorkTask(void *pvParameters);
    static void ConnectWorkTask(void *pvParameters);
    static void DelayedAPCloseTask(void *pvParameters);
    
    // 核心功能
    void GetNetWorkInfo();
    void StartConection();
    void PerformSafeScan();
    void StartSafeAPMode();
    bool ConnectToWiFi();
    void SafeHandleWebClient();
    
    // Web处理函数
    void HandleRoot();
    void HandleConfig();
    void HandleScan();
    void HandleSuccess();
    void HandleFailure();
    void HandleStatus();
    
    // 工具函数
    bool LoadWiFiConfig();
    bool SaveWiFiConfig();
    void AddWiFi(const String& ssid, const String& pass);
    String EscapeJsonString(const String& input);
    void SafeWiFiReset();
    
    // 互斥锁保护函数
    void SetScanning(bool value);
    void SetApStarted(bool value);
    bool GetScanning();
    bool GetApStarted();
    void IncrementConnectCount();
    void ResetConnectCount();
    int GetConnectCount();

    // 成员变量
    std::unique_ptr<WebServer> server_;
    Preferences prefs_;
    std::vector<WiFiCredential> netList_;
    QueueHandle_t queue_;
    NetWorkInfoStruct net_work_info_;
    
    // 受保护的共享变量
    bool scanning_ = false;
    bool apStarted_ = false;
    int connect_count_ = 0;
    String ssid_;
    String password_;
    int scanRes_ = 0;
    
    // 互斥锁
    SemaphoreHandle_t networkMutex_;
};
