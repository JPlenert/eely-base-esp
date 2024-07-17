// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "Eelybase.h"
#include "WifiSettingsSetHandler.h"
#include "esp_log.h"
#include "ConfigStore.h"

Json WifiSettingsSetHandler :: HandleRequest(Json& request)
{
    g_eelybase->_config->_wifiSsid = request.GetString("ssid");
    g_eelybase->_config->_wifiPassword = request.GetString("password");
    g_eelybase->_config->Write();

    ESP_LOGI("WifiSettingsSetHandler", "Wrote wifi settings");

    return GetOkReply();
}

string WifiSettingsSetHandler :: GetApiId() { return "WifiSettingsSet"; }
