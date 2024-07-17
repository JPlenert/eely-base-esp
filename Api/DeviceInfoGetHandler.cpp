// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "DeviceInfoGetHandler.h"
#include "esp_ota_ops.h"
#include "esp_idf_version.h"
#include "Eelybase.h"
#include "esp_log.h"

using namespace std;

#include <esp_app_desc.h>
#include <esp_app_format.h>

void DeviceInfoGetHandler :: CollectInfo(Json &reply)
{
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    reply.SetString("version", esp_app_get_description()->version);
    #else
    reply.SetString("version", esp_ota_get_app_description());
    #endif

    reply.SetString("system_status", g_eelybase->GetSystemStatusString(false));
    reply.SetInt("system_base_status", g_eelybase->GetSystemBaseStatus());
}

Json DeviceInfoGetHandler :: HandleRequest(Json& request)
{
    Json reply = GetOkReply();

    CollectInfo(reply);

    return reply;
}

string DeviceInfoGetHandler :: GetApiId() { return "DeviceInfoGet"; }
