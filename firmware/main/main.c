/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : main.c
 * PURPOSE     : Entry point module.
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 29.05.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "wifi.h"
#include "server.h"

static const char TAG[] = "main";

int app_main(void) {
    ESP_LOGI(TAG, "Hello world!");
    wifi_connect();
    server_init();

    while (1) {
        server_response();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    return 0;
}