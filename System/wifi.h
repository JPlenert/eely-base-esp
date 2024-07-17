// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef Wifi_h
#define Wifi_h

#include <string>
#include <functional>

#define esp_err_t int

void wifi_generateName(char hostname[25]);

void wifiAp_init(int channel);
uint32_t wifiAp_get_ip_address();
esp_err_t wifiSta_do_connect_by_ssid(const char* ssid, const char* password);

// Wifi

// returns false if started in AP-Mode
bool wifiStartFromConfig();

enum WiFiStatusEn{
    WifiStatus_Startup,
    WifiStatus_AP,
    WifiStatus_StationConnecting,
    WifiStatus_StationWaitForIP,
    WifiStatus_StationOK,
};

WiFiStatusEn wifi_status_get();
std::string wifi_status_get_string();
void wifi_status_set(WiFiStatusEn wifi_status);
void wifi_status_set_cb(std::function<void(WiFiStatusEn wifistat)> wifi_status_cb);
void wifi_status_clear_cb();

#endif