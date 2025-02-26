#include "utility.h"
#include "address.h"
#include "cache.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


#define BUF_SIZE (1 << 20)

void check_next_line_prefetching(void* target, const size_t t_size, void* eviction, const size_t e_size)
{
    #define NLP_SAMPLES 50000
    #define NLP_TS_MAX 64

    // print progress 
    printf("Testing next line...\n");

    uint16_t training_size[NLP_SAMPLES] = {};
    uint64_t times[NLP_SAMPLES] = {};

    // Go over a bunch of training sizes
    for (uint16_t i = 0; i < NLP_SAMPLES; i++) {
        // clear out buffer and fill cache with eviction buffer
        // this ensures we start with a "clean slate"
        flush_buffer(target, t_size);
        write_buffer(eviction, e_size);

        // ensure all prior memory accesses have completed
        fence();

        // Calculate the training size
        const uint64_t ts = i % NLP_TS_MAX;
        training_size[i] = ts;
       
        // Write to `ts` consecutive cache lines, then calculate the read time of 
        // the next cache line
        const byte* next = write_lines(target, ts);
        times[i] = time_one_line_read_access(next);
    }
    
    // print progress 
    printf("... Finished next line test.\n");

    // print results
    FILE *results = fopen("results/next_line.csv", "w");
    fprintf(results, "TrainingSize,Cycles\n");
    for (size_t i = 0; i < NLP_SAMPLES; i++) {
        fprintf(results, "%hu,%lu\n", training_size[i], times[i]);
    }

    #undef NLP_SAMPLES
    #undef NLP_TS_MAX
}

void check_stride_prefetching(void* target, const size_t t_size, void* eviction, const size_t e_size)
{
    #define STRIDE_SAMPLES 5000
    #define STRIDE_TS_MAX 32

    const uint16_t strides_to_test[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 32};

    uint16_t training_size[STRIDE_SAMPLES] = {};
    uint16_t strides[STRIDE_SAMPLES] = {};
    uint64_t times[STRIDE_SAMPLES] = {};

    // print progress 
    printf("Testing Stride...\n");

    // for all strides from 1 to STRIDE_LINES_MAX
    for(uint16_t stride_index = 0; stride_index < sizeof(strides_to_test)/sizeof(uint16_t); stride_index++) { 
        const uint16_t stride = strides_to_test[stride_index];

        // Take STRIDE_SAMPLES of the timing of each access
        for (uint16_t i = 0; i < STRIDE_SAMPLES; i++) {
            // clear out buffer and fill cache with eviction buffer
            // this ensures we start with a "clean slate"
            flush_buffer(target, t_size);
            write_buffer(eviction, e_size);

            // ensure all prior memory accesses have completed
            fence();

            // calculate the training size 
            const uint64_t ts = i % STRIDE_TS_MAX;
            training_size[i] = ts;

            // Write to `ts` cache lines, and then check the access time of the 
            // next cache line
            const byte* next = write_lines_stride(target, ts, stride);
            times[i] = time_one_line_read_access(next);
        }

        // print progress 
        printf("... Finished stride of %u\n", stride);

        // print results
        char filename[100] = {}; 
        snprintf(filename, 100, "results/strides/%u.csv", stride);
        FILE *results = fopen(filename, "w");
        fprintf(results, "TrainingSize,Cycles\n");
        for (size_t i = 0; i < STRIDE_SAMPLES; i++) {
            fprintf(results, "%hu,%lu\n", training_size[i], times[i]);
        }
    }

    #undef STRIDE_SAMPLES
    #undef STRIDE_TS_MAX
}

int main()
{ 
    byte* target = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                        MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB,
                        -1, 0);
    byte* eviction = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                          MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB,
                          -1, 0);

    check_next_line_prefetching(target, BUF_SIZE, eviction, BUF_SIZE);
    check_stride_prefetching(target, BUF_SIZE, eviction, BUF_SIZE);
}
