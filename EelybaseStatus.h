// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef EelybaseStatus_h
#define EelybaseStatus_h

// Overall system status
enum EelybaseStatusEn{
  EelybaseStatus_OK = 0,
  EelybaseStatus_InitStart,
  EelybaseStatus_FlashFormat,
  EelybaseStatus_Spiffs,
  EelybaseStatus_NetworkInit,
  EelybaseStatus_ConfigRead,
  EelybaseStatus_WifiInit,
  EelybaseStatus_InitFeatures,
  EelybaseStatus_SNTP,
  EelybaseStatus_SNTPWait,
};

#endif