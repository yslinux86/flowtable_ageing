#ifndef __SM3_H__
#define __SM3_H__


#define MAX_BUFSIZE 4096

#define GET_UINT32(n,b,i)                         \
do {                                              \
    (n) = ( (uint32_t) (b)[(i)    ] << 24 )       \
        | ( (uint32_t) (b)[(i) + 1] << 16 )       \
        | ( (uint32_t) (b)[(i) + 2] <<  8 )       \
        | ( (uint32_t) (b)[(i) + 3]       );      \
} while(0)

#define PUT_UINT32(n,b,i)                         \
do {                                              \
    (b)[(i)    ] = (uint8_t) ( (n) >> 24 );       \
    (b)[(i) + 1] = (uint8_t) ( (n) >> 16 );       \
    (b)[(i) + 2] = (uint8_t) ( (n) >>  8 );       \
    (b)[(i) + 3] = (uint8_t) ( (n)       );       \
} while(0)

/* ROTATE_LEFT rotates x left n bits.*/
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define P0(x) (x ^ ROTL(x, 9) ^ ROTL(x,17))
#define P1(x) (x ^ ROTL(x,15) ^ ROTL(x,23))

#ifndef BYTE
typedef unsigned char  BYTE;
typedef unsigned char  uint8_t;
#endif

#ifndef UINT16
typedef unsigned short uint16_t;
#endif

#ifndef UINT32
typedef unsigned int   uint32_t;
#endif

#ifndef UINT64
typedef unsigned long  uint64_t;
#endif

static uint8_t sch_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

typedef struct
{
    uint32_t total[2];
    uint32_t state[8];
    uint8_t buffer[64];
}sch_context;

void hex_dump(const void *src, size_t length);
int TCM_SM3_soft(BYTE* content, size_t length, BYTE digest[32]);

#endif
