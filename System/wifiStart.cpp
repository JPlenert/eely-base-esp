// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "Eelybase.h"
#include "ConfigStore.h"
#include "esp_log.h"
#include "esp_err.h"
#include "wifi.h"
#include "string_format.h"
#include "DnsServer/DnsServer.h"

#include <optional>
#include <functional>

using namespace std;

optional<function<void(WiFiStatusEn wifistat)>> g_wifi_status_cb;
WiFiStatusEn g_wifi_status = WifiStatus_Startup;
DnsServer* g_dnsServer = nullptr;

extern int s_wifiSta_retry_num;

esp_err_t wifiStart(string ssid, string password)
{
    esp_err_t err;

    if (ssid.empty())
    {
        ESP_LOGI("main", "Entering AP-Mode");
        wifiAp_init(5);

        wifi_status_set(WifiStatus_AP);

        // Start the Captive Portal DnsServer
        g_dnsServer = new DnsServer();
        g_dnsServer->Start();
    }   
    else
    {
        wifi_status_set(WifiStatus_StationConnecting);

        ESP_LOGI("main", "Entering Station-Mode");

        ESP_LOGI("main", "wifi_connect()");
        err = wifiSta_do_connect_by_ssid(ssid.c_str(), password.c_str());
        if (err != ESP_OK)
        {
            ESP_LOGI("main", "wifi_connect() failed!");
            return err;
        }
        
        ESP_LOGI("main", "wifi_connect() done");
    }

    return ESP_OK;
}

bool wifiStartFromConfig()
{
    if (g_eelybase->_config->_wifiSsid.empty())
    {
        ESP_LOGI("main", "Found no config, starting in SoftAP mode.");
        wifiStart({}, {});
        return false;
    }
    else
    {
        if (wifiStart(g_eelybase->_config->_wifiSsid, g_eelybase->_config->_wifiPassword) != ESP_OK)
        {
            ESP_LOGI("main", "Unable to connect to configured SSID, going back to AP mode.");
            wifiStart({}, {});
            return false;
        }
    }
    return true;
}

WiFiStatusEn wifi_status_get()
{
    return g_wifi_status;
}

string wifi_status_get_string()
{
    switch (g_wifi_status)
    {
        case WifiStatus_Startup: return "startup";
        case WifiStatus_AP: return "AP";
        case WifiStatus_StationConnecting: return string_format("con %d", s_wifiSta_retry_num);
        case WifiStatus_StationWaitForIP: return "wait ip";
        case WifiStatus_StationOK: return {};
    }
    return {};
}

void wifi_status_set(WiFiStatusEn wifi_status)
{
    g_wifi_status = wifi_status;
    if (g_wifi_status_cb.has_value())
        (g_wifi_status_cb.value())(wifi_status);
}

void wifi_status_set_cb(std::function<void(WiFiStatusEn wifistat)> wifi_status_cb)
{
    g_wifi_status_cb = wifi_status_cb;
}

void wifi_status_clear_cb()
{
    g_wifi_status_cb.reset();
}