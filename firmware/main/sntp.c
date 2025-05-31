#include "sntp.h"

#include <time.h>

#include "esp_sntp.h"
#include "esp_log.h"

static const char TAG[] = "sntp";

static void sntp_notification(struct timeval *tv)
{
    time_t now = 0;
    time(&now);
    ESP_LOGI(TAG, "Time synchronized %li", now);
}

void sntp_run(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "time.nist.gov");
    sntp_set_time_sync_notification_cb(sntp_notification);
    sntp_init();
}