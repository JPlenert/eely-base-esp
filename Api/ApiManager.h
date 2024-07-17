// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef ApiManager_h
#define ApiManager_h

#include <string>
#include <map>
#include "ApiHandlerBase.h"

using namespace std;

class ApiManager
{
    protected:
        map<string, ApiHandlerBase*> _apiMap;

    public:
        void AddApi(ApiHandlerBase* handler);
        bool RemoveApi(string apiId);
        ApiHandlerBase* GetApi(string apiId);
        string HandleRequest(char *requestString);
};

#endif