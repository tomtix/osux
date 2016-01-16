#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <assert.h>

#ifndef min
#define min(x, y)  ((x) < (y) ? (x) : (y))
#endif

unsigned char *osux_md5_hash_file(FILE *f)
{
    unsigned char *c = malloc(MD5_DIGEST_LENGTH + 1);
    MD5_CTX mdContext;
    int bytes = 0;
    unsigned char data[1024];

    assert( NULL != f );
    rewind(f);
    MD5_Init(&mdContext);
    while ((bytes = fread(data, 1, 1024, f)) != 0)
        MD5_Update(&mdContext, data, bytes);
    MD5_Final(c, &mdContext);
    c[MD5_DIGEST_LENGTH] = '0';
    return c;
}

unsigned char *osux_md5_hash_buf(size_t size, const unsigned char *buf)
{
    unsigned char *c = malloc(MD5_DIGEST_LENGTH + 1);
    MD5_CTX mdContext;
    int bytes = 0;
    const unsigned char *data = buf;
    assert( 0 == size || NULL != buf );
    MD5_Init(&mdContext);
    while ( data < buf+size ) {
        bytes = min(1024, (buf+size)-data);
        MD5_Update(&mdContext, data, bytes);
        data += bytes;
    }
    MD5_Final(c, &mdContext);
    c[MD5_DIGEST_LENGTH] = '0';
    return c;
}
