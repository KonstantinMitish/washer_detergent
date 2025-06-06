/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : storage.h
 * PURPOSE     : NVS storage handle
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 06.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */
#ifndef __STORAGE_H_
#define __STORAGE_H_

#include <stdbool.h>
#include <sys/types.h>

void storage_init();
bool storage_write(const char *key, const void *data, size_t size);
bool storage_read(const char *key, void *out_data, size_t *inout_size);

#endif /* __STORAGE_H_ */