#ifndef c10mbench_pixie_memzie_h
#define c10mbench_pixie_memzie_h
#include <stdio.h>

size_t pixie_get_memsize(void);

enum PixieHugeErr {
    HugeErr_Success,
    HugeErr_MemoryFragmented,
    HugeErr_NoPermissions,
    HugeErr_Unknown,
};

size_t
pixie_align_huge(size_t size);


/**
 * Allocate huge pages, the number of bytes requested must be aligned,
 * such as with 'pixie_align_huge()'
 */
void *
pixie_alloc_huge(size_t aligned_size, int *err);

/**
 * Free memory that was allocated by 'pixie_alloc_huge()'. A size
 * parameter must be provided.
 */
void
pixie_free_huge(void *p, size_t size);


#endif
