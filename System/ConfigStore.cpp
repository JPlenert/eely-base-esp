// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "ConfigStore.h"
#include <stdio.h>
#include "esp_log.h"

static const char *TAG = "ConfigStore";

Json ConfigStore :: ReadRaw()
{
    Json json;

    if (!json.ReadFromFile(_fileName))
    {
        ESP_LOGE(TAG, "No config file found / not readable");
    }

    return json;
}

void ConfigStore :: WriteRaw(Json& json)
{
    json.WriteToFile(_fileName);
}

void ConfigStore :: Read()
{
    Json values = ReadRaw();
    ReadValues(values);
}

void ConfigStore :: ReadValues(Json& json)
{
    _wifiSsid = json.GetString("wifi_ssid");
    _wifiPassword = json.GetString("wifi_password");
}

void ConfigStore :: Write()
{
    Json json;

    WriteValues(json);
    WriteRaw(json);
}

void ConfigStore :: WriteValues(Json& json)
{
    json.SetString("wifi_ssid", _wifiSsid);
    json.SetString("wifi_password", _wifiPassword);
}