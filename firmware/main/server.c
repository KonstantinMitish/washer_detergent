/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : server.c
 * PURPOSE     : Server logic module.
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 04.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */

#include "server.h"

#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "encryption.h"
#include "pump.h"
#include "secret.h"
#include "sockets.h"

SOCKET server_socket;

#define SERVER_PORT 30239

#define BUF_SIZE 512

#define BUILTIN_LED GPIO_NUM_2

static const char TAG[] = "server";

bool server_init() {
    server_socket = socket_tcp(SERVER_PORT);
    if (server_socket < 0) {
        return false;
    }
    ESP_LOGI(TAG, "Opened server socket %i", server_socket);
    return true;
}

static bool server_execute(const struct payload *data) {
    switch (data->command) {
        case CMD_PUMP_WORK_VOLUME:
            return pump_work_volume(data->pin, data->volume);
        case CMD_PUMP_WORK_TIME:
            return pump_work_time(data->pin, data->time);
        case CMD_PUMP_CALLIBRATE:
            return pump_callibrate(data->pin, data->volume);
    }
    ESP_LOGE(TAG, "Unknown command 0x%X", (unsigned) data->command);
    return false;
}

bool server_response() {
    static byte buf[BUF_SIZE] = {0};
    int size = BUF_SIZE;

    if (!socket_has_data(server_socket)) {
        return true;
    }

    SOCKET c = socket_accept(server_socket, NULL);


    if (!socket_has_data(c)) {
        ESP_LOGI(TAG, "No data");
        socket_close(c);
        return false;
    }

    if (!socket_recv(c, (char *) buf, &size)) {
        ESP_LOGI(TAG, "Recv error");
        return false;
    }

    ESP_LOGI(TAG, "Recv %i bytes", size);

    struct payload packet;

    if (!encryption_extract(buf, size, &packet)) {
        ESP_LOGE(TAG, "Failed to verify payload");
        socket_send(c, "\xFF", 1);
        return false;
    }

    ESP_LOGI(TAG, "Payload command: 0x%X pin:%u volume:%f time:%u", (unsigned) packet.command, packet.pin, packet.volume, packet.time);
    socket_send(c, "\0", 1);
    return server_execute(&packet);
}
