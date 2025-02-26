#include "utility.h"
#include "address.h"
#include "cache.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


#define BUF_SIZE (L2_SIZE * 2)
#define SAMPLES 100000

static inline uint64_t time(byte *addr) {
    return time_one_line_read_access(addr);
}

int main()
{ 
    byte* target = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                        MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB,
                        -1, 0);
    
    byte* eviction = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                          MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB,
                          -1, 0);

    // Result buffers
    uint64_t l1_latencies[SAMPLES] = {};
    uint64_t l2_latencies[SAMPLES] = {};
    uint64_t l3_latencies[SAMPLES] = {};
    uint64_t ram_latencies[SAMPLES] = {};

    // Byte to read or write target
    // volatile byte b;

    // L1 latencies
    fence();
    for (size_t i = 0; i < SAMPLES; i++) {
        for (size_t j = 0; j < 10; j++) {
            write_buffer(target, 1);
        }
        *target = 0;

        l1_latencies[i] = time(target);
    }
    
    // L2 Latencies
    fence();
    for (size_t i = 0; i < SAMPLES; i++) {
        // bring target into L1
        for (size_t j = 0; j < 10; j++) {
            write_buffer(target, 1);
        }

        // evict target into L2 
        for (size_t j = 0; j < 10; j++) {
            write_buffer(eviction, L1_SIZE);
        }

        l2_latencies[i] = time(target);
    }

    // L3 Latencies 
    fence();
    for (size_t i = 0; i < SAMPLES; i++) {
        // bring target into L1
        for (size_t j = 0; j < 10; j++) {
            write_buffer(target, 1);
        }

        // evict target into L3
        for (size_t j = 0; j < 10; j++) {
            write_buffer(eviction, L2_SIZE);
        }

        l3_latencies[i] = time(target);
    }
    
    // RAM Latencies 
    fence();
    for (size_t i = 0; i < SAMPLES; i++) {
        // flush target to RAM
        flush_buffer(target, L2_SIZE);

        ram_latencies[i] = time(target);
    }

    // Print results 
    FILE *data_fp = fopen("results/timing.csv", "w");
    fprintf(data_fp, "L1,L2,L3,RAM\n");
    for (size_t i = 0; i < SAMPLES; i++) {
        fprintf(data_fp, "%lu,%lu,%lu,%lu\n", 
                l1_latencies[i], l2_latencies[i], l3_latencies[i], ram_latencies[i]);
    }
}
