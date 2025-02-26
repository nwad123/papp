#include "address.h"
#include "cache.h"
#include "eviction_set.h"
#include "occupancy_profile.h"
#include <stdio.h>

int main()
{
    {
        // TEST 1, no warmup
        const size_t l2_sets = L2_SETS;
        const size_t l2_associativity = L2_ASSOCIATIVITY;
        const size_t warmup_lines = 0;

        printf("Starting no warmup test...");
        fflush(stdout);

        eviction_set es = new_eviction_set(l2_sets, l2_associativity, warmup_lines);
        occupancy_profile(es, 100, "results/occupancy/no_warmup_O1_CPU4.csv");
        free_eviction_set(&es);
        
        printf("Finished\n");
        fflush(stdout);
    }
    {
        // TEST 2, 8 warmup lines
        const size_t l2_sets = L2_SETS;
        const size_t l2_associativity = L2_ASSOCIATIVITY;
        const size_t warmup_lines = 8;

        printf("Starting test with 8 warmup lines...\n");
        fflush(stdout);

        eviction_set es = new_eviction_set(l2_sets, l2_associativity, warmup_lines);
        occupancy_profile(es, 100, "results/occupancy/8_warmup_lines_O1_CPU4.csv");
        free_eviction_set(&es);
        
        printf("Finished\n");
        fflush(stdout);
    }

    return 0;
}
