// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "ApiManager.h"
#include "Json.h"
#include "esp_log.h"

void ApiManager :: AddApi(ApiHandlerBase* handler)
{
    _apiMap[handler->GetApiId()] = handler;
}

ApiHandlerBase* ApiManager :: GetApi(string apiId)
{
    return _apiMap[apiId];
}

bool ApiManager :: RemoveApi(string apiId)
{
    auto iter = _apiMap.find(apiId);
    if (iter == _apiMap.end())
        return false;
    _apiMap.erase(iter);
    return true;
}

string ApiManager :: HandleRequest(char *requestString)
{
    string retStr;
    Json jsonRequest(requestString);   

    if (jsonRequest.IsEmpty())
    {
        ESP_LOGI("ApiManager", "handleRequest: Got invalid json to handle");
        return retStr;
    }

    string apiId = jsonRequest.GetString("ApiId");
    if (apiId.empty())
    {
        ESP_LOGI("ApiManager", "handleRequest: No ApiId in request");
        return retStr;
    }

    ApiHandlerBase* handler = GetApi(apiId);
    if (handler == NULL)
    {
        ESP_LOGI("ApiManager", "handleRequest: Did not found requested Api '%s'", apiId.c_str());
        return retStr;
    }

    Json jsonReply = handler->HandleRequest(jsonRequest);
    return jsonReply.ToString();
}