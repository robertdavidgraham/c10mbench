#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "rte-ring.h"
#include <stdlib.h>

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*100)

uint64_t verifier1, verifier2;

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
    char c = 1;
    uint64_t result = 0;

    for (i=0; i<BENCH_ITERATIONS2; i++) {
        int err;

        again:
        err = rte_ring_sp_enqueue(fd, (void*)i);
        if (err) {
            //printf(".");
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
#define BUFFER_COUNT 16384
     ring = rte_ring_create(BUFFER_COUNT, RING_F_SP_ENQ|RING_F_SC_DEQ);
     if (ring == NULL) {
         fprintf(stderr, "****FAILURE***\n");
         exit(1);
     }


    start = pixie_gettime();
    reader_thread = pixie_begin_thread(reader, 0, ring);
    writer_thread = pixie_begin_thread(writer, 0, ring);
    pixie_join(reader_thread, 0);
    pixie_join(writer_thread, 0);
    stop = pixie_gettime();

    {
        double ellapsed = (stop-start)/1000000.0;
        double speed = BENCH_ITERATIONS2*1.0/ellapsed;
        printf("\nbenchmark: ring msg rate\n");
        printf("verifier: %lu = %lu\n", verifier1, verifier2);
        printf("rate = %5.2f mega-msgs/sec\n", speed/1000000.0);
        printf("time = %5.4f usec\n", 1000000.0/speed);
    }
}
