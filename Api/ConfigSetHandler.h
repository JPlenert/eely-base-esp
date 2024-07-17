// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef ConfigSetHandler_h
#define ConfigSetHandler_h

#include <cJSON.h>
#include <string>
#include "ApiHandlerBase.h"

class ConfigSetHandler : public ApiHandlerBase
{
    public:
        Json HandleRequest(Json& request);
        string GetApiId();
};


#endif