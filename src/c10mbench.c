#include "c10mbench.h"
#include "pixie-threads.h"
#include <stdio.h>



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
    
    print_version();

    
    /*
     * Run all the benchmarks
     */
    printf("          ,CPUs,   Mm/sec,    nsecs\n");

    bench_mainmem(cpu_count);
    bench_cache_bounce(cpu_count);
    bench_syscall(cpu_count);
    bench_funcall(cpu_count, add_two_numbers);
    bench_msgrate_pipe(cpu_count);
    bench_msgrate_ring(cpu_count);
    return 1;
}
