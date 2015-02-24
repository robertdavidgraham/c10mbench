#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "rte-ring.h"
#include <stdlib.h>

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*100)

uint64_t verifier1, verifier2;

uint64_t read_wait, write_wait;

/******************************************************************************
 ******************************************************************************/
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
        err = rte_ring_dequeue(fd, (void**)&p);
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

/******************************************************************************
 ******************************************************************************/
static void
writer(void *parms)
{
    struct rte_ring *fd = (struct rte_ring*)parms;
    size_t i;
    uint64_t result = 0;

    for (i=0; i<BENCH_ITERATIONS2; i++) {
        int err;

        again:
        err = rte_ring_enqueue(fd, (void*)i);
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


/******************************************************************************
 ******************************************************************************/
void
bench_msgrate_ring(unsigned cpu_count)
{
    struct rte_ring *ring;
    unsigned i;
    
    /*
     * Create the ring
     */
#define BUFFER_COUNT 16384*256

    for (i=0; i<cpu_count; i += 2) {
        uint64_t start, stop;
        unsigned j;
        double ellapsed;
        double speed;
        size_t thread_handles[256];
        size_t thread_count = 0;
        
        if (i == 0) {
            ring = rte_ring_create(BUFFER_COUNT, RING_F_SC_DEQ|RING_F_SP_ENQ);
        } else {
            ring = rte_ring_create(BUFFER_COUNT, 0);
        }
        if (ring == NULL) {
            fprintf(stderr, "****FAILURE***\n");
            exit(1);
        }
        
        read_wait = 0;
        write_wait = 0;
        
        start = pixie_gettime();
        for (j=0; j<=i; j += 2) {
            thread_handles[thread_count++] = pixie_begin_thread(reader, 0, ring);
            thread_handles[thread_count++] = pixie_begin_thread(writer, 0, ring);
        }
        for (j=0; j<thread_handles[j]; j++)
            pixie_join(thread_handles[j], 0);
        stop = pixie_gettime();
        
        ellapsed = (stop-start)/1000000.0;
        speed = (BENCH_ITERATIONS2*1.0)/ellapsed;
        printf("ring,        %2u-cpus, %7.3f-mmsgs/s,   %6.1f-nsec, ww=%llu, rw=%llu\n",
               (unsigned)thread_count,
               speed/1000000.0,
               1000000000.0/speed,
               write_wait, read_wait);
        
        
    }
}
