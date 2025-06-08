/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : sockets.c
 * PURPOSE     : UNIX sockets module
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 04.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */
#include "sockets.h"

#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

static const char TAG[] = "sockets";

bool socket_has_data(SOCKET sock) {
    fd_set set;
    struct timeval time = {0};

    FD_ZERO(&set);
    FD_SET(sock, &set);
    int val = select(sock + 1, &set, NULL, NULL, &time);
    if (val < 0) {
        ESP_LOGE(TAG, "select() error: %i", errno);
        return false;
    }
    return val > 0;
}

bool socket_recvfrom(SOCKET s, char *buf, int *len, IP *ip) {
    struct sockaddr_in addr = {0};
    socklen_t addr_len = sizeof(addr);
    *len = recvfrom(s, buf, *len, 0, (struct sockaddr *) &addr, &addr_len);
    if (*len < 0) {
        return false;
    }
    *ip = addr.sin_addr.s_addr;
    return true;
}

SOCKET socket_udp(int port) {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        shutdown(sock, 0);
        close(sock);
        return -1;
    }

    return sock;
}

SOCKET socket_tcp(int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        shutdown(sock, 0);
        close(sock);
        return -1;
    }

    if (listen(sock, 5) == -1) {
        shutdown(sock, 0);
        close(sock);
        return -1;
    }

    return sock;
}

SOCKET socket_accept(SOCKET s, IP *ip) {
    struct sockaddr_in addr = {0};
    size_t size = sizeof(addr);

    SOCKET c = accept(s, (struct sockaddr *) &addr, &size);

    if (c < 0) {
        ESP_LOGE(TAG, "Accept error %d", errno);
        return c;
    }
    ESP_LOGI(TAG, "New connection " IP_FORMAT, IP_FORMAT_DATA(addr.sin_addr.s_addr));

    if (ip != NULL)
        *ip = addr.sin_addr.s_addr;
    return c;
}

bool socket_recv(SOCKET s, char *buf, int *len) {
    *len = recv(s, buf, *len, 0);
    if (*len < 0) {
        return false;
    }
    return true;
}


//bool socket_recv(SOCKET s, char *buf, int *len) {
//    int totalBytes = 0;
//    int bytesRead = 0;
//
//    do {
//        bytesRead = recv(s, buf + totalBytes, *len - totalBytes, 0);
//        if (bytesRead < 0) {
//            return false;
//        }
//        totalBytes += bytesRead;
//    } while (bytesRead != 0);
//
//    *len = totalBytes;
//    return true;
//}

bool socket_send(SOCKET s, const char *buf, int len) {
    if (send(s, buf, len, 0) < 0) {
        return false;
    }
    return true;
}

void socket_close(SOCKET s) {
    shutdown(s, 0);
    close(s);
}