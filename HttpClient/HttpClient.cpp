// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "HttpClient.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "default_http_client_event_handler.h"

static const char *TAG = "HttpClient";

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_client.html
// https://github.com/espressif/esp-idf/blob/master/examples/protocols/esp_http_client/main/esp_http_client_example.c

HttpClient :: HttpClient()
{
    // Any url must be set at init of the client
    _requestUrl = "http://127.0.0.1";
    Init();
}

HttpClient :: HttpClient(string url)
{
    _requestUrl = url;
    Init();
}

HttpClient :: ~HttpClient()
{
    Cleanup();
}

void HttpClient :: Init()
{
    _httpClient = nullptr;

    memset(&_httpConfig, 0, sizeof(_httpConfig));
    _httpConfig.url = _requestUrl.c_str();
    _httpConfig.event_handler = default_http_client_event_handler;
    _httpConfig.user_data = NULL;
    _httpConfig.disable_auto_redirect = true;
    _httpConfig.crt_bundle_attach = esp_crt_bundle_attach;
    _httpConfig.disable_auto_redirect = true;
    _httpClient = esp_http_client_init(&_httpConfig);
}

void HttpClient :: Cleanup()
{
    esp_http_client_cleanup(_httpClient);
}

void HttpClient :: SetUrl(string url) 
{ 
    _requestUrl = url;
    esp_http_client_set_url(_httpClient, _requestUrl.c_str());
    ESP_LOGI(TAG, "Url %s", _requestUrl.c_str());
}

void HttpClient :: SetBasicAuth(string user, string password)
{
    _authUser = user;
    _authPassword = password;

    esp_http_client_set_username(_httpClient, _authUser.c_str());
    esp_http_client_set_password(_httpClient, _authPassword.c_str());
    esp_http_client_set_authtype(_httpClient, HTTP_AUTH_TYPE_BASIC);
}

void HttpClient :: SetDigestAuth(string user, string password)
{
    _authUser = user;
    _authPassword = password;

    esp_http_client_set_username(_httpClient, _authUser.c_str());
    esp_http_client_set_password(_httpClient, _authPassword.c_str());
    esp_http_client_set_authtype(_httpClient, HTTP_AUTH_TYPE_DIGEST);
}

void HttpClient :: AddRequestHeader(string key, string value)
{
    esp_http_client_set_header(_httpClient, key.c_str(), value.c_str());
}

int HttpClient :: ExecuteRequest(const char* post_data, char *result_buffer, int result_buffer_len)
{
    int retVal;

    // Create, init and set user data to http_client
    default_http_event_handler_data udata;
    memset(&udata, 0, sizeof(udata));
    udata.maxLen = result_buffer_len;
    udata.content = result_buffer;
    esp_http_client_set_user_data(_httpClient, &udata);

    // check for post
    if (post_data != NULL)
        esp_http_client_set_post_field(_httpClient, post_data, strlen(post_data));
    else
        esp_http_client_set_post_field(_httpClient, NULL, 0);

    // Make request
    esp_err_t err = esp_http_client_perform(_httpClient);
    // ToDo: Check error & status code -> fill _lastError
    if (err == ESP_OK) 
    {
        ESP_LOGD(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(_httpClient),
                udata.curLen);
        retVal = udata.curLen;
    } else {        
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
        retVal = -1;
    }

    esp_http_client_close(_httpClient);
    return retVal;
}

int HttpClient :: Get(char *result_buffer, int result_buffer_len)
{
    esp_http_client_set_method(_httpClient, HTTP_METHOD_GET);
    return ExecuteRequest(NULL, result_buffer, result_buffer_len);
}

int HttpClient :: Post(const char* post_data, char *result_buffer, int result_buffer_len)
{
    esp_http_client_set_method(_httpClient, HTTP_METHOD_POST);
    return ExecuteRequest(post_data, result_buffer, result_buffer_len);
}
