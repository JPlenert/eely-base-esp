// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "Json.h"
#include "string.h"
#include "esp_log.h"
#include "base64.h"

static const char *TAG = "Json";

Json :: Json(char* jsonString)
{
    _cJson = cJSON_Parse(jsonString);
    if (_cJson == NULL)
        ESP_LOGI(TAG, "Unable to parse at %s", cJSON_GetErrorPtr());
    _jsonRef = new CJsonRef(_cJson);
}

Json :: Json()
{
    _cJson = cJSON_CreateObject();
    _jsonRef = new CJsonRef(_cJson);
}

Json :: Json(const Json& other)
{
    _cJson = other._cJson;
    _jsonRef = other._jsonRef;
    _jsonRef->IncRef();
}

Json :: Json(Json&& other) noexcept
{
    _cJson = other._cJson;
    _jsonRef = other._jsonRef;
    other._cJson = nullptr;
    other._jsonRef = nullptr;
    // move - ref count does not change!
}

Json :: Json(const cJSON *childJson, CJsonRef *json_ref)
{
    _cJson = (cJSON*)childJson;
    _jsonRef = json_ref;
    _jsonRef->IncRef();
}

Json :: ~Json()
{
    Deinit();
}

void Json::Deinit()
{
    if (_jsonRef && _jsonRef->DecRef())
    {
        ESP_LOGD(TAG, "Deleted json");
        // If it's the last instance of the cjson, also delete the ref
        delete _jsonRef;
        _jsonRef = nullptr;
    }
    _cJson = nullptr;
}

Json& Json :: operator=(Json&& other) noexcept
{
    if (this != &other)
    {
        Deinit();
        _cJson = other._cJson;
        _jsonRef = other._jsonRef;
        _jsonRef->IncRef();
    }
    return *this;
}

bool Json :: ReadFromFile(const string fileName)
{
    FILE* file = fopen(fileName.c_str(), "r");

    if (file == NULL)
        return false;

    fseek(file, 0L, SEEK_END);
    int fileLen = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char* fileContent = (char*)malloc(fileLen+1);
    fileLen = fread(fileContent, 1, fileLen, file);
    fileContent[fileLen] = 0;

    fclose(file);

    _cJson = cJSON_Parse(fileContent);
    free(fileContent);

    if (_cJson == NULL)
        return false;

    return true;
}

bool Json :: WriteToFile(const string fileName)
{
    FILE* file = fopen(fileName.c_str(), "w");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Error opening file %s for writing", fileName.c_str());
        return false;
    }

    char* jsonStr = cJSON_Print(_cJson);
    fwrite(jsonStr, strlen(jsonStr), 1, file);

    free(jsonStr);

    fclose(file);

    return true;
}

bool Json :: HasNode(string nodeName, cJSON *parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    return cJSON_GetObjectItem(parentNode, nodeName.c_str()) != NULL;
}

string Json :: GetString(string nodeName, cJSON *parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    const cJSON *jNode = cJSON_GetObjectItem(parentNode, nodeName.c_str());
    if (!cJSON_IsString(jNode))
    {
        ESP_LOGI(TAG, "Did not found string '%s'", nodeName.c_str());
        return {};
    }

    return jNode->valuestring;
}

char* Json :: GetCString(string nodeName, cJSON *parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    const cJSON *jNode = cJSON_GetObjectItem(parentNode, nodeName.c_str());
    if (!cJSON_IsString(jNode))
        return NULL;

    char* retStr = (char*)malloc(sizeof(char) * strlen(jNode->valuestring)+1);
    strcpy(retStr, jNode->valuestring);

    return retStr;
}

int Json :: GetInt(string nodeName, cJSON *parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    const cJSON *jNode = cJSON_GetObjectItem(parentNode, nodeName.c_str());
    if (!cJSON_IsNumber(jNode))
    {
        if (cJSON_IsString(jNode))
            return atoi(jNode->valuestring);
        else
            return {};
    }
    return jNode->valueint;
}

double Json :: GetDouble(string nodeName, cJSON *parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    const cJSON *jNode = cJSON_GetObjectItem(parentNode, nodeName.c_str());
    if (!cJSON_IsNumber(jNode))
        return {};
    return jNode->valuedouble;
}

bool Json :: GetBool(string nodeName, cJSON *parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;
    
    const cJSON *jNode = cJSON_GetObjectItem(parentNode, nodeName.c_str());
    if (!cJSON_IsBool(jNode))
        return false;
    
    return cJSON_IsTrue(jNode);
}

unsigned char* Json :: GetBinary(string nodeName, size_t *outlen, cJSON *parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    const cJSON *jNode = cJSON_GetObjectItem(parentNode, nodeName.c_str());
    if (!cJSON_IsString(jNode))
        return NULL;
    return base64_decode(jNode->valuestring, strlen(jNode->valuestring), outlen);
}

optional<Json> Json :: GetNode(string nodeName, cJSON *parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    const cJSON *jNode = cJSON_GetObjectItem(parentNode, nodeName.c_str());
    if (!cJSON_IsObject(jNode))
        return {};
    return Json(jNode, _jsonRef);
}

optional<JsonArray> Json :: GetArray(string nodeName, cJSON *parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    const cJSON *jNode = cJSON_GetObjectItem(parentNode, nodeName.c_str());
    if (!cJSON_IsArray(jNode))
        return {};
    return JsonArray(jNode, _jsonRef);
}

void Json :: SetString(string nodeName, string data, cJSON* parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    cJSON_AddItemToObject(parentNode, nodeName.c_str(), cJSON_CreateString(data.c_str()));
}

void Json :: SetInt(string nodeName, int data, cJSON* parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    cJSON_AddItemToObject(parentNode, nodeName.c_str(), cJSON_CreateNumber(data));
}

void Json :: SetDouble(string nodeName, double data, cJSON* parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    cJSON_AddItemToObject(parentNode, nodeName.c_str(), cJSON_CreateNumber(data));
}

void Json :: SetBool(string nodeName, bool data, cJSON* parentNode)
{
    if (parentNode == NULL)
        parentNode = _cJson;

    cJSON_AddItemToObject(parentNode, nodeName.c_str(), cJSON_CreateBool(data));
}

void Json :: SetNode(string nodeName, Json& nodeObject)
{
   cJSON_AddItemToObject(_cJson, nodeName.c_str(), nodeObject._cJson); 
   nodeObject._cJson = cJSON_CreateObject();
   nodeObject._jsonRef = new CJsonRef(nodeObject._cJson);
}

JsonArray Json :: AddArray(string nodeName)
{
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToObject(_cJson, nodeName.c_str(), array);
    return JsonArray(array, _jsonRef);
}

Json Json :: AddNode(string nodeName)
{
    cJSON *newNode = cJSON_CreateObject();
    cJSON_AddItemToObject(_cJson, nodeName.c_str(), newNode); 
    return Json(newNode, _jsonRef);
}

bool Json :: IsEmpty()
{
    return _cJson == NULL || (_cJson->next == NULL && _cJson->prev == NULL && _cJson->child == NULL);
}

string Json :: ToString()
{
    char* jsonStr = cJSON_Print(_cJson);
    string retStr(jsonStr);   
    free(jsonStr);

    return retStr;
}
