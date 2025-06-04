/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : encryption.c
 * PURPOSE     : Encryption module.
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 04.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */
#include "encryption.h"

#include <esp_log.h>


static const char TAG[] = "encryption";

bool encryption_md5(const byte *data, size_t size, byte *md5) {
    wc_Md5     md5_ctx;
    int        ret;

    ret = wc_InitMd5(&md5_ctx);
    if (ret != 0) {
        ESP_LOGE(TAG, "wc_InitMd5 failed, error %d", ret);
        return false;
    }

    ret = wc_Md5Update(&md5_ctx, data, size);
    if (ret != 0) {
        ESP_LOGE(TAG, "wc_Md5Update failed, error %d", ret);
        return false;
    }

    ret = wc_Md5Final(&md5_ctx, md5);
    if (ret != 0) {
        ESP_LOGE(TAG, "wc_Md5Final failed, error %d", ret);
        return false;
    }

    return true;
}