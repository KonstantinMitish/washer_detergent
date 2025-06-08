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

// see generate_key_h.sh
#include "key.h"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include <esp_log.h>
#include <wolfssl/openssl/rsa.h>
#include <wolfssl/wolfcrypt/asn_public.h>

static const char TAG[] = "encryption";

bool encryption_md5(const byte *data, size_t size, byte *md5) {
    wc_Md5 md5_ctx;
    int ret;

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

static void hexdump(const char *name, const uint8_t *data, size_t length) {
    size_t buf_len = length * 2 + 1;
    char *line = (char *) malloc(buf_len);
    if (!line) {
        return;
    }

    for (size_t i = 0; i < length; i++) {
        snprintf(line + (i * 2), 3, "%02X", data[i]);
    }
    line[buf_len - 1] = '\0';

    ESP_LOGI(TAG, "%s: %s", name, line);
    free(line);
}

bool encryption_verify(byte *md5, const byte *signature, size_t size) {
    RSA *rsaKey = wolfSSL_RSA_new();

    if (rsaKey == NULL) {
        ESP_LOGE(TAG, "Failed to allocate key");
        return false;
    }

    const byte *n_ptr, *e_ptr;
    uint32_t idx = 0, nSz, eSz;
    int ret = wc_RsaPublicKeyDecode_ex(public_key, &idx,
                                       public_key_len,
                                       &n_ptr, &nSz, &e_ptr, &eSz);

    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to load key %i", ret);
        wolfSSL_RSA_free(rsaKey);
        return false;
    }

    ret = wolfSSL_RSA_verify(WC_MD5,
                             md5,
                             ENCRYPTION_MD5_SIZE,
                             signature, size,
                             rsaKey);

    wolfSSL_RSA_free(rsaKey);
    if (ret != 0) {
        ESP_LOGE(TAG, "Verify returned %i", ret);
        return false;
    }
    return true;
}

#define LOG_UINT64_FORMAT "0x%08X%08X"
#define LOG_UINT64_DATA(X) (uint32_t)((X) >> 32), (uint32_t) ((X) &0xFFFFFFFF)

bool encryption_extract(const byte *data, size_t size, struct payload *result) {
    static uint64_t last_ts = 0;
    const static uint64_t allowed_delta = 1000000 * 60;// 1 min
    if (size < sizeof(struct payload)) {
        ESP_LOGE(TAG, "Packet size (%u) is too short", size);
        return false;
    }

    hexdump("pakcet", data, size);

    hexdump("payload", data, sizeof(struct payload));

    byte md5[ENCRYPTION_MD5_SIZE];
    if (!encryption_md5(data, sizeof(struct payload), md5)) {
        ESP_LOGE(TAG, "Failed to calculate MD5");
        return false;
    }
    hexdump("md5", md5, sizeof(md5));

    struct timeval now_tv;
    gettimeofday(&now_tv, NULL);
    uint64_t now = (uint64_t) now_tv.tv_sec * 1000000ULL + (uint64_t) now_tv.tv_usec;

    hexdump("signature", data + sizeof(struct payload), size - sizeof(struct payload));
    if (!encryption_verify(md5, data + sizeof(struct payload), size - sizeof(struct payload))) {
        ESP_LOGE(TAG, "Failed to verify signature");
        return false;
    }
    memcpy(result, data, sizeof(struct payload));

    ESP_LOGI(TAG, "Payload command: 0x%X pin:%u time:%u timestamp:" LOG_UINT64_FORMAT, (unsigned) result->command, result->pin, result->time, LOG_UINT64_DATA(result->timestamp));
    if (result->timestamp < now - allowed_delta) {
        ESP_LOGE(TAG, "Payload is too old " LOG_UINT64_FORMAT " < " LOG_UINT64_FORMAT " - " LOG_UINT64_FORMAT,
                 LOG_UINT64_DATA(result->timestamp), LOG_UINT64_DATA(now), LOG_UINT64_DATA(allowed_delta));
        return false;
    }

    if (result->time < last_ts) {
        ESP_LOGE(TAG, "Payload is now seqential");
        return false;
    }

    last_ts = result->time;

    return true;
}