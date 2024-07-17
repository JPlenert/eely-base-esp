// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "SystemTime.h"
#include "esp_sntp.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Eelybase.h"

SystemTime* g_SystemTimeInstace;
static const char *TAG = "SystemTime";

SystemTime :: SystemTime()
{
    _timeWasSet = false;

    // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    // Europe with DST
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();

    _lock = xSemaphoreCreateBinary();
    xSemaphoreGive(_lock);
}

void SystemTime :: AddTimeInitInfo(std::function<void()> inform)
{
    xSemaphoreTake(_lock, (TickType_t) 10 );

    _timeInitInform.push_back(inform);
    // If time was already set, inform now
    if (_timeWasSet)
        inform();

    xSemaphoreGive(_lock);
}

void SystemTime :: EnableSNTP()
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    sntp_setservername(0, "ptbtime1.ptb.de");
#if SNTP_MAX_SERVERS > 1
    sntp_setservername(1, "pool.ntp.org");     // set the secondary NTP server (will be used only if SNTP_MAX_SERVERS > 1)
#endif

    // Register for callback
    g_SystemTimeInstace = this;

    sntp_set_time_sync_notification_cb([](struct timeval *tv) {
        g_SystemTimeInstace->SntpNotifyCallback(tv);
     });

    sntp_init();
}

void SystemTime :: SntpNotifyCallback(struct timeval *tv)
{
    if (!_timeWasSet && sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED)
    {
        ESP_LOGI(TAG, "Got inital time by SNTP");

        time_t now;
        struct tm timeinfo;
        char strftime_buf[50];

        time(&now);
        gmtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current system date/time after sync is: %s", strftime_buf);

        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current local date/time after sync is: %s", strftime_buf);
        
        xSemaphoreTake(_lock, (TickType_t) 10 );

        _timeWasSet = true;
        for (auto &cb : _timeInitInform)
        {
            cb();
        }

        xSemaphoreGive(_lock);        
    }
    else
        ESP_LOGI(TAG, "Got additional time by SNTP");
}

SystemTime* SystemTime :: GetInstance()
{
    return (SystemTime*)g_eelybase->GetFeature(g_eelybase->FEATKEY_SYSTEMTIME);
}
