idf_component_register(
    SRCS main.c wifi.c sockets.c server.c sntp.c encryption.c storage.c pump.c
    INCLUDE_DIRS ""
    REQUIRES "esp-wolfssl" "nvs_flash" "pthread"
)