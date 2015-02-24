#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include <stdlib.h>

#if defined(WIN32)
#include <Windows.h>
#include <io.h>
#include <process.h>

#define pipe(x) _pipe(x,0,0)
#define read(x,y,z) _read(x,y,z)
#define write(x,y,z) _write(x,y,z)
#else
#include <unistd.h>
#include <fcntl.h> 
#endif

/******************************************************************************
 ******************************************************************************/
static void
reader(void *parms)
{
    int fd = *(int*)parms;
    size_t i;

    for (i=0; i<BENCH_ITERATIONS; i++) {
        int x;
        char c;

        x = read(fd, &c, 1);
        if (x != 1)
            break;
    }
}

/******************************************************************************
 ******************************************************************************/
static void
writer(void *parms)
{
    int fd = *(int*)parms;
    size_t i;
    char c = 1;

    for (i=0; i<BENCH_ITERATIONS; i++) {
        int x;

        x = write(fd, &c, 1);
        if (x != 1)
            break;
    }
}


/******************************************************************************
 ******************************************************************************/
void
bench_msgrate_pipe(unsigned cpu_count)
{
    int fd[2];
    int x;
    unsigned i;
    
    x = pipe(fd);
    if (x != 0) {
        perror("pipe");
        exit(1);
    }

    for (i=0; i<cpu_count; i += 2) {
        uint64_t start, stop;
        unsigned j;
        double ellapsed;
        double speed;
        size_t thread_handles[256];
        size_t thread_count = 0;
        
        start = pixie_gettime();
        for (j=0; j<=i; j += 2) {
            thread_handles[thread_count++] = pixie_begin_thread(reader, 0, &fd[0]);
            thread_handles[thread_count++] = pixie_begin_thread(writer, 0, &fd[1]);
        }
        for (j=0; j<thread_handles[j]; j++)
            pixie_join(thread_handles[j], 0);
        stop = pixie_gettime();
        
        ellapsed = (stop-start)/1000000.0;
        speed = (BENCH_ITERATIONS*1.0)/ellapsed;
        printf("pipe,        %2u-cpus, %7.3f-mmsgs/s,   %6.1f-nsec\n",
                (unsigned)thread_count,
                speed/1000000.0,
                1000000000.0/speed);
        
    }
}
