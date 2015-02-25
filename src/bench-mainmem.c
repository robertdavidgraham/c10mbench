#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "pixie-memsize.h"
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
#else
#include <unistd.h>
#endif

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*10)

struct MainmemParms {
    uint64_t memsize;
    unsigned char *pointer;
    unsigned result;
    unsigned id;
};

unsigned primes[] = {
    1003001,
    1008001,
    1114111,
    1221221,
    1881881,
    3007003,
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

/******************************************************************************
 ******************************************************************************/
static void
worker_thread(void *v_parms)
{
    size_t i;
    struct MainmemParms *parms = (struct MainmemParms *)v_parms;
    unsigned result = 0;
    unsigned char *pointer = parms->pointer;
    uint64_t offset = 0;
    uint64_t memsize = parms->memsize;
    unsigned jump = primes[parms->id];

    //pixie_cpu_set_affinity(parms->id);
    for (i=0; i<BENCH_ITERATIONS2; i++) {
        result += pointer[(size_t)offset];
        offset += jump + pointer[(size_t)offset];
        while (offset > memsize)
            offset -= memsize;
    }
    
    pixie_locked_add_u32(&parms->result, result);
    free(parms);
}

//https://wiki.linaro.org/LEG/Engineering/Kernel/HugePages

#if defined(WIN32)

void win_perror(const char *str)
{
    DWORD dwError = GetLastError();
    LPVOID lpvMessageBuffer;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dwError,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPSTR)&lpvMessageBuffer, 0, NULL);

    //... now display this string
    fprintf(stderr, "%s: %s\n", str, lpvMessageBuffer);

    
    // Free the buffer allocated by the system
    LocalFree(lpvMessageBuffer);

}

//http://blogs.msdn.com/b/oldnewthing/archive/2011/01/28/10121300.aspx
void Privilege(TCHAR* pszPrivilege, BOOL bEnable)
{
  HANDLE      hToken;
  TOKEN_PRIVILEGES tp;
  BOOL       status;
  DWORD      error;

  OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
  LookupPrivilegeValue(NULL, pszPrivilege, &tp.Privileges[0].Luid);
  tp.PrivilegeCount = 1;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  status = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
  if (status) {
      win_perror("SeLockMemoryPrivilege");
  }
  error = GetLastError();
  CloseHandle(hToken);
}

#endif

/******************************************************************************
 ******************************************************************************/
void
bench_mainmem(unsigned cpu_count)
{
    size_t i;
    struct MainmemParms parms[1];
    

    parms->memsize = pixie_get_memsize()/8;
    parms->pointer = NULL;
    while (parms->pointer == NULL) {
        
#if defined(WIN32)
        {
            size_t align = GetLargePageMinimum();

            //Privilege(TEXT("SeLockMemoryPrivilege"), TRUE);

            while (parms->memsize % align)
                parms->memsize++;
            parms->pointer = (unsigned char*)VirtualAlloc(0, (size_t)parms->memsize, MEM_LARGE_PAGES|MEM_COMMIT, PAGE_READWRITE);
            if (parms->pointer == NULL && GetLastError() == ERROR_PRIVILEGE_NOT_HELD) {
                parms->pointer = (unsigned char *)VirtualAlloc(0, (size_t)parms->memsize, MEM_COMMIT, PAGE_READWRITE);
            }
        }
#else
        parms->pointer = (unsigned char*)malloc((size_t)parms->memsize);
#endif
        if (parms->pointer == NULL)
            parms->memsize /= 2;
    }
    memset(parms->pointer, 0, (size_t)parms->memsize);
    parms->result = 0;

    fprintf(stderr, "memsize = %llu\n", (uint64_t)parms->memsize);
    
    /*for (i=0; i<parms->memsize; i++) {
        parms->pointer[i] = (unsigned char)i;
    }*/
    
    for (i=0; i<cpu_count; i++) {
        unsigned j;
        double ellapsed;
        double speed;
        size_t thread_handles[256];
        size_t thread_count = 0;
        uint64_t start, stop;
        
        start = pixie_gettime();
        for (j=0; j<=i; j++) {
            struct MainmemParms *parms2 = (struct MainmemParms*)malloc(sizeof(*parms2));
            parms->id = j;
            memcpy(parms2, parms, sizeof(parms2[0]));
            thread_handles[thread_count++] = pixie_begin_thread(worker_thread, 0, parms2);
        }
        for (j=0; j<thread_count; j++)
            pixie_join(thread_handles[j], 0);
        stop = pixie_gettime();
        
        
        ellapsed = (stop-start)/1000000.0;
        speed = BENCH_ITERATIONS2*1.0/ellapsed;
        
        printf("mainmem,     %2u,  %7.3f,  %7.1f\n",
               (unsigned)thread_count,
               speed/1000000.0,
               1000000000.0/speed);
    }
}
