#ifndef C10MBENCH_H
#define C10MBENCH_H

#define BENCH_ITERATIONS (1000*1000)

void print_version(void);

void
bench_msgrate_ring(unsigned cpu_count);

void
bench_msgrate_pipe(unsigned cpu_count);

void
bench_cache_bounce(unsigned cpu_count);

void
bench_syscall(unsigned cpu_count);

void
bench_funcall(unsigned cpu_count, void (*add_two_numbers)(volatile unsigned *a, unsigned b));

void
bench_mainmem(unsigned cpu_count);

#endif

