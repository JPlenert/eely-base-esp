// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "nvs.h"

TimerHandle_t g_factory_reset_handle;

static const char *TAG = "factory_reset";

void factory_reset_writenvs(nvs_handle_t hNvs, int8_t newVal)
{
    if (hNvs == 0)
        ESP_ERROR_CHECK(nvs_open("eely", NVS_READWRITE, &hNvs));
    ESP_ERROR_CHECK(nvs_set_i8(hNvs, "resetCou", newVal));
    nvs_close(hNvs);
}

void factory_reset_handler( TimerHandle_t xTimer )
{
    //ESP_LOGI("factory_reset", "Resetting recommendation");
    ESP_LOGI(TAG, "Resetting recommendation");
    factory_reset_writenvs(0, 0);
    xTimerDelete(g_factory_reset_handle, 10);
}

bool factory_reset_check()
{
    nvs_handle_t hNvs;
    int8_t resetCou;

    ESP_ERROR_CHECK(nvs_open("eely", NVS_READWRITE, &hNvs));
    if (nvs_get_i8(hNvs, "resetCou", &resetCou) != ESP_OK)
        resetCou = 0;

    resetCou++;
    ESP_LOGI(TAG, "Current count %d", resetCou);

    if (resetCou >= 5)
        resetCou = 0;

    factory_reset_writenvs(hNvs, resetCou);

    if (resetCou != 0)
    {
        g_factory_reset_handle = xTimerCreate
                        ( /* Just a text name, not used by the RTOS
                            kernel. */
                            "factory_reset_handler",
                            /* The timer period in ticks, must be
                            greater than 0. */
                            10000 / portTICK_PERIOD_MS,
                            /* The timers will auto-reload themselves
                            when they expire. */
                            pdFALSE,
                            /* The ID is used to store a count of the
                            number of times the timer has expired, which
                            is initialised to 0. */
                            ( void * ) 0,
                            /* Each timer calls the same callback when
                            it expires. */
                            factory_reset_handler
                        );

        xTimerStart(g_factory_reset_handle, 0);

        return false;
    }

    ESP_LOGI(TAG, "Recommending factory reset");
    return true;
}
