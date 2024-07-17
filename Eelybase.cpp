// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "Eelybase.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "_ApiBase.h"
#include "_SystemBase.h"
#include "WebServer.h"
#include "SystemTime.h"
#include "SystemMessage.h"
#include "lwip/inet.h"
#include "string_format.h"

Eelybase* g_eelybase;

const string Eelybase::FEATKEY_WEBSERVER = "webserver";
const string Eelybase::FEATKEY_SYSTEMTIME = "systemtime";
const string Eelybase::FEATKEY_SYSTEMMESSAGE = "systemmessage";

void Eelybase :: SetBaseStatus(EelybaseStatusEn status)
{
    _baseStatus = status;
    OnSystemStatusChanged("Eelybase");
}

void Eelybase :: Init()
{    
    SetBaseStatus(EelybaseStatus_InitStart);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        SetBaseStatus(EelybaseStatus_FlashFormat);
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    bool doFactoryReset = factory_reset_check();

    SetBaseStatus(EelybaseStatus_Spiffs);

    spiffs_init(doFactoryReset);

    SetBaseStatus(EelybaseStatus_NetworkInit);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    SetBaseStatus(EelybaseStatus_ConfigRead);
    InitConfigStore();
    _config->Read();

    SetBaseStatus(EelybaseStatus_WifiInit);

    wifi_status_set_cb([&](WiFiStatusEn status) { OnSystemStatusChanged("Eelybase"); });
    bool stationMode = wifiStartFromConfig();
    wifi_status_clear_cb();

    SetBaseStatus(EelybaseStatus_InitFeatures);
    InitFeatures();
    
    // Normally we wait for SNTP now. This maybe different if inherited InitFeatures will do different
    if (stationMode)
        SetBaseStatus(EelybaseStatus_SNTPWait);
    else
        SetBaseStatus(EelybaseStatus_WifiInit);
}

void Eelybase :: InitFeatures()
{
    _featureMap[FEATKEY_SYSTEMMESSAGE] = new SystemMessage();

    Webserver* m_webServer = new Webserver(80);
    m_webServer->Start();

    SystemTime *sysTime = new SystemTime();
    if (wifi_status_get() != WifiStatus_AP)
    {
        SetBaseStatus(EelybaseStatus_SNTP);
        sysTime->EnableSNTP();
    }

    m_webServer->AddApi(new WifiSettingsSetHandler());
    m_webServer->AddApi(new OtaUploadHandler());
    m_webServer->AddApi(new RestartHandler());
    m_webServer->AddApi(new DiagnosticGetHandler());
    m_webServer->AddApi(new DeviceInfoGetHandler());
    m_webServer->AddApi(new ConfigGetHandler());
    m_webServer->AddApi(new ConfigSetHandler());

    _featureMap[FEATKEY_WEBSERVER] = m_webServer;
    _featureMap[FEATKEY_SYSTEMTIME] = sysTime;
}

void Eelybase :: InitConfigStore() 
{ 
    _config = new ConfigStore(); 
}

void* Eelybase :: GetFeature(string featureName) 
{ 
    return _featureMap[featureName]; 
}

void Eelybase :: OnSystemStatusChanged(const char* source) {}

EelybaseStatusEn Eelybase :: GetSystemBaseStatus()
{
    return _baseStatus;
}

string Eelybase :: GetSystemStatusString(bool emptyOnOk)
{
    switch (_baseStatus)
    {
        case EelybaseStatus_InitStart: return "init";
        case EelybaseStatus_FlashFormat: return "flash format";
        case EelybaseStatus_Spiffs: return "Spiffs init";
        case EelybaseStatus_NetworkInit: return "Network init";
        case EelybaseStatus_ConfigRead: return "Config read";
        case EelybaseStatus_WifiInit:{
            if (wifi_status_get() == WifiStatus_AP)   { 
                ip4_addr_t addr = (ip4_addr_t)wifiAp_get_ip_address();
                return string_format("AP %s", inet_ntoa(addr));
                // )"AP 192.168.4.1"+
            }
            return "wifis "+wifi_status_get_string();
        } 
        case EelybaseStatus_InitFeatures: return "Feat init";
        case EelybaseStatus_SNTPWait: return "SNTP wait";
        case EelybaseStatus_SNTP: return "SNTP init";
        case EelybaseStatus_OK: return emptyOnOk ? "" : "Ok";
    }
    return {};
}

