/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : storage.c
 * PURPOSE     : NVS storage handle
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 06.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */

#include "storage.h"

#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "storage";
#define NVS_NAMESPACE "storage"// Namespace in NVS; must match across functions

/**
 * @brief  Initialize NVS. Must be called once at startup before any NVS operations.
 */
void storage_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // If NVS is full or a new version is found, erase and re-initialize
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "NVS initialized");
}

/**
 * @brief  Write a blob (binary data) to NVS under the specified key.
 * @param  key    Null-terminated string for the key.
 * @param  data   Pointer to the buffer containing data to store.
 * @param  size   Length of the buffer in bytes.
 * @return ESP_OK on success, otherwise an ESP error code.
 */
bool storage_write(const char *key, const void *data, size_t size) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open (%s) failed: %s", NVS_NAMESPACE, esp_err_to_name(err));
        return false;
    }

    err = nvs_set_blob(handle, key, data, size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_blob (%s) failed: %s", key, esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Blob written: key=\"%s\", size=%d bytes", key, (int) size);
    }

    nvs_close(handle);
    return true;
}

/**
 * @brief  Read a blob (binary data) from NVS by key.
 * @param  key            Null-terminated string for the key.
 * @param  out_data       Buffer to receive the data.
 * @param  inout_size     On entry: size of out_data buffer; on exit: actual bytes read.
 * @return ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if key not found, 
 *         ESP_ERR_NVS_INVALID_LENGTH if buffer is too small, or another ESP error.
 */
bool storage_read(const char *key, void *out_data, size_t *inout_size) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open (%s) failed: %s", NVS_NAMESPACE, esp_err_to_name(err));
        return false;
    }

    // First find out required size
    size_t required_size = 0;
    err = nvs_get_blob(handle, key, NULL, &required_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key \"%s\" not found", key);
        nvs_close(handle);
        return false;
    } else if (err != ESP_OK && err != ESP_ERR_NVS_INVALID_LENGTH) {
        ESP_LOGE(TAG, "nvs_get_blob (size) failed: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }

    if (*inout_size < required_size) {
        *inout_size = required_size;
        ESP_LOGW(TAG, "Buffer too small, need %d bytes", (int) required_size);
        nvs_close(handle);
        return false;
    }

    // Read the data
    err = nvs_get_blob(handle, key, out_data, inout_size);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Blob read: key=\"%s\", size=%d bytes", key, (int) *inout_size);
    } else {
        ESP_LOGE(TAG, "nvs_get_blob failed: %s", esp_err_to_name(err));
    }

    nvs_close(handle);
    return true;
}