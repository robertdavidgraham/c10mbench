#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "pixie-mem.h"
#include "rte-ring.h"
#include "pixie-strerror.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef WIN32
#include <process.h>
#include <Windows.h>
#include <WinError.h>
#define getpid _getpid
#define read _read 
#elif defined(__APPLE__)
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#else
#include <unistd.h>
#endif

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*10)

struct MainmemParms {
    uint64_t memsize;
    size_t *pointer;
    uint64_t result;
    unsigned id;
    unsigned cpu_count;
};

unsigned primes[] = {
    1003001,
    1008001,
    1114111,
    1221221,
    1333331,
    1411141,
    1422241,
    1444441,
    1508051,
    1551551,
    1600061,
    1611161,
    1628261,
    1633361,
    1646461,
    1660661,
    1707071,
    1777771,
    1881881,
    3007003,
    3233323,
    3331333,
    7118117,
    7722277,
    9002009,
    9110119,
    9200029,
    9222229,
    9332339,
    9338339,
    9400049,
    9440449,
    9700079,
};

volatile uint64_t global_result = 0;

/******************************************************************************
 * This is a typical LCG random function like 'rand()'. I define my own only
 * so that results will be consistent across all platforms, that I have a
 * 64-bit version, and so that that state is explicit instead of implicit.
 ******************************************************************************/
static size_t 
myrand(unsigned long long *state)
{
    *state = *state * 6364136223846793005UL + 1442695040888963407UL;
    return (size_t)((*state)>>16);
}

/******************************************************************************
 * This takes a pointer-chasing array and randomizes the order in which the
 * pointers will be chased.
 ******************************************************************************/
static void
pointer_chase_randomize(size_t array[], size_t size)
{
    size_t i;
    unsigned long long seed = 0;
    
    for (i=1; i<size; i++)
        array[i-1] = i;
    array[size-1] = 0;
    
    /*
    a->b->c->d->e->f->g->h->i->j->k->l
    
    a->b->c->i->d->e->f->g->h->j->k->l
    */
    
    for (i=0; i<size*2; i++) {
        size_t a = myrand(&seed) % size;
        size_t b = myrand(&seed) % size;
        size_t swap;
        
        swap = array[b];
        array[b] = array[array[b]];
        
        array[swap] = array[a];
        array[a] = swap;
    }
}



/******************************************************************************
 * This thread maximizes the number of random memory accesses, maximizing
 * the number of simultaneous accesses and predictable accesses.
 ******************************************************************************/
static void
rate_thread(void *v_parms)
{
    size_t i;
    struct MainmemParms *parms = (struct MainmemParms *)v_parms;
    uint64_t result = 0;
    size_t *array = (size_t*)parms->pointer;
    uint64_t offset = 0;
    uint64_t count = parms->memsize/sizeof(size_t);
    unsigned jump = primes[parms->id];
    
    for (i=0; i<BENCH_ITERATIONS2; i += 1) {
        result += array[(size_t)offset];
        offset += jump;
        while (offset > count)
            offset -= count;
    }
    
    pixie_locked_add_u64(&global_result, result);
    free(parms);
}


/******************************************************************************
 * This thread is designed for "pointer-chasing", so that out-of-order
 * processors cannot prefetch/predict future bits of data
 ******************************************************************************/
static void
chase_thread(void *v_parms)
{
    size_t i;
    struct MainmemParms *parms = (struct MainmemParms *)v_parms;
    uint64_t result = 0;
    size_t *array = (size_t*)parms->pointer;
    uint64_t offset = 0;
    uint64_t count = parms->memsize/sizeof(size_t);
    unsigned jump = primes[parms->id];

    //pixie_cpu_set_affinity(parms->id);
    for (i=0; i<BENCH_ITERATIONS2; i++) {
        result += array[(size_t)offset];
        offset += jump + array[(size_t)offset];
        while (offset > count)
            offset -= count;
    }
    
    pixie_locked_add_u64(&global_result, result);
    free(parms);
}


/******************************************************************************
 ******************************************************************************/
