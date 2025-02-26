#include "occupancy_profile.h"
#include "cache.h"
#include "eviction_set.h"
#include "utility.h"
#include <stdio.h>

void occupancy_profile(eviction_set es, const size_t num_iterations, const char* output_filename)
{
    // Init the CSV file 
    FILE* csv = fopen(output_filename, "w");
    fprintf(csv, "Set,Iteration,SPrime,LPrime,Cycles\n");

    // For the number of iterations given in the call
    for (size_t iter = 0; iter < num_iterations; iter ++)
    {
        // For each set, s, in ES
        for (size_t set = 0; set < es.cache_sets; set++)
        {
            // For each line, (s`, l`), in ES
            for (size_t s_prime = 0; s_prime < es.cache_sets; s_prime++)
            {
                for (size_t l_prime = 0; l_prime < es.cache_lines; l_prime++)
                {
                    // Calculate the address of the line we want to observe, (s`, l`)
                    const size_t line_index = s_prime * l_prime;
                    byte* line = es.occupation_section.start_addr + (line_index * CACHE_LINE_SIZE);

                    // Flush ES from the cache 
                    flush_eviction_set(es);

                    // Prime set s in ES 
                    prime_set_write_with_warmup(es, set); 

                    // Access cache line (s`, l`) and determine hit or miss 
                    const uint64_t time = time_one_line_read_access(line);

                    // Print the data from this iteration
                    // Format is: "Set,Iteration,SPrime,LPrime,Cycles"
                    fprintf(csv, "%lu,%lu,%lu,%lu,%lu\n",
                            set,iter,s_prime,l_prime,time);
                    fence();
                } // l_prime
            } // s_prime 
        } // set 
    } // iter 
    
    // Close the csv file 
    fclose(csv);
}
