/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : encryption.h
 * PURPOSE     : Encryption module.
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 04.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */

#ifndef __ENCRYPTION_H_
#define __ENCRYPTION_H_

#include "../../payload.h"

#include <wolfssl/wolfcrypt/md5.h>

#define ENCRYPTION_MD5_SIZE MD5_DIGEST_SIZE

bool encryption_md5(const byte *data, size_t size, byte *md5);

bool encryption_verify(byte *md5, const byte *signature, size_t size);

bool encryption_extract(const byte *data, size_t size, struct payload *result);

#endif /* __ENCRYPTION_H_ */