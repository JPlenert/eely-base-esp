// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "Eelybase.h"
#include "ConfigGetHandler.h"
#include "esp_log.h"
#include "ConfigStore.h"

Json ConfigGetHandler :: HandleRequest(Json& request)
{
    Json reply = GetOkReply();
    Json jConfig = g_eelybase->_config->ReadRaw();

    reply.SetNode("config", jConfig);

    return reply;    
}

string ConfigGetHandler :: GetApiId() { return "ConfigGet"; }
