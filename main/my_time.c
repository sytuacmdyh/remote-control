#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

static const char *TAG = "time_sync";

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void initialize_sntp(void)
{
    // ESP_ERROR_CHECK(nvs_flash_init());
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();

    // wait for time to be set
    // time_t now = 0;
    // struct tm timeinfo = {0};
    // int retry = 0;
    // const int retry_count = 10;
    // while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    // {
    //     ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    //     vTaskDelay(2000 / portTICK_PERIOD_MS);
    // }
    // time(&now);
    // localtime_r(&now, &timeinfo);

    // Set timezone to China Standard Time
    // char strftime_buf[64];
    setenv("TZ", "CST-8", 1);
    tzset();
    // localtime_r(&now, &timeinfo);
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    // ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
}
