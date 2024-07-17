// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef SystemTime_h
#define SystemTime_h

#include <string>
#include <vector>
#include <functional>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

using namespace std;

class SystemTime
{
    protected:
        std::vector<std::function<void()>> _timeInitInform;
        bool _timeWasSet;
        SemaphoreHandle_t _lock;

    public:
        SystemTime();
        void EnableSNTP();
        void AddTimeInitInfo(std::function<void()> inform);

        // Only for internal use
        void SntpNotifyCallback(struct timeval *tv);

        static SystemTime* GetInstance();
};

#endif