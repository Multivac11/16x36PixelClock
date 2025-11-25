#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <nvs_flash.h>

class NetWork
{
    public:

        static NetWork& GetInstance()
        {
            static NetWork instance;
            return instance;
        }

        NetWork();
        
        ~NetWork() = default;

        struct NetWorkInfoStruct
        {
            wl_status_t status;
            wifi_mode_t mode;
        };

        void InitNetWork();

        bool Available();

        NetWorkInfoStruct ReadNetWorkInfo();
    
    private:
        static void NetWorkTask(void *);

        static void ConnectWorkTask(void *);

        void GetNetWorkInfo();

        bool LoadWiFiConfig();

        bool SaveWiFiConfig();

        void ConnectToWiFi();

        void StartAPMode();

        String  MakeOptionList();

        void AddWiFi(const String& ssid, const String& pass);

        void StartConection();

    private:

        struct NetRecord {          // 一条记录
            String ssid;
            String pass;
        };
        uint8_t connect_count_ = 0;
        bool scanning_ = false;
        bool apStarted_ = false;
        std::vector<NetRecord> netList_;
        QueueHandle_t queue_;      
        WebServer server_;
        Preferences prefs_;
        static constexpr const char* NS       = "wifi";
        static constexpr const char* KEY_LIST = "list";
        static constexpr const uint8_t MAX_NETS = 10;
        NetWorkInfoStruct net_work_info_;
        String ssid_;
        String password_;
        String  configPage_;
        String successPage_;
        String failurePage_;
};