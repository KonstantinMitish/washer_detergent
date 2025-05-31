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
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "wifi.h"
#include "sntp.h"
#include "server.h"

static const char TAG[] = "main";

int app_main(void) {
    ESP_LOGI(TAG, "Hello world!");
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_NUM_2, 1);
    wifi_connect();
    sntp_run();
    if (!server_init()) {
        ESP_LOGE(TAG, "Failed to initialize server");
    }

    while (1) {
        if (!server_response()) {
            ESP_LOGE(TAG, "Server error");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    return 0;
}