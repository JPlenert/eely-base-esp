// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef WifiSettingsSetHandler_h
#define WifiSettingsSetHandler_h

#include <cJSON.h>
#include <string>
#include "ApiHandlerBase.h"

class WifiSettingsSetHandler : public ApiHandlerBase
{
    public:
        Json HandleRequest(Json& request);
        string GetApiId();
};

#endif
