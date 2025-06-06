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
#include "encryption.h"
#include "server.h"

static const char TAG[] = "main";

int app_main(void) {
    byte md5[ENCRYPTION_MD5_SIZE];
    encryption_md5((const byte *)"jopaslona", 9, md5);
    ESP_LOGI(TAG, "MD5: %2X%2X%2X%2X", md5[0], md5[1], md5[2], md5[3]);
    ESP_LOGI(TAG, "Hello world!");
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_NUM_2, 1);
    wifi_connect();
    sntp_run();
    storage_init();

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