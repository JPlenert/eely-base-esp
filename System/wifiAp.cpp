// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "string.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "dhcpserver/dhcpserver.h"
#include "dhcpserver/dhcpserver_options.h"
#include "esp_mac.h"

#include "wifi.h"

static const char *TAG = "wifi softAP";
uint32_t g_wifi_ap_ipaddr = 0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifiAp_init(int channel)
{
    esp_netif_t* netif = esp_netif_create_default_wifi_ap();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));


    uint8_t off = 0;
    esp_netif_dhcps_option(netif, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &off, 1);    

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    wifi_config.ap.channel = channel;
    wifi_config.ap.max_connection = 2;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    char ssid[25];
    wifi_generateName(ssid);
    strcpy((char*)wifi_config.ap.ssid, ssid);

    wifi_config.ap.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s channel:%d",
             ssid, wifi_config.ap.channel);

    // Get the IP-Address
    esp_netif_ip_info_t info;
    esp_netif_get_ip_info(esp_netif_next(NULL), &info);
    g_wifi_ap_ipaddr = info.ip.addr;
}

uint32_t wifiAp_get_ip_address()
{
    return g_wifi_ap_ipaddr;
}
