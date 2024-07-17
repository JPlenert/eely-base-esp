// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef ConfigGetHandler_h
#define ConfigGetHandler_h

#include <cJSON.h>
#include <string>
#include "ApiHandlerBase.h"

class ConfigGetHandler : public ApiHandlerBase
{
    public:
        Json HandleRequest(Json& request);
        string GetApiId();
};

#endif