#ifndef __ENCRYPTION_H_
#define __ENCRYPTION_H_

#include <wolfssl/wolfcrypt/md5.h>

#define ENCRYPTION_MD5_SIZE MD5_DIGEST_SIZE

bool encryption_md5(const byte *data, size_t size, byte *md5);

#endif /* __ENCRYPTION_H_ */