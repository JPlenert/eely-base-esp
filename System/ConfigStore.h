// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef ConfigStore_h
#define ConfigStore_h

#include <string>
#include "Json.h"

using namespace std;

class ConfigStore
{
    public:
        string _wifiSsid;
        string _wifiPassword;

    protected:
        const char *_fileName;

    public:
        ConfigStore() : _fileName("/spiffs/config") {}
        ConfigStore(const char* fileName) : _fileName(fileName) {}
        void Read();
        void Write();
        Json ReadRaw();
        void WriteRaw(Json& json);

    protected:
        virtual void ReadValues(Json& json);
        virtual void WriteValues(Json& json);
};

#endif