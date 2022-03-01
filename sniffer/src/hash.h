#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <string>

#define BYTES_PER_BLOCK 64
#define BYTES_FOR_LENGTH 8

#define ONE 0x80
#define ZERO 0x00

namespace hashIP{   
    char *hash(uint32_t ipSrc, uint16_t portSrc, uint32_t ipDest, uint16_t portDest, int l3_proto);
}