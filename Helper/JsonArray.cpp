// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "Json.h"
#include "esp_log.h"

static const char* TAG = "JsonArray";

JsonArray :: JsonArray(const cJSON* cJsonArray, CJsonRef* json_ref)
{
    _cJsonArray = (cJSON*)cJsonArray;
    _jsonRef = json_ref;
    _jsonRef->IncRef();
    _currentPtr = NULL;
}

JsonArray::JsonArray(const JsonArray& other) 
{
    _cJsonArray = other._cJsonArray;
    _jsonRef = other._jsonRef;
    _jsonRef->IncRef();
    _currentPtr = NULL;
}

JsonArray::JsonArray(JsonArray&& other) noexcept 
{
    _cJsonArray = other._cJsonArray;
    _jsonRef = other._jsonRef;
    other._cJsonArray = nullptr;
    other._jsonRef = nullptr;
    // move - ref count does not change!
    _currentPtr = NULL;
}

JsonArray::~JsonArray()
{
    Deinit();
}

void JsonArray::Deinit()
{
    if (_jsonRef && _jsonRef->DecRef())
    {
        ESP_LOGD(TAG, "Deleted json");
        // If it's the last instance of the cjson, also delete the ref
        delete _jsonRef;
        _jsonRef = nullptr;
    }
}

JsonArray& JsonArray :: operator=(JsonArray&& other) noexcept
{
    if (this != &other)
    {
        Deinit();
        _cJsonArray = other._cJsonArray;
        _jsonRef = other._jsonRef;
        _jsonRef->IncRef();       
        _currentPtr = nullptr;
    }
    return *this;
}

void JsonArray :: AddInt(int data)
{
    cJSON_AddItemToArray(_cJsonArray, cJSON_CreateNumber(data));
}

void JsonArray :: AddString(string data)
{
    cJSON_AddItemToArray(_cJsonArray, cJSON_CreateString(data.c_str()));
}

Json JsonArray :: AddNode()
{
    cJSON* newCJson = cJSON_CreateObject();  
    
    cJSON_AddItemToArray(_cJsonArray, newCJson);

    return Json(newCJson, _jsonRef);
}

int JsonArray :: GetSize()
{
    return cJSON_GetArraySize(_cJsonArray);
}

optional<Json> JsonArray :: GetFirst()
{
    _currentPtr = _cJsonArray->child;

    if (_currentPtr == NULL)
        return {};
    return Json(_currentPtr, _jsonRef);
}

optional<Json> JsonArray :: GetNext()
{
    if (_currentPtr == NULL)
        _currentPtr = _cJsonArray->child;
    else
        _currentPtr = _currentPtr->next;

    if (_currentPtr == NULL)
        return {};
    return Json(_currentPtr, _jsonRef);
}

bool JsonArray :: IsInvalid()
{
    return _cJsonArray == NULL;
}

bool JsonArray :: IsEmpty()
{
    return _cJsonArray == NULL || _cJsonArray->child == NULL;
}
