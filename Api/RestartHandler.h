// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef RestartHandler_h
#define RestartHandler_h

#include <cJSON.h>
#include <string>
#include "ApiHandlerBase.h"
#include "esp_ota_ops.h"


class RestartHandler : public ApiHandlerBase
{
    public:
        Json HandleRequest(Json& request);
        string GetApiId();
};

#endif
