#pragma once

#include <WebServer.h>
#include <WiFi.h>
#include <Arduino.h>
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
        uint8_t _connect_count = 0;
        bool _scanning = false;
        bool _apStarted = false;
        std::vector<NetRecord> _netList;
        QueueHandle_t _queue;      
        WebServer _server;
        Preferences _prefs;
        static constexpr const char* NS       = "wifi";
        static constexpr const char* KEY_LIST = "list";
        static constexpr const uint8_t MAX_NETS = 10;
        NetWorkInfoStruct _net_work_info;
        String _ssid;
        String _password;
        String  _configPage;
        String _successPage;
        String _failurePage;
};