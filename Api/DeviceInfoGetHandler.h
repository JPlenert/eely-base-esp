// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef DeviceInfoGetHandler_h
#define DeviceInfoGetHandler_h

#include "ApiHandlerBase.h"
#include "cJSON.h"

class DeviceInfoGetHandler : public ApiHandlerBase
{
    public:
        Json HandleRequest(Json& request);        
        string GetApiId();

    protected:
        virtual void CollectInfo(Json &repyl);
};

#endif





