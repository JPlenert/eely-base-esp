// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu

// Information regarding captiva portals
// https://github.com/espressif/esp-protocols/blob/master/components/mdns/examples/main/mdns_example_main.c
// https://de.wikipedia.org/wiki/Captive_Portal
// https://github.com/zhouhan0126/DNSServer---esp32/blob/master/examples/CaptivePortalAdvanced/CaptivePortalAdvanced.ino
// https://www.az-delivery.de/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/captive-portal-fuer-den-esp32-teil-2
// https://www.smartlab.at/implement-an-esp32-hot-spot-that-runs-a-captive-portal/
// https://github.com/esp8266/Arduino/blob/master/libraries/DNSServer/src/DNSServer.cpp
// https://gitlab.com/defcronyke/wifi-captive-portal-esp-idf/-/blob/master/components/wifi-captive-portal-esp-idf-component

#include <sys/time.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "esp_netif.h"
#include "DnsServer.h"
#include "endian.h"
#include "wifi.h"
#include "DnsServer.h"

static const char *TAG = "dnsServer";

#define DNS_LEN 512

#define DNS_FLAG_QR (1 << 7)
#define DNS_FLAG_AA (1 << 2)
#define DNS_FLAG_TC (1 << 1)
#define DNS_FLAG_RD (1 << 0)

#define DNS_QTYPE_A 1
#define DNS_QTYPE_NS 2
#define DNS_QTYPE_CNAME 5
#define DNS_QTYPE_SOA 6
#define DNS_QTYPE_WKS 11
#define DNS_QTYPE_PTR 12
#define DNS_QTYPE_HINFO 13
#define DNS_QTYPE_MINFO 14
#define DNS_QTYPE_MX 15
#define DNS_QTYPE_TXT 16
#define DNS_QTYPE_URI 256

#define DNS_QCLASS_IN 1
#define DNS_QCLASS_ANY 255
#define DNS_QCLASS_URI 256

struct __attribute__((packed)) DnsHeader
{
  uint16_t id;
  uint8_t flags;
  uint8_t rcode;
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
};

struct __attribute__((packed)) DnsQuestionFooter
{
  uint16_t type;
  uint16_t cl;
};

struct __attribute__((packed)) DnsResourceFooter
{
  uint16_t type;
  uint16_t cl;
  uint32_t ttl;
  uint16_t rdlength;
};

void DnsServer_vTaskCode(void* pvParameters)
{
    ((DnsServer*)pvParameters)->TaskCode();
}

void DnsServer :: Start()
{
    xTaskCreate(DnsServer_vTaskCode, "DnsServer", 10*2024, this, tskIDLE_PRIORITY, &_taskHandle);
}

void DnsServer :: TaskCode()
{
    struct sockaddr_in server_addr;
    uint32_t ret;
    struct sockaddr_in from;
    socklen_t fromlen;
    char udp_msg[DNS_LEN];

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(53);
    server_addr.sin_len = sizeof(server_addr);

    do
    {
        _socket_handle = socket(AF_INET, SOCK_DGRAM, 0);
        if (_socket_handle == -1)
        {
            ESP_LOGI(TAG, "Dns: failed to create socket");
            vTaskDelay(1000 /  portTICK_PERIOD_MS);
        }
    } while (_socket_handle == -1);

    do
    {
        ret = bind(_socket_handle, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret != 0)
        {
            ESP_LOGI(TAG, "Dns: failed to bind socket");
            vTaskDelay(1000 /  portTICK_PERIOD_MS);
        }
    } while (ret != 0);

    ESP_LOGI(TAG, "Dns: initialized");

    while (1)
    {
        memset(&from, 0, sizeof(from));
        fromlen = sizeof(struct sockaddr_in);
        ret = recvfrom(_socket_handle, (uint8_t *)udp_msg, DNS_LEN, 0, (struct sockaddr *)&from, (socklen_t *)&fromlen);
        if (ret > 0)
            HandleDnsMessage(&from, udp_msg, ret);
    }

    // You'll never come here!
    close(_socket_handle);
    vTaskDelete(NULL);
}

