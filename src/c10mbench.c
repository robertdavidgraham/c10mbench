#include "c10mbench.h"
#include "pixie-threads.h"
#include <stdio.h>

/*
#include <Windows.h>

void alloc_all(void)
{
    char **p;
    unsigned i;
    unsigned count = 6*1024*1024;
    unsigned j;

    p = (char**)malloc(sizeof(*p) * count);
    for (i=0; i<count; i++) {
        p[i] = (char*)VirtualAlloc(0, 4096, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        if (p[i] == NULL)
            break;
        p[i][0] = 0xa3;
    }

    for (j=0; j<i; j++) {
        VirtualFree(p[i], 4096, MEM_RELEASE);
    }
}
 */


/******************************************************************************
 * This if for benchmarking function-call times. We locate this in a separate
 * module so that it doesn't get optmized away. Also, we pass it as a function
 * point in order to stress the worst-case function call time, which relies
 * upon modern-branch prediction to call function-pointers efficiently.
 ******************************************************************************/
void
add_two_numbers(volatile unsigned *a, unsigned b)
{
    *a += b;
}


/******************************************************************************
 ******************************************************************************/
int
main(int argc, char *argv[])
{
    unsigned cpu_count;
    
    /* Grab the number of CPUs. We want to run benchmarks on as many CPUs as
     * possible in order to test scaling */
    cpu_count = pixie_cpu_get_count();

    /* Print OS, version, CPU, etc. */
    print_version();

    /*
     * Print header information
     */
    printf("           ,CPUs,   Mm/sec,    Total,    nsecs\n");

    //alloc_all();

    bench_mainmem(cpu_count, MemBench_MaxRateHuge);
    bench_mainmem(cpu_count, MemBench_PointerChaseHuge);
    bench_mainmem(cpu_count, MemBench_MaxRate);
    bench_mainmem(cpu_count, MemBench_PointerChase);
    bench_cache_bounce(cpu_count, CacheBench_Add);
    bench_cache_bounce(cpu_count, CacheBench_LockedAdd);
    bench_cache_bounce(cpu_count, CacheBench_MutexAdd);
    bench_syscall(cpu_count);
    bench_funcall(cpu_count, add_two_numbers);
    bench_msgrate_pipe(cpu_count);
    bench_msgrate_ring(cpu_count);
    return 1;
}
