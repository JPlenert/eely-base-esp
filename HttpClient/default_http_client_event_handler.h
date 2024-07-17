// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef DefaultHttpClientHandler_h
#define DefaultHttpClientHandler_h

#include "esp_http_client.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct
    {
        int maxLen;
        int curLen;
        int memLen;
        char* content;
    }default_http_event_handler_data;

    esp_err_t default_http_client_event_handler(esp_http_client_event_t *evt);
    
#ifdef __cplusplus
}
#endif


#endif

