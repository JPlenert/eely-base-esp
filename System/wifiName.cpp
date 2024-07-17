// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "esp_wifi.h"
#include "esp_mac.h"

void wifi_generateName(char hostname[25])
{
    // Set unique host name
    uint8_t mac_addr[6] = {0};
    esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);
    sprintf(hostname, "eely_%02X%02X%02X", mac_addr[3], mac_addr[4], mac_addr[5]);
}