#ifndef BITREADER_H
#define BITREADER_H

typedef struct {
    FILE *f;
    unsigned char buf;
    int pos;
}bitreader;

void init_br(bitreader *br, FILE *f);
int br_read(bitreader *br);
int br_read_byte(bitreader *br);

#ifdef BITREADER_IMPLEMENTATION
#include <stdio.h>
#include <stdint.h>

void init_br(bitreader *br, FILE *f){
    br->f = f;
    br->pos = 8;
}

int br_read(bitreader *br){
    if (br->pos == 8){
        if (fread(&br->buf, 1, 1, br->f) != 1){
            return -1;
        }
        br->pos = 0;
    }
    int bit = ( br->buf >> (7 - br->pos) ) & 1;
    br->pos++;
    return bit;
}

int br_read_byte(bitreader *br) {
    unsigned char byte = 0;

    for (int i = 0; i < 8; i++) {
        int bit = br_read(br);
        if (bit == -1) return -1; // EOF
        byte = (byte << 1) | bit;
    }

    return byte;
}

#endif
#endif
