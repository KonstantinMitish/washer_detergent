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
    select(sock + 1, &set, NULL, NULL, &time);

    return FD_ISSET(sock, &set);
}

bool socket_recvfrom(SOCKET s, char *buf, int *len, IP *ip) 
{
    struct sockaddr_in addr = {0};
    socklen_t addr_len = sizeof(addr);
    *len = recvfrom(s, buf, *len, 0, (struct sockaddr *)&addr, &addr_len);
    if (*len < 0) {
        return false;
    }
    *ip = addr.sin_addr.s_addr;
    return true;
}

SOCKET socket_udp(int port)
{
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        shutdown(sock, 0);
        close(sock);
        return -1;
    }

    return sock;
}

SOCKET socket_tcp(IP ip, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = ip;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        shutdown(sock, 0);
        close(sock);
        return -1;
    }

    return sock;
}

bool socket_send(SOCKET s, char *buf, int len) {
    if (send(s, buf, len, 0) < 0) {
        return false;
    }
    return true;
}