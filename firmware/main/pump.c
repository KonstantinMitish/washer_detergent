/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : pump.c
 * PURPOSE     : Pump logic handler
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 06.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */
#include "pump.h"

#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "storage.h"

static const char TAG[] = "pump";

#define NVS_KEY(PIN) char nvs_key[] = "pump_A"; \
        nvs_key[sizeof(nvs_key) - 2] += (PIN)

static const int pin_to_gpio[] = {
    GPIO_NUM_16,
    GPIO_NUM_5,
    GPIO_NUM_4,
    GPIO_NUM_0,
    GPIO_NUM_2,
    GPIO_NUM_14,
    GPIO_NUM_12,
    GPIO_NUM_13,
};
static const size_t pins_count = sizeof(pin_to_gpio) / sizeof(pin_to_gpio[0]);

/**
 * @brief  Инициализирует все “безопасные” GPIO из списка как OUTPUT и сразу сбрасывает в 0.
 * @return true  — успешно, false — произошла ошибка конфигурации хотя бы одного пина.
 */
bool pump_init() {
    esp_err_t err;

    for (size_t idx = 0; idx < pins_count; ++idx) {
        gpio_num_t gpio_pin = pin_to_gpio[idx];

        err = gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "gpio_config failed for GPIO%d: %s",
                     gpio_pin, esp_err_to_name(err));
            return false;
        }

        err = gpio_set_level(gpio_pin, 0);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "gpio_set_level(0) failed for GPIO%d: %s",
                     gpio_pin, esp_err_to_name(err));
            return false;
        }
    }

    ESP_LOGI(TAG, "pump_init: configured %d GPIOs (with pull-down) and set to LOW",
             (int)pins_count);
    return true;
}

bool pump_callibrate(int pin, double new_speed) {
    NVS_KEY(pin);
    struct pump_data pump;
    pump.speed = new_speed;

    if (!storage_write(nvs_key, (const void *)&pump, sizeof(pump)))
    {
        ESP_LOGE(TAG, "Can't write to NVS");
        return false;
    }
    
    ESP_LOGE(TAG, "Pump %i updated", pin);
    return true;
}

bool pump_work_time(int pin, uint32_t time_ms) {
    if (pin < 0 || (size_t)pin >= pins_count) {
        ESP_LOGE(TAG, "Invalid pin index %d", pin);
        return false;
    }

    ESP_LOGI(TAG, "Pump on %i pin will work for %ums", pin, time_ms);

    int gpio_pin =  pin_to_gpio[pin];
    ESP_LOGI(TAG, "GPIO_NUM_%i for pin %i", gpio_pin, pin);

    if (gpio_set_level(gpio_pin, 1) != 0) {
        ESP_LOGE(TAG, "Can't turn pin %i on", pin);
        return false;
    }
    ESP_LOGI(TAG, "Pump %i turned on", pin);
    vTaskDelay(time_ms / portTICK_PERIOD_MS);
    if (gpio_set_level(gpio_pin, 0) != 0) {
        ESP_LOGE(TAG, "Can't turn pin %i off", pin);
        ESP_LOGE(TAG, "Situation pizdec, force reseting");
        esp_restart();
        return false;
    }
    ESP_LOGI(TAG, "Pump %i turned off", pin);
    return true;
}

bool pump_work_volume(int pin, double volume) {
    NVS_KEY(pin);
    struct pump_data pump;
    size_t size = sizeof(pump);

    if (!storage_read(nvs_key, (void *)&pump, &size))
    {
        ESP_LOGE(TAG, "Can't read pump %i data", pin);
        return false;
    }

    if (size != sizeof(pump)) {
        ESP_LOGE(TAG, "NVS data size does not match, please re-callibrate");
        return false;
    }
    
    uint32_t time = (uint32_t)(pump.speed * volume);

    ESP_LOGI(TAG, "Calculated time: %u", time);

    return pump_work_time(pin, time);
}