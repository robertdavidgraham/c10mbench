#ifndef C10MBENCH_H
#define C10MBENCH_H

#define BENCH_ITERATIONS (1000*1000)

void
bench_msgrate_ring(void);

void
bench_msgrate_pipe(void);

void
bench_cache_bounce(void);

void
bench_syscall(void);

void
bench_funcall(void (*add_two_numbers)(volatile unsigned *a, unsigned b));



#endif

