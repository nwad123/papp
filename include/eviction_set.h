#ifndef EVICTION_SET_H
#define EVICTION_SET_H

#include "address.h"
#include "cache.h"
#include <stddef.h>

typedef struct {
    byte* start_addr;
    size_t size;
} wide_ptr;

typedef struct {
    wide_ptr warmup_section;
    wide_ptr occupation_section;
    size_t cache_sets;
    size_t cache_lines;
    size_t warmup_lines;
} eviction_set;

/// Generates an eviction set based on the given cache parameters. Note
/// that this function can be used for any set-associative style cache, 
/// as long as the correct parameters are supplied.
///
/// This function **allocates** the new eviction set using `mmap`. You
/// must call free_eviction_set on the generated structure in order 
/// to avoid memory leaks.
///
/// @param cache_sets The number of sets in the targeted cache. In the 
///                   PAPP paper this is `s`.
///
///  @param cache_lines The number of lines per set in the cache. In
///                     the PAPP paper this is `a`.
///
///  @param extra_lines The number of extra lines to generate for each 
///                     set (for the warmup section), this is `n` in the
///                     paper.
eviction_set new_eviction_set(
    /*in*/ const size_t cache_sets,
    /*in*/ const size_t cache_lines, 
    /*in*/ const size_t extra_lines);

/// Frees the memory allocated to the given eviction set. Sets the values
/// in the original eviction set struct to 0's in order to attempt to 
/// prevent extraneous memory access.
///
/// @param es A pointer to the eviction set data from which to first free 
///           the memory and then clear the data in es as to prevent invalid 
///           memory accesses.
void free_eviction_set(/*inout*/ eviction_set* es);

/// Flushes an entire eviction set from the cache. Calls `fence()` after 
/// to ensure that all flushes are completed before returning.
///
/// @param es The eviction set to flush 
///
/// PERF: since we are inlining this function we set `es` as a pass-by-value
/// argument.
static inline __attribute__((always_inline))
void flush_eviction_set(/*inout*/ eviction_set es)
{
    flush_buffer_unfenced(es.warmup_section.start_addr, 
                          es.warmup_section.size);
    flush_buffer_unfenced(es.occupation_section.start_addr, 
                          es.occupation_section.size);
    fence();
}

#endif // EVICTION_SET_H
