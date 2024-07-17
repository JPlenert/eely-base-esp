// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "RestartHandler.h"
#include "esp_log.h"
#include "esp_system.h"

Json RestartHandler :: HandleRequest(Json& request)
{
    ESP_LOGI("RestartHandler", "Restarting");
    esp_restart();

    // you'll never get here ....
    return GetOkReply();
}

string RestartHandler :: GetApiId() { return "Restart"; }
