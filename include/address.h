#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdint.h>

#define BLOCK_BITS 6 
#define NUM_SETS 512 
#define SET_BITS 9
#define TAG_BITS (64 - (SET_BITS + BLOCK_BITS))

static inline uint64_t get_set_index(const void* ptr)
{
    const uint64_t addr_u = (uint64_t)ptr;
    return (addr_u << TAG_BITS) >> (TAG_BITS + BLOCK_BITS);
}

static inline uint64_t get_block_index(const void* ptr)
{
    const uint64_t addr_u = (uint64_t)ptr;
    return (addr_u << (TAG_BITS + SET_BITS)) >> (TAG_BITS + SET_BITS);
}

static inline uint64_t get_tag(const void* ptr)
{
    const uint64_t addr_u = (uint64_t)ptr;
    return addr_u >> (SET_BITS + BLOCK_BITS);
}

static inline void* concat_address(const uint64_t tag, const uint64_t set, const uint64_t block)
{
    const uint64_t addr_u = (tag << (SET_BITS + BLOCK_BITS)) | (set << BLOCK_BITS) | (block);
    return (void*)addr_u;
}

typedef uint8_t byte;

#endif
