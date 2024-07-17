// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef webserver_h
#define webserver_h

#include <esp_http_server.h>
#include "esp_log.h"
#include <functional>
#include "ApiManager.h"

class Webserver
{
    private:
        httpd_handle_t _server;
        int _port;

        httpd_uri_t _com_post_uri;
        httpd_uri_t _get_uri;

        ApiManager _apiManager;
        std::function<bool(const char* fileName)> _spiffsAllowCheckFn;

        char* _indexContent;
        int _indexLen;

    public:
        Webserver(int port);
        esp_err_t Start();
        esp_err_t Stop();
        void AddApi(ApiHandlerBase* handler);
        bool RemoveApi(string apiId);
        void SetIndex(char* content, int len);

        void SetSpiffsDownloadAllowCheckFn(std::function<bool(const char* fileName)> spiffsAllowCheckFn);
    
    public:
        esp_err_t Com_post_handler(httpd_req_t *req);
        esp_err_t Get_handler(httpd_req_t *req);
};

#endif