#ifndef OCCUPANCY_H
#define OCCUPANCY_H

#include "address.h"
#include "cache.h"
#include "eviction_set.h"
#include <stddef.h>

/// Performs the occupancy profiling from the PAPP paper.
///
/// @param es The eviction set that is used to test the prefetcher. If 
///           `es` is set up to have a warm up section then this will 
///           perform the warmup first as described in Section 3.1 of 
///           the PAPP paper. 
/// @param num_iterations The number of iterations to run the analysis for.
/// @param output_filename The CSV file to write the results to.
///
/// The output CSV file follows the format of:
///
/// Set,Iteration,SPrime,LPrime,Cycles
/// 0,0,0,0,270
/// 0,0,1,0,45
/// ...
void occupancy_profile(/*inout*/ eviction_set es, 
                       /*in*/ const size_t num_iterations, 
                       /*in*/const char* output_filename);

/// Primes a given set (with warmup if specified in es) inside an eviction set.
///
/// @param es The eviction set to prime `set` in.
/// @param set The set to prime in `es`.
///
/// PERF: `es` and `set` are both pass by value because we've nicely asked the 
/// compiler to inline this function.
static inline __attribute__((always_inline))
void prime_set_write_with_warmup(/*inout*/ eviction_set es, /*in*/ const size_t set)
{
    // calculate the stride for moving through the eviction set, as well as 
    // the offset for the initial set 
    const size_t set_stride = es.cache_sets * CACHE_LINE_SIZE;
    const size_t offset = set * CACHE_LINE_SIZE;
    
    // if any warmup is required, access the warmup lines here in preparation for 
    // the priming of the set
    for (size_t line = 0; line < es.warmup_lines; line++)
    {
        byte* addr = es.warmup_section.start_addr + (line * set_stride);
        *addr = *addr * 2;
    }

    // stride through all the lines after the warmup section in ES, writing 
    // to each line to prime it
    for (size_t line = 0; line < es.cache_lines; line++)
    {
        byte* addr = es.occupation_section.start_addr + (line * set_stride);
        *addr = *addr * 2;
    }
}

#endif // OCCUPANCY_H
