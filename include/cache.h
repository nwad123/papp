#ifndef CACHE_H
#define CACHE_H

#include "address.h"
#include "utility.h"

#include <stddef.h>

#define CACHE_LINE_SIZE 64 
#define L1_SIZE 32768
#define L2_SIZE 262144
#define L2_SETS 512
#define L2_ASSOCIATIVITY 8

static inline __attribute__((always_inline))
void flush_buffer(const byte* start, const size_t length)
{
    for (size_t i = 0; i < length; i += CACHE_LINE_SIZE)
    {
        const byte* to_flush = start + i;
        clflush(to_flush);
    }

    // fence to ensure flush has completed 
    fence();
}

static inline __attribute__((always_inline))
void flush_buffer_unfenced(const byte* start, const size_t length)
{
    for (size_t i = 0; i < length; i += CACHE_LINE_SIZE)
    {
        const byte* to_flush = start + i;
        clflush(to_flush);
    }
}

static inline void read_buffer(const byte* start, const size_t length)
{
    volatile byte b;

    for (size_t i = 0; i < length; i += CACHE_LINE_SIZE)
    {
        const volatile byte* to_read = start + i;
        b = *to_read; 
    }
}

// returns next address to access
static inline byte* read_lines(byte* start, const size_t num_lines)
{
    volatile byte b;

    for (size_t i = 0; i < (num_lines * CACHE_LINE_SIZE); i += CACHE_LINE_SIZE)
    {
        const volatile byte* to_read = start + i;
        b = *to_read; 
    }

    return start  + (num_lines * CACHE_LINE_SIZE);
}

// returns next address to access
static inline byte* write_buffer(byte* start, const size_t length)
{
    for (size_t i = 0; i < length; i += CACHE_LINE_SIZE)
    {
        volatile byte* to_write = start + i;
        *to_write = *to_write + 1;
    }
    
    return start + length;
}

// returns next address to access
static inline byte* write_lines(byte* start, const size_t num_lines)
{
    const size_t max_index = (num_lines * CACHE_LINE_SIZE);
    for (size_t i = 0; i < max_index; i += CACHE_LINE_SIZE)
    {
        volatile byte* to_write = start + i;
        *to_write = *to_write + 1;
    }

    return start + max_index;
}

// returns next address to access
static inline byte* write_lines_stride(byte* start, const size_t num_lines, const size_t stride)
{
    const size_t max_index = (num_lines * CACHE_LINE_SIZE * stride);

    for (size_t i = 0; i < max_index; i += (stride * CACHE_LINE_SIZE))
    {
        volatile byte* to_write = start + i;
        *to_write = *to_write + 1;
    }

    return start + max_index;
}

#endif 
