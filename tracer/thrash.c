#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void thrash(uint32_t cache_size)
{
    uint32_t buf_size = cache_size / 2;
    uint8_t * buffers = (uint8_t *) malloc(buf_size * 3);
    uint8_t * buf1 = &buffers[0];
    uint8_t * buf2 = &buffers[buf_size];
    uint8_t * buf3 = &buffers[buf_size * 2];
    for (uint32_t it = 0; it < 1000; ++it) {
        for (uint32_t i = 0; i < buf_size; ++i) {
            buf3[i] = buf1[i] + buf2[i];
        }
    }
    free(buffers);
}

int main(int argc, char ** argv)
{
    if (argc != 2) {
        printf("Usage: %s cache-size-kb\n", argv[0]);
        return 0;
    }
    uint32_t cache_size = atoi(argv[1]);
    thrash(cache_size);
    return 0;
}
