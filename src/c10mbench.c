#include "c10mbench.h"
#include <stdio.h>



void add_two_numbers(volatile unsigned *a, unsigned b)
{
    *a += b;
}

int main(int argc, char *argv[])
{
    printf("--- C10M low-level benchmarks ---\n");

#if defined(WIN32)
    printf("platform: Windows\n");
#elif defined(__linux__)
    printf("platform: Linux\n");
#else
    printf("platform: POSIX (unknown)\n");
#endif

    printf("pointer: %u-bits\n", (unsigned)(sizeof(void*)*8));
    printf("\n");

    
    bench_msgrate_pipe();
    bench_syscall();
    bench_cache_bounce();
    bench_msgrate_ring();
    bench_funcall(add_two_numbers);

    return 1;
}
