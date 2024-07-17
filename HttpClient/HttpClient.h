// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef HttpClient_h
#define HttpClient_h

#include <string>
#include "esp_http_client.h"

using namespace std;

class HttpClient
{
    private:
        string _requestUrl;
        string _lastError;
        esp_http_client_config_t _httpConfig;
        esp_http_client_handle_t _httpClient;
        string _authUser;
        string _authPassword;

    public:
        HttpClient();
        HttpClient(string url);
        ~HttpClient();

        void SetUrl(string url);
        void SetBasicAuth(string user, string password);
        void SetDigestAuth(string user, string password);
        void AddRequestHeader(string key, string value);

        int Get(char *resultBuffer, int resultBufferLen);
        int Post(const char* post_data, char *resultBuffer, int resultBufferLen);
        string GetLastError() { return _lastError; }

    protected:
        void Init();
        void Cleanup();
        int ExecuteRequest(const char* post_data, char *resultBuffer, int resultBufferLen);
};

#endif