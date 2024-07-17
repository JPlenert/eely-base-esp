// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "WebServer.h"
#include <esp_http_server.h>
#include "esp_log.h"
#include <list>

static const char *TAG = "Webserver";

esp_err_t Webserver_com_post_handler(httpd_req_t *req){
    return ((Webserver*)req->user_ctx)->Com_post_handler(req);
}

esp_err_t Webserver_get_handler(httpd_req_t *req){
    return ((Webserver*)req->user_ctx)->Get_handler(req);
}

Webserver :: Webserver(int port)
{
    _port = port;
    _server = NULL;
    _com_post_uri =  {.uri = "/com",  .method   = HTTP_POST,   .handler  = Webserver_com_post_handler,  .user_ctx = this };
    _get_uri = {.uri = "/*",  .method   = HTTP_GET,   .handler  = Webserver_get_handler,  .user_ctx = this };
}      

esp_err_t Webserver :: Start()
{
    esp_err_t err;            
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = _port;
    config.max_uri_handlers = 2;
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting Webserver on port: '%d'", config.server_port);
    if ((err = httpd_start(&_server, &config)) != ESP_OK)
    {
        ESP_LOGI(TAG, "Error starting Webserver (%x) !", err);
    }
    else
    {
        err = httpd_register_uri_handler(_server, &_com_post_uri);
        if (err != ESP_OK)
        {
            ESP_LOGI(TAG, "Error registering com-url (%x) !", err);
            Stop();
        }

        err = httpd_register_uri_handler(_server, &_get_uri);
        if (err != ESP_OK)
        {
            ESP_LOGI(TAG, "Error registering get-url (%x) !", err);
            Stop();
        }
    }


    return err;
}

esp_err_t Webserver :: Stop()
{
    esp_err_t err;

    err = httpd_stop(_server);

    return err;
}

esp_err_t Webserver :: Get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Requested Uri %s", req->uri);

    if (strcmp(req->uri, "/") == 0)
    {
        httpd_resp_send(req, _indexContent, _indexLen);
        ESP_LOGI(TAG, "Sending index.html");
    }
    // Captive portal redirects
    else if (strcmp(req->uri, "/generate_204") == 0 || strcmp(req->uri, "/gen_204") == 0 || strcmp(req->uri, "/204") == 0 || strcmp(req->uri, "/redirect") == 0 || strcmp(req->uri, "/hotspot-detect.html") == 0 || strcmp(req->uri, "/ncsi.txt") == 0 || strcmp(req->uri, "/connecttest.txt") == 0)
    {
        const char resp[] = "302 Found";

        httpd_resp_set_status(req, resp);
        httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/");
        httpd_resp_send(req, NULL, 0);
        ESP_LOGI(TAG, "Sending 302 & redirect to http://192.168.4.1");
    }
    else if (_spiffsAllowCheckFn != NULL && strncmp(req->uri, "/spiffs?", 8) == 0 && strlen(req->uri)> 8)
    {
        if (!_spiffsAllowCheckFn(&req->uri[8]))        
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, NULL);
        else
        {
            // Get file and send it!
            httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, NULL);
        }
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, NULL);
    }

    return ESP_OK;
}

void Webserver :: SetIndex(char* content, int len)
{
    _indexContent = content;
    _indexLen = len;
}

esp_err_t Webserver :: Com_post_handler(httpd_req_t *req)
{
    char *content;

    // Reject to long packets
    if (req->content_len+1 > 10240)
    {
        // 403 Payload Too Large
        httpd_resp_send_err(req, (httpd_err_code_t)413, NULL);
        return ESP_FAIL;
    }

    content = (char*)malloc(req->content_len+1);
    int written = 0;

    while (written < req->content_len)
    {
        int ret = httpd_req_recv(req, &content[written], req->content_len - written);
        if (ret <= 0) {  /* 0 return value indicates connection closed */
            free(content);
            /* Check if timeout occurred */
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* In case of timeout one can choose to retry calling
                * httpd_req_recv(), but to keep it simple, here we
                * respond with an HTTP 408 (Request Timeout) error */
                httpd_resp_send_408(req);
            }
            /* In case of error, returning ESP_FAIL will
            * ensure that the underlying socket is closed */
            return ESP_FAIL;
        }
        written += ret;
    }

    content[req->content_len] = 0;

    string respString = _apiManager.HandleRequest(content);

    free(content);

    // ONLY FOR TESTING!!!!
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (respString.length() == 0)
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, NULL);
    else
        httpd_resp_send(req, respString.c_str(), respString.length());

    return ESP_OK;
}

void Webserver :: AddApi(ApiHandlerBase* handler)
{
    _apiManager.AddApi(handler);
}

bool Webserver :: RemoveApi(string apiId)
{
    return _apiManager.RemoveApi(apiId);
}

void Webserver :: SetSpiffsDownloadAllowCheckFn(std::function<bool(const char* fileName)> spiffsAllowCheckFn)
{
    _spiffsAllowCheckFn = spiffsAllowCheckFn;
}
