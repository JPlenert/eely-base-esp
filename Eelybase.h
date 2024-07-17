// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef Eelybase_h
#define Eelybase_h

#include "ConfigStore.h"
#include <map>
#include <string>
#include "EelybaseStatus.h"

class Eelybase
{
  public:
    ConfigStore* _config;
    static const string FEATKEY_WEBSERVER;
    static const string FEATKEY_SYSTEMTIME;
    static const string FEATKEY_SYSTEMMESSAGE;
  
  protected:
    std::map<string, void*> _featureMap;
    EelybaseStatusEn _baseStatus;
     
  public:
    // Must be called to init the whole system
    virtual void Init();
    void* GetFeature(string featureName);
    virtual string GetSystemStatusString(bool emptyOnOk);
    EelybaseStatusEn GetSystemBaseStatus();
  
  protected:    
    // will be called by init() and should be overwritten by the app to init additional features
    virtual void InitFeatures();
    virtual void InitConfigStore();
    virtual void OnSystemStatusChanged(const char* source);
    void SetBaseStatus(EelybaseStatusEn status);
};

extern Eelybase* g_eelybase;

#endif