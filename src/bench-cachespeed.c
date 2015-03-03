#include <stdio.h>
#include <stdlib.h>
#include "pixie-timer.h"
#include "pixie-threads.h"

#define BENCH_ITERATIONS 100000000

size_t myrand(unsigned long long *state)
{
    *state = *state * 6364136223846793005UL + 1442695040888963407UL;
    return (size_t)((*state)>>16);
}
void randomize(size_t array[], size_t size)
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

void bench(unsigned size, unsigned half, unsigned quarter)
{
    size_t *array;
    size_t count = (1<<size)/sizeof(size_t);
    size_t iterations = BENCH_ITERATIONS/size;
    size_t i;
    size_t index = 0;
    unsigned long long start, stop;
    double ellapsed, speed;
    const char *test_name = "cachelatency";
    
    if (half)
        count += count/2;
    if (quarter)
        count += count/4;
    
    array = (size_t*)malloc(count * sizeof(size_t));
    if (array == 0) {
        fprintf(stderr, "out of memory: malloc(%llu)\n", (unsigned long long)count * sizeof(size_t));
        exit(1);
    }
    randomize(array, count);
    
    start = pixie_gettime();
    for (i=0; i<iterations; i++)
        index = array[index];
    stop = pixie_gettime();
    ellapsed = (stop-start)/1000000.0;
    //printf("%llu: %llu %7.3f\n", (unsigned long long)index, stop - start, ellapsed);
    speed = iterations*1.0/ellapsed;
    printf("%-12s, %2u%s,  %7llu,  %7.1f\n",
           test_name,
           (unsigned)size,
           quarter?(half?".75":".25"):(half?".50":".00"),
           (unsigned long long)count*sizeof(size_t)/1024UL,
           1000000000.0/speed,
           index);
    free(array);

}

int mainx(int argc, char *argv[])
{
    size_t array[16];
    size_t i;
    
    randomize(array, 16);
    
    for (i=0; i<sizeof(array)/sizeof(array[0]); i++) {
        printf("%2u ", (unsigned)i);
    }
    printf("\n");
    for (i=0; i<sizeof(array)/sizeof(array[0]); i++) {
        printf("%2u ", (unsigned)array[i]);
    }
    printf("\n");
    for (i=12; i<30; i++) {
        fprintf(stderr, "%llu\n", (unsigned long long)i);
        bench(i, 0, 0);
        bench(i, 0, 1);
        bench(i, 1, 0);
        bench(i, 1, 1);
    }
    
}
