// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef Json_h
#define Json_h

#include "cJSON.h"
#include <string>
#include <optional>

using namespace std;

class CJsonRef
{
    private:
        cJSON* _cJson;
        int ref_count_;

    public:
        CJsonRef(cJSON* cjson) : _cJson(cjson), ref_count_(1) {}
        void IncRef() { ref_count_++; /* printf("i-rc:%u\n", ref_count_); */ }
        bool DecRef() { /* printf("d-rc:%u\n", ref_count_ - 1); */ if (--ref_count_ == 0) { cJSON_Delete(_cJson);  _cJson = NULL; return true; } return false; }
        cJSON* Get() { return _cJson; }
};

class JsonArray;

class Json
{
    protected:
        CJsonRef* _jsonRef;
        cJSON* _cJson;

    public:
        Json();
        Json(char* jsonString);
        Json(const Json& other);
        Json(Json&& other) noexcept;
        Json(const cJSON* childJson, CJsonRef* json_ref);

        ~Json();

        Json& operator=(Json&& other) noexcept;

        bool ReadFromFile(const string fileName);
        bool WriteToFile(const string fileName);

        bool HasNode(string nodeName, cJSON *parentNode = NULL);

        string GetString(string nodeName, cJSON *parentNode = NULL);
        char* GetCString(string nodeName, cJSON *parentNode = NULL);
        int GetInt(string nodeName, cJSON *parentNode = NULL);
        double GetDouble(string nodeName, cJSON *parentNode = NULL);
        bool GetBool(string nodeName, cJSON *parentNode = NULL);
        unsigned char* GetBinary(string nodeName, size_t *outlen, cJSON *parentNode = NULL);

        optional<Json> GetNode(string nodeName, cJSON *parentNode = NULL);
        optional<JsonArray> GetArray(string nodeName, cJSON *parentNode = NULL);        
        
        void SetString(string nodeName, string data, cJSON* parentNode = NULL);
        void SetInt(string nodeName, int data, cJSON* parentNode = NULL);
        void SetDouble(string nodeName, double data, cJSON* parentNode = NULL);
        void SetBool(string nodeName, bool data, cJSON* parentNode = NULL);

        // Warning - The nodeObject will be moved to the destination, no copied!
        void SetNode(string nodeName, Json& nodeObject);
        
        JsonArray AddArray(string nodeName);
        Json AddNode(string nodeName);

        bool IsEmpty();

        string ToString();

    protected:
        void Deinit();
};

class JsonArray
{
    protected:
        CJsonRef* _jsonRef;
        cJSON *_cJsonArray;
        cJSON *_currentPtr;
    
    public:
        JsonArray(const cJSON *cjson_array, CJsonRef* json_ref);
        JsonArray(const JsonArray& other);
        JsonArray(JsonArray&& other) noexcept;
        ~JsonArray();

        JsonArray& operator=(JsonArray&& other) noexcept;

        bool IsInvalid();
        bool IsEmpty();
        
        void AddInt(int data);
        void AddString(string data);
        Json AddNode();

        optional<Json> GetFirst();
        optional<Json> GetNext();
        int GetSize();

    protected:
        void Deinit();
};

#endif