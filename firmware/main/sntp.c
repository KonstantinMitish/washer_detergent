/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : sntp.c
 * PURPOSE     : Time sync module.
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 04.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */
#include "sntp.h"

#include <time.h>

#include "esp_log.h"
#include "esp_sntp.h"

static const char TAG[] = "sntp";

static void sntp_notification(struct timeval *tv) {
    time_t now = 0;
    time(&now);
    ESP_LOGI(TAG, "Time synchronized %li", now);
}

void sntp_run(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "time.nist.gov");
    sntp_set_time_sync_notification_cb(sntp_notification);
    sntp_init();
}