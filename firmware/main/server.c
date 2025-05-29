#include "server.h"

#include "esp_log.h"

#include "sockets.h"

static struct {
    SOCKET udp_socket;
    SOCKET tcp_socket;
} server = {0};

#define PORT_BROADCAST 30239
#define PORT_WORK 30240

static const char TAG[] = "server";

bool server_init() {
    server.udp_socket = socket_udp(PORT_BROADCAST);
    if (server.udp_socket < 0) {
        return false;
    }
    return true;
}

bool server_response() {
    if (!socket_has_data(server.udp_socket))
    {
        return true;
    }

    char buf[239];
    IP ip;
    int size = sizeof(buf);
    if (!socket_recvfrom(server.udp_socket, buf, &size, &ip)) {
        ESP_LOGI(TAG, "Recvfrom error");
        return false;
    }
    ESP_LOGI(TAG, "RECV from " IP_FORMAT, IP_FORMAT_DATA(ip));
    
    // if (server.tcp_socket) close();
    server.tcp_socket = socket_tcp(ip, PORT_WORK);
    if (server.tcp_socket < 0) {
        return false;
    }

    static char response[] = "how's mom?";

    if (!socket_send(server.tcp_socket, response, sizeof(response))) {
        ESP_LOGE(TAG, "Send error");
        return false;
    }

    return true;
}
