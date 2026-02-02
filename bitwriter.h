#ifndef BITWRITER_H
#define BITWRITER_H

typedef struct {
    FILE *f;
    unsigned char byte;
    int count;
} bitwriter;

void bw_init(bitwriter *bw, FILE *f);
void bw_write(bitwriter *bw, int bit);
void bw_write_byte(bitwriter *bw, unsigned char byte);
void bw_flush(bitwriter *bw);

#ifdef BITWRITER_IMPLEMENTATION
#include <stdio.h>
#include <stdint.h>

void bw_init(bitwriter *bw, FILE *f) {
    bw->f = f;
    bw->byte = 0;
    bw->count = 0;
}

void bw_write(bitwriter *bw, int bit) {
    bw->byte = (bw->byte << 1) | (bit & 1);
    bw->count++;

    if (bw->count == 8) {
        fputc(bw->byte, bw->f);
        bw->byte = 0;
        bw->count = 0;
    }
}

void bw_write_byte(bitwriter *bw, unsigned char byte) {
    if (bw->count == 0) {
        fputc(byte, bw->f);
    } else {
        unsigned char out = (bw->byte << (8 - bw->count)) | (byte >> bw->count);
        fputc(out, bw->f);
        bw->byte = byte & ((1 << bw->count) - 1);
    }
}

void bw_flush(bitwriter *bw) {
    if (bw->count > 0) {
        bw->byte <<= (8 - bw->count);
        fputc(bw->byte, bw->f);
    }
}

#endif
#endif
