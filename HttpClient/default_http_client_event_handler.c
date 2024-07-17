// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "default_http_client_event_handler.h"
#include "esp_log.h"

static const char *TAG = "DefaultHttpEventHandler";

esp_err_t default_http_client_event_handler(esp_http_client_event_t *evt)
{
    default_http_event_handler_data* userdata = evt->user_data;

    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);

            // Check current chunk against the max len
            if (evt->data_len + userdata->curLen + 1 > userdata->maxLen)
            {
                ESP_LOGI(TAG, "Content len (received) %d exceeds max len %d", evt->data_len + userdata->curLen, userdata->maxLen);
                return ESP_FAIL;
            }          

            // copy over the data
            memcpy(userdata->content + userdata->curLen, evt->data, evt->data_len);
            userdata->curLen += evt->data_len;

            userdata->content[userdata->curLen] = 0;
            break;

        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
            break;
    }
    return ESP_OK;
}