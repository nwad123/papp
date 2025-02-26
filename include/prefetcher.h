#ifndef PREFETCHER_H
#define PREFETECHER_H

#include <stddef.h>

static inline void retrain_prefetcher(const size_t num_streams)
{
    static size_t n = 0;

    // dynamically store a list of blocks the size of 2x L2 to 
    // use to retrain the prefetcher by striding through the 
    // blocks several times.
}

#endif 
