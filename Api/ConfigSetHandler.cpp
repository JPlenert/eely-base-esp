// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "Eelybase.h"
#include "ConfigSetHandler.h"
#include "esp_log.h"
#include "ConfigStore.h"

Json ConfigSetHandler :: HandleRequest(Json& request)
{
    Json jConfig = request.GetNode("config").value();
    g_eelybase->_config->WriteRaw(jConfig);

    ESP_LOGI("ConfigSetHandler", "Wrote config");

    return GetOkReply(); 
}

string ConfigSetHandler :: GetApiId() { return "ConfigSet"; }
