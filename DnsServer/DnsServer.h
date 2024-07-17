// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef webserver_h
#define webserver_h

#include <esp_http_server.h>
#include "esp_log.h"
#include <functional>
#include "ApiManager.h"

// Simple CaptivePortal-DNS-Server
// Will answer to DNS requests always with the IP-Address of the AP
class DnsServer
{
    private:
        int _socket_handle;
        TaskHandle_t _taskHandle;

    public:
        DnsServer() : _socket_handle(0), _taskHandle(nullptr) {};
        void Start();

        void TaskCode();

    private: 
        void HandleDnsMessage(struct sockaddr_in *premote_addr, char *request, unsigned short length);
        int DecodeLabels(char* questionLabelStart, char* displayBuffer);

};

#endif