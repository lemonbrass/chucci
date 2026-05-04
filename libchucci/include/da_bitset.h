#ifndef DA_BITSET_H
#define DA_BITSET_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    size_t   num_bits;
    uint8_t* bytes;
} bitset_t;

bitset_t new_bs(size_t num_bits);
void free_bs(bitset_t* bs);
int is_valid_bs(const bitset_t* bs);
int get_bit(bitset_t* bs, size_t bitid);
int set_bit(bitset_t* bs, size_t bitid);
int clear_bit(bitset_t* bs, size_t bitid);

#endif

