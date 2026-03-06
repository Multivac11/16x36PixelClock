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
