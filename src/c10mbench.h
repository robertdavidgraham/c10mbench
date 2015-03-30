#ifndef C10MBENCH_H
#define C10MBENCH_H

#define BENCH_ITERATIONS (1000*1000)

void print_version(void);

void
bench_msgrate_ring(unsigned cpu_count);

void
bench_msgrate_pipe(unsigned cpu_count);

void
bench_cache_bounce(unsigned cpu_count, unsigned which_test);

void
bench_syscall(unsigned cpu_count);

void
bench_funcall(unsigned cpu_count, void (*add_two_numbers)(volatile unsigned *a, unsigned b));

enum {
    MemBench_PointerChase,
    MemBench_PointerChaseHuge,
    MemBench_CmovChase,
    MemBench_CmovChaseHuge,
    MemBench_MaxRate,
    MemBench_MaxRateHuge,
    
    CacheBench_Add,
    CacheBench_LockedAdd,
    CacheBench_MutexAdd,
};

void
bench_mainmem(unsigned cpu_count, unsigned which_test);

#endif

