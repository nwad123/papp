#include "eviction_set.h"
#include "cache.h"

#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>

eviction_set new_eviction_set(
    /*in*/ const size_t cache_sets,
    /*in*/ const size_t cache_lines, 
    /*in*/ const size_t warmup_lines)
{
    // calculate the total number of bytes we will need to allocate
    // for the eviction set
    const size_t num_bytes_in_cache = CACHE_LINE_SIZE * cache_sets * (cache_lines + warmup_lines);

    // allocate using mmap. This call guarantees that the returned memroy 
    // will be in set 0, block 0
    byte* mem = mmap(NULL, num_bytes_in_cache, 
                     PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | 
                     MAP_PRIVATE | MAP_HUGETLB,
                     -1, 0);

    // calculate the starting address for the warmup section, if 
    // `warmup_size == 0` then this will be the same as 
    // `occupation_start`
    byte* warmup_start = mem;
    // the occupation section starts after `warmup_lines` in the cache.
    byte* occupation_start = mem + (warmup_lines * cache_sets * CACHE_LINE_SIZE);

    const size_t warmup_size = CACHE_LINE_SIZE * cache_sets * warmup_lines;
    const size_t occupation_size = CACHE_LINE_SIZE * cache_sets * cache_lines;

    return (eviction_set){
        .warmup_section = (wide_ptr){
            .start_addr = warmup_start,
            .size = warmup_size 
        },
        .occupation_section = (wide_ptr){
            .start_addr = occupation_start,
            .size = occupation_size 
        },
        .cache_sets = cache_sets,
        .cache_lines = cache_lines,
        .warmup_lines = warmup_lines
    };
}

void free_eviction_set(/*inout*/ eviction_set* es)
{
    // check if we were given a NULL ptr
    if (es == NULL) {
        return;
    }

    // if we were given a memory address that isn't null, deallocate it 
    // using munmap
    if (es->warmup_section.start_addr != NULL) {
        munmap(es->warmup_section.start_addr, es->warmup_section.size + es->occupation_section.size);
    }

    // clear the data in the eviction set data structure
    *es = (eviction_set){
        .warmup_section = (wide_ptr){
            .start_addr = NULL,
            .size = 0
        },
        .occupation_section = (wide_ptr){
            .start_addr = NULL,
            .size = 0
        },
        .cache_sets = 0,
        .cache_lines = 0,
        .warmup_lines = 0
    };
}
