#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"

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


void
bench_msgrate_pipe(void)
{
    int fd[2];
    int x;
    size_t reader_thread, writer_thread;
    uint64_t start, stop;

    x = pipe(fd);
    if (x != 0) {
        perror("pipe");
        exit(1);
    }

    start = pixie_gettime();
    reader_thread = pixie_begin_thread(reader, 0, &fd[0]);
    writer_thread = pixie_begin_thread(writer, 0, &fd[1]);
    pixie_join(reader_thread, 0);
    pixie_join(writer_thread, 0);
    stop = pixie_gettime();

    {
        double ellapsed = (stop-start)/1000000.0;
        double speed = BENCH_ITERATIONS*1.0/ellapsed;
        printf("\nbenchmark: pipe msg rate\n");
        printf("rate = %5.4f mega-msgs/sec\n", speed/1000000.0);
        printf("time = %5.3f usec\n", 1000000.0/speed);
    }
}
