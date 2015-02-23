#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "rte-ring.h"
#include <stdlib.h>

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*100)

uint64_t verifier1, verifier2;

size_t read_wait, write_wait;

static void
reader(void *parms)
{
    struct rte_ring *fd = (struct rte_ring*)parms;
    size_t i;
    uint64_t result = 0;

    for (i=0; i<BENCH_ITERATIONS2; i++) {
        int err;
        size_t p;

        again:
        err = rte_ring_sc_dequeue(fd, (void**)&p);
        if (err) {
            read_wait++;
            pixie_usleep(100);
            goto again;
        }
        result += p;
    }

    if (i != BENCH_ITERATIONS2)
        printf("***FAILURE***\n");
    verifier1 = result;
}

static void
writer(void *parms)
{
    struct rte_ring *fd = (struct rte_ring*)parms;
    size_t i;
    uint64_t result = 0;

    for (i=0; i<BENCH_ITERATIONS2; i++) {
        int err;

        again:
        err = rte_ring_sp_enqueue(fd, (void*)i);
        if (err) {
            //printf(".");
            write_wait++;
            pixie_usleep(100);
            goto again;
        }
        result += i;
    }
    if (i != BENCH_ITERATIONS2)
        printf("***FAILURE***\n");
    verifier2 = result;
}


void
bench_msgrate_ring(void)
{
    size_t reader_thread, writer_thread;
    uint64_t start, stop;
    struct rte_ring *ring;

    /*
     * Create the ring
     */
#define BUFFER_COUNT 16384*256
     ring = rte_ring_create(BUFFER_COUNT, RING_F_SP_ENQ|RING_F_SC_DEQ);
     if (ring == NULL) {
         fprintf(stderr, "****FAILURE***\n");
         exit(1);
     }


    start = pixie_gettime();
    writer_thread = pixie_begin_thread(writer, 0, ring);
    reader_thread = pixie_begin_thread(reader, 0, ring);
    pixie_join(reader_thread, 0);
    pixie_join(writer_thread, 0);
    stop = pixie_gettime();
    

    {
        double ellapsed = (stop-start)/1000000.0;
        double speed = BENCH_ITERATIONS2*1.0/ellapsed;
        printf("\nbenchmark: ring msg rate\n");
        printf("verifier: %llu = %llu\n", verifier1, verifier2);
        printf("rate = %5.2f mega-msgs/sec\n", speed/1000000.0);
        printf("time = %5.4f usec\n", 1000000.0/speed);
        printf("waits = %llu %llu\n", (unsigned long long)read_wait, (unsigned long long)write_wait);
        printf("%5.3f\n", (read_wait+write_wait)*10.0/(stop-start));
    }
}