void
bench_mainmem(unsigned cpu_count, unsigned which_test)
{
    size_t i;
    struct MainmemParms parms[1];
    const char *test_name = "unknown";
    static const uint64_t minsize = 32*1024*1024;
    int is_huge, is_chase;
    
    
    /*
     * We support 4 types of memory tests, a combination of 
     * "pointer-chasing" vs. "rate", and "small-pages" vs.
     * "huge-pages".
     */
    switch (which_test) {
        case MemBench_PointerChase:
            test_name = "memchase";
            is_huge = 0;
            is_chase = 1;
            break;
        case MemBench_PointerChaseHuge:
            test_name = "hugechase";
            is_huge = 1;
            is_chase = 1;
            break;
        case MemBench_MaxRate:
            test_name = "memrate";
            is_huge = 0;
            is_chase = 0;
            break;
        case MemBench_MaxRateHuge:
            test_name = "hugerate";
            is_huge = 1;
            is_chase = 0;
            break;
        default:
            fprintf(stderr, "%s:%u: unknown test\n", __FILE__, __LINE__);
            return;
    }
    

    memset(parms, 0, sizeof(parms[0]));
    parms->cpu_count = cpu_count;
    
    /* We choose 1/4 of the RAM by default */
    parms->memsize = pixie_get_memsize()/4;
    
    if (is_huge) {
        /* We are doing "huge" pages in this test, which is likely to fail
         * unless the user has recently rebooted */
        int err = 0;
        
        parms->memsize = pixie_align_huge(parms->memsize);
        
        parms->pointer = pixie_alloc_huge(parms->memsize, &err);

        switch (err) {
            case HugeErr_Success:
                break;
            case HugeErr_MemoryFragmented:
                fprintf(stderr, "%s: test not run: memory too fragmented (reboot)\n", test_name);
                return;
            case HugeErr_NoPermissions:
                fprintf(stderr, "%s: test not run: need SeLockMemoryPrivilege permission\n", test_name);
                return;
            default:
                fprintf(stderr, "%s: unknown error allocating huge pages\n", test_name);
                return;
        }
    } else {
        /* We aren't doing huge pages, so free the memory as normal */
        parms->pointer = (size_t*)malloc((size_t)parms->memsize);
        if (parms->memsize < minsize) {
            fprintf(stderr, "%s: test not run: buffer too small\n", test_name);
            return;
        }
    }

    /* Zero out the memory. This also has the effect of committing all the 
     * pages if they weren't already, as well as rev up the CPU to full
     * speed if it's in some sort of sleep state */
    fprintf(stderr, "memsize = %llu\n", (uint64_t)parms->memsize);
    memset(parms->pointer, 0, (size_t)parms->memsize);

    /* Initialize the memory according to the test we are performing */
    {
        size_t *array = (size_t*)parms->pointer;
        size_t count = parms->memsize / sizeof(size_t);
        if (is_chase) {
            pointer_chase_randomize(array, count);
        } else {
            uint64_t seed = 0;
            for (i=0; i<count; i++)
                array[i] = primes[myrand(&seed)&0x1f];
        }
    }
    
    
    for (i=0; i<cpu_count; i++) {
        unsigned j;
        double ellapsed;
        double speed;
        size_t thread_handles[256];
        size_t thread_count = 0;
        uint64_t start, stop;
        
        start = pixie_gettime();
        
        /* start the threads */
        for (j=0; j<=i; j++) {
            void (*worker)(void*);
            struct MainmemParms *parms2 = (struct MainmemParms*)malloc(sizeof(*parms2));
            parms->id = j;
            memcpy(parms2, parms, sizeof(parms2[0]));
            
            if (is_chase)
                worker = chase_thread;
            else 
                worker = rate_thread;

            /* launch the worker thread to performa the benchmark */
            thread_handles[thread_count++] = pixie_begin_thread(worker, 0, parms2);
        }
        
        /* wait for all threads to complete their work */
        for (j=0; j<thread_count; j++)
            pixie_join(thread_handles[j], 0);
        stop = pixie_gettime();
        
        
        ellapsed = (stop-start)/1000000.0;
        speed = BENCH_ITERATIONS2*1.0/ellapsed;
        
        printf("%-12s, %2u,  %7.3f,  %7.3f,  %7.1f\n",
               test_name,
               (unsigned)thread_count,
               speed/1000000.0,
               1.0*thread_count*speed/1000000.0,               
               1000000000.0/speed);
        
    }

    /* 
     * Free the huge buffer we allocated 
     */
    if (is_huge)
        pixie_free_huge(parms->pointer, parms->memsize);
    else {
        free(parms->pointer);
    }
}

