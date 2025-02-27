#include "address.h"
#include "cache.h"
#include "eviction_set.h"
#include "occupancy_profile.h"
#include <stdio.h>

int main()
{
    const size_t test_set[] = {0, 1, 2, 4, 8, 32, 64, 128, 256, 384, 448, 480, 488, 500, 508, 510, 511};
    const size_t size_test_set = sizeof(test_set) / sizeof(size_t);
    const size_t iterations = 100;
    const size_t l2_sets = L2_SETS;
    const size_t l2_associativity = L2_ASSOCIATIVITY;
    
    char filename[150] = {0};

    {
        // TEST 1, no warmup
        const size_t warmup_lines = 0;

        printf("Starting no warmup test...");
        fflush(stdout);

        eviction_set es = new_eviction_set(l2_sets, l2_associativity, warmup_lines);

        for (const size_t* set = test_set; set < (test_set + size_test_set); set++)
        {
            snprintf(filename, 150, "results/occupancy/no_warmup_O1_CPU4_S%lu.csv", *set);
            occupancy_profile(es, *set, iterations, filename);
        }

        free_eviction_set(&es);
        
        printf("Finished\n");
        fflush(stdout);
    }
    {
        // TEST 2, 8 warmup lines
        const size_t warmup_lines = 8;

        printf("Starting test with 8 warmup lines...\n");
        fflush(stdout);

        eviction_set es = new_eviction_set(l2_sets, l2_associativity, warmup_lines);

        for (const size_t* set = test_set; set < (test_set + size_test_set); set++)
        {
            snprintf(filename, 150, "results/occupancy/8_warmup_O1_CPU4_S%lu.csv", *set);
            occupancy_profile(es, *set, iterations, filename);
        }

        free_eviction_set(&es);
        
        printf("Finished\n");
        fflush(stdout);
    }

    return 0;
}