int DnsServer :: DecodeLabels(char* questionLabelStart, char* displayBuffer)
{
    char* ptr = questionLabelStart;
    int qNameLen;

    while (true)
    {
        qNameLen = *ptr;

        // Decode label
        // RFC 1035 - 4.1.4. Message compression ?
        if ((qNameLen & 0xC0) == 0xC0)
        {
            qNameLen = be16dec(ptr) & 0x3FFF;
            ptr += 2;
        }
        if (qNameLen == 0)
        {
            *displayBuffer++ = 0;
            ptr++;
            break;
        }
        else
        {
            ptr++;
        }

        memcpy(displayBuffer, ptr, qNameLen);
        displayBuffer += qNameLen;
        *displayBuffer++ = '.';

        ptr += qNameLen;
    }

    return ptr - questionLabelStart;
}

// Receive a DNS packet and maybe send a response back
void DnsServer :: HandleDnsMessage(struct sockaddr_in *premote_addr, char *request, unsigned short length)
{
    // Reply may be double the size of the request, as we more or less copy the questions
    char reply[DNS_LEN*2];
    char hostDisplayBuffer[DNS_LEN];

    char *request_ptr = request;
    char *reply_ptr = reply;

    DnsHeader *request_hdr = (DnsHeader *)request_ptr;
    DnsHeader *reply_hdr = (DnsHeader *)reply_ptr;
    request_ptr += sizeof(DnsHeader);

    // Plausibility checks
    if (
        // Message to long for buffer
        length > DNS_LEN || 
        // Message to small
        length < sizeof(DnsHeader) ||
        // Message has answers -> can't be an request
        request_hdr->ancount != 0 || request_hdr->nscount != 0|| request_hdr->arcount != 0 || 
        request_hdr->flags & DNS_FLAG_QR ||
        // Message is truncated -> not supported
        request_hdr->flags & DNS_FLAG_TC)
        return;

    // Copy the request, as the reply is the request with appended answers
    memcpy(reply, request, length);
    reply_hdr->flags |= DNS_FLAG_QR;
    // Add the answers to the end of the packet
    reply_ptr += length;

    for (int qIdx = 0; qIdx < be16dec(&request_hdr->qdcount); qIdx++)
    {
        char *qNameLabelStart = request_ptr;
        int qNameLabelLen = DecodeLabels(qNameLabelStart, hostDisplayBuffer);

        // Copy name to answer
        memcpy(reply_ptr, qNameLabelStart, qNameLabelLen);
        request_ptr += qNameLabelLen;
        reply_ptr += qNameLabelLen;

        // Decode footer
        DnsQuestionFooter *qf = (DnsQuestionFooter *)request_ptr;
        request_ptr += sizeof(DnsQuestionFooter);

        ESP_LOGI(TAG, "Dns: Got question with Type=%u and Class=%u for %s\n", be16dec(&qf->type), be16dec(&qf->cl), hostDisplayBuffer);

        // We always give the same answer - no matter what type of question - our IP-Address
        DnsResourceFooter *rf = (DnsResourceFooter *)reply_ptr;

        be16enc(&rf->type, DNS_QTYPE_A);
        be16enc(&rf->cl, DNS_QCLASS_IN);
        be32enc(&rf->ttl, 0);
        be16enc(&rf->rdlength, sizeof(uint32_t)); // IPv4 addr is 4 bytes;
        reply_ptr += sizeof(DnsResourceFooter);

        *((uint32_t*)reply_ptr) = wifiAp_get_ip_address(); // info.ip.addr;
        reply_ptr += sizeof(uint32_t);

        be16enc(&reply_hdr->ancount, be16dec(&reply_hdr->ancount) + 1);
    }

    // Send the response
    sendto(_socket_handle, (uint8_t *)reply, reply_ptr - reply, 0, (struct sockaddr *)premote_addr, sizeof(struct sockaddr_in));
}
