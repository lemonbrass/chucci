#include <assert.h>
#include <da_bitset.h>
#include <stdlib.h>
#include <string.h>


bitset_t new_bs(size_t num_bits) {
    bitset_t bs = {0};
    bs.num_bits = num_bits;
    size_t num_bytes = (num_bits + 7) / 8;
    bs.bytes = calloc(num_bytes, 1);
    return bs;
}

void free_bs(bitset_t* bs) {
    if (!bs) return;
    free(bs->bytes);
    bs->bytes = NULL;
    bs->num_bits = 0;
}

int is_valid_bs(const bitset_t* bs) {
    return bs && bs->bytes;
}

void resize_bs(bitset_t* bs) {
  size_t new_num_bits = bs->num_bits==0 ? 16 : bs->num_bits*2;
  bs->bytes = realloc(bs->bytes, (new_num_bits + 7)/8);
  assert(bs->bytes != NULL);
  memset(bs->bytes, 0, (new_num_bits + 7)/8 - (bs->num_bits + 7)/8);
}

int get_bit(bitset_t* bs, size_t bitid) {
    if (!is_valid_bs(bs)) return -1;
    if (bitid >= bs->num_bits) resize_bs(bs);

    size_t byteid = bitid >> 3;
    size_t offset = bitid & 7;

    return (bs->bytes[byteid] >> offset) & 1;
}

int set_bit(bitset_t* bs, size_t bitid) {
    if (!is_valid_bs(bs)) return -1;
    if (bitid >= bs->num_bits) resize_bs(bs);

    size_t byteid = bitid >> 3;
    size_t offset = bitid & 7;

    bs->bytes[byteid] |= (1u << offset);
    return 0;
}

int clear_bit(bitset_t* bs, size_t bitid) {
    if (!is_valid_bs(bs)) return -1;
    if (bitid >= bs->num_bits) resize_bs(bs);

    size_t byteid = bitid >> 3;
    size_t offset = bitid & 7;

    bs->bytes[byteid] &= ~(1u << offset);
    return 0;
}
