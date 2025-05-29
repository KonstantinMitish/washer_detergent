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

#include "wifi.h"

int app_main(void) {
    printf("Hello world!\n");
    wifi_connect();
    return 0;
}