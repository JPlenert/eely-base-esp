// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "wifi.h"

#define NETIF_DESC_STA "WifiSTA"
#define CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY 10

static const char* TAG = "WifiSta";

static esp_netif_t *s_wifiSta_netif = NULL;
static SemaphoreHandle_t s_wifiSta_semph_get_ip_addrs = NULL;
#if CONFIG_EXAMPLE_CONNECT_IPV6
static SemaphoreHandle_t s_wifiSta_semph_get_ip6_addrs = NULL;
#endif
int s_wifiSta_retry_num = 0;

void wifiSta_start(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    esp_netif_config.if_desc = NETIF_DESC_STA;
    esp_netif_config.route_prio = 128;
    s_wifiSta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);

    // Set unique host name
    char hostname[25];
    wifi_generateName(hostname);
    esp_netif_set_hostname(s_wifiSta_netif, hostname);

    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void wifiSta_handler_disconnect(void *arg, esp_event_base_t event_base,
                                       int32_t event_id, void *event_data)
{
    s_wifiSta_retry_num++;

    // int delay = 10;
    // if (s_wifiSta_retry_num > 5)
    //     delay = s_wifiSta_retry_num * 50;
    // if (delay > 10000)
    //     delay = 10000;

    // ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect in %ums...", delay);
    // wifi_status_set(WifiStatus_StationWaitReconnect);
    // vTaskDelay(delay);
    // wifi_status_set(WifiStatus_StationConnecting);

    wifi_status_set(WifiStatus_StationConnecting);
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED)
    {
        return;
    }
    ESP_ERROR_CHECK(err);
}

static void wifiSta_handler_connect(void *esp_netif, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data)
{
    wifi_status_set(WifiStatus_StationWaitForIP);
#if CONFIG_EXAMPLE_CONNECT_IPV6
    esp_netif_create_ip6_linklocal(esp_netif);
#endif // CONFIG_EXAMPLE_CONNECT_IPV6
}

static void wifiSta_handler_got_ip(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data)
{
    // Check if the event is for this netif
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (strcmp(NETIF_DESC_STA, esp_netif_get_desc(event->esp_netif)) != 0)
        return;

    wifi_status_set(WifiStatus_StationOK);

    s_wifiSta_retry_num = 0;
    ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    if (s_wifiSta_semph_get_ip_addrs)
    {
        xSemaphoreGive(s_wifiSta_semph_get_ip_addrs);
    }
    else
    {
        ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
    }
}

#if CONFIG_EXAMPLE_CONNECT_IPV6
static void wifiSta_handler_got_ipv6(void *arg, esp_event_base_t event_base,
                                     int32_t event_id, void *event_data)
{
    // Check if the event is for this netif
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (strcmp(NETIF_DESC_STA, esp_netif_get_desc(event->esp_netif)) != 0)
    {
        return;
    }

    wifi_status_set(WifiStatus_StationOK);

    esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
    ESP_LOGI(TAG, "Got IPv6 event: Interface \"%s\" address: " IPV6STR ", type: %s", esp_netif_get_desc(event->esp_netif),
             IPV62STR(event->ip6_info.ip), example_ipv6_addr_types_to_str[ipv6_type]);

    if (ipv6_type == EXAMPLE_CONNECT_PREFERRED_IPV6_TYPE)
    {
        if (s_wifiSta_semph_get_ip6_addrs)
        {
            xSemaphoreGive(s_wifiSta_semph_get_ip6_addrs);
        }
        else
        {
            ESP_LOGI(TAG, "- IPv6 address: " IPV6STR ", type: %s", IPV62STR(event->ip6_info.ip), example_ipv6_addr_types_to_str[ipv6_type]);
        }
    }
}
#endif // CONFIG_EXAMPLE_CONNECT_IPV6

esp_err_t wifiSta_do_connect(wifi_config_t wifi_config)
{
    s_wifiSta_semph_get_ip_addrs = xSemaphoreCreateBinary();
    if (s_wifiSta_semph_get_ip_addrs == NULL)
    {
        return ESP_ERR_NO_MEM;
    }
#if CONFIG_EXAMPLE_CONNECT_IPV6
    s_wifiSta_semph_get_ip6_addrs = xSemaphoreCreateBinary();
    if (s_wifiSta_semph_get_ip6_addrs == NULL)
    {
        vSemaphoreDelete(s_wifiSta_semph_get_ip_addrs);
        return ESP_ERR_NO_MEM;
    }
#endif

    s_wifiSta_retry_num = 0;

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifiSta_handler_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiSta_handler_got_ip, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifiSta_handler_connect, s_wifiSta_netif));
#if CONFIG_EXAMPLE_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &wifiSta_handler_got_ipv6, NULL));
#endif

    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
        return ret;
    }
    return ESP_OK;
}

static void print_auth_mode(int authmode)
{
    switch (authmode) {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_OWE:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OWE");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_ENTERPRISE:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA3_ENT_192:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_ENT_192");
        break;
    // case WIFI_AUTH_WPA3_EXT_PSK:
    //     ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_EXT_PSK");
    //     break;
    default:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
    switch (pairwise_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    case WIFI_CIPHER_TYPE_AES_CMAC128:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_AES_CMAC128");
        break;
    case WIFI_CIPHER_TYPE_SMS4:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_SMS4");
        break;
    case WIFI_CIPHER_TYPE_GCMP:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP");
        break;
    case WIFI_CIPHER_TYPE_GCMP256:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP256");
        break;
    default:
        ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }

    switch (group_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    case WIFI_CIPHER_TYPE_SMS4:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_SMS4");
        break;
    case WIFI_CIPHER_TYPE_GCMP:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP");
        break;
    case WIFI_CIPHER_TYPE_GCMP256:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP256");
        break;
    default:
        ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }
}


void wifiSta_Scan()
{
    #define DEFAULT_SCAN_LIST_SIZE 30
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);
    for (int i = 0; i < number; i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        ESP_LOGI(TAG, "Country \t\t%s", ap_info[i].country.cc);
        print_auth_mode(ap_info[i].authmode);
        if (ap_info[i].authmode != WIFI_AUTH_WEP) {
            print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
        }
        ESP_LOGI(TAG, "Channel \t\t%d", ap_info[i].primary);
    }
}

esp_err_t wifiSta_do_connect_by_ssid(const char *ssid, const char *password)
{
    esp_err_t err;

    wifiSta_start();

    // wifiSta_Scan();

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN,
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    wifi_config.sta.bssid_set = false;

    // Old Settings (0.8)
    // wifi_config.sta.scan_method = WIFI_FAST_SCAN;
    // wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    // wifi_config.sta.threshold.rssi = -127;
    // wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;

    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);

    err = wifiSta_do_connect(wifi_config);
    if (err != ESP_OK)
        return err;

    ESP_LOGI(TAG, "Waiting for IP(s)");
    xSemaphoreTake(s_wifiSta_semph_get_ip_addrs, portMAX_DELAY);
#if CONFIG_EXAMPLE_CONNECT_IPV6
    xSemaphoreTake(s_wifiSta_semph_get_ip6_addrs, portMAX_DELAY);
#endif
    if (s_wifiSta_retry_num > CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY)
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}
