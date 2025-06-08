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
#ifndef __SOCKETS_H_
#define __SOCKETS_H_

#include <stdbool.h>

typedef int SOCKET;

typedef unsigned long IP;

#define IP_FORMAT "%d.%d.%d.%d"

#define IP_FORMAT_DATA(X) (int) (X & 0xFF), (int) ((X >> 8) & 0xFF), (int) ((X >> 16) & 0xFF), (int) (X >> 24)

#define IP_MAKE(A, B, C, D) ((((D) << 24) & 0xFF) | (((C) << 16) & 0xFF) | (((B) << 8) & 0xFF) | ((A) &0xFF))

SOCKET socket_udp(int port);

SOCKET socket_tcp(int port);

SOCKET socket_accept(SOCKET s, IP *ip);

bool socket_has_data(SOCKET s);

bool socket_recv(SOCKET s, char *buf, int *len);

bool socket_recvfrom(SOCKET s, char *buf, int *len, IP *ip);

bool socket_send(SOCKET s, const char *buf, int len);

void socket_close(SOCKET s);

#endif /* __SOCKETS_H_ */