#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdint.h>
#include "address.h"

#define ALWAYS_INLINE __attribute__((always_inline))
#define DEPRECATED(msg) __attribute__((deprecated(msg)))

// Reads the time stamp counter as a 64-bit integer value. 
// (Code is derived from https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)
//
// This function uses the `rdtscp` instruction to retrieve the 
// timestamp. According to the Intel Software Developer Manual 
// Vol. 2B, pg. 4-560 `rdtscp` is "not a serializing instruction" 
// but it does wait for all previous _loads_ to become visible 
// and all previous instructions to execute. It also states that 
// for this instruction to complete prior to any following 
// instructions running that an `lfence` instruction can be inserted 
// immediately following the instruction.
// 
// The `rdtsc` instruction returns the lower 32-bits of the timestamp 
// into the EAX register, and the upper 32-bits of the timestamp into 
// the EDX register. The IA32_TSC_AUX is also stored in the RCX 
// register. We don't use this value but we indicate in the inline 
// assembly that it's used so that the compiler knows that the value in 
// RCX will be overwritten.
static inline ALWAYS_INLINE uint64_t read_timestamp() {
    uint64_t time;
    asm volatile(
        "rdtscp\n\t"            // Returns the time in EDX:EAX.
        "shl $32, %%rdx\n\t"    // Shift the upper bits left.
        "or %%rdx, %0"          // 'Or' in the lower bits.
        : "=a" (time)           // Put result from RAX into `time` variable
        : 
        : "rdx", "rcx");        // Indicate that RDX and RCX will be written to
    return time;
}

// Load fence (Intel Software Developer Manual Vol. 1 - 11.4.4.3)
// This function guarantees ordering between two loads and prevents 
// speculative loads until all load prior to this instruction have 
// been carried out.
static inline ALWAYS_INLINE void lfence() {
    asm volatile("lfence");
}

// Memory fence (Intel Software Developer Manual Vol. 1 - 11.4.4.3) 
// This functions ensures that no loads or stores after this instruction 
// will be globally visible until after all loads or stored previous to 
// this instruction are globally visible.
static inline ALWAYS_INLINE void mfence() {
    asm volatile("mfence");
}

// General fence instruction 
// This instruction ensures that previous memory instructions have all 
// completed.
static inline ALWAYS_INLINE void fence() {
    mfence();
    lfence();
}

// Cache Line Flush (Intel Software Developer Manual Vol. 1 - 11.4.4.1)
// Flushes the cache lines from all levels of cache back into main 
// memory.
// Note: this only flushes ONE cache line, so if it's desired that an 
// entire buffer is to be flushed each cache line needs to be flushed 
// individually.
static inline ALWAYS_INLINE void clflush(const void *v) {
    asm volatile ("clflush 0(%0)": : "r" (v):);
}

// Uses `read_timestamp()` and fencing instruction to time a read access 
// to *addr.
static inline ALWAYS_INLINE uint64_t time_one_line_read_access(const byte *addr) {
    volatile byte b;

    mfence(); // ensure all previous memory access is complete
    const uint64_t time_init = read_timestamp();
    lfence(); // inserted due to the advice in Intel SDM Vol. 2B 4-560
    b = *addr; // read address
    const uint64_t elapsed_time = read_timestamp() - time_init;
    return elapsed_time;
}

// Compatability Section For Older Versions of utility.h

static inline uint64_t 
ALWAYS_INLINE 
DEPRECATED("This is the old timing function, please update to `time_one_line_read_access`") 
measure_one_block_access_time(uint64_t addr) {
    return time_one_line_read_access((const void*)addr);
}

#undef ALWAYS_INLINE
#endif
