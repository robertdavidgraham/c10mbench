#include "pixie-mem.h"

#if defined(WIN32)
#include <Windows.h>
#elif defined (__APPLE__) || defined (__MACH__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <sys/mman.h>
#include <errno.h>
#elif defined (BSD)
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#elif defined (__unix__) || defined (__unix) || defined (unix)
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#elif
#error pixie-mem.c: unsupported OS
#endif


/******************************************************************************
 ******************************************************************************/
size_t
pixie_align_huge(size_t size)
{
    size_t align;
    
#if defined(WIN32)
    align = GetLargePageMinimum();
#else
    if (sizeof(void*) == 8)
        align = 2 * 1024 * 1024;
    else
        align = 4 * 1024 * 1024;
#endif
    size = (size + (align-1)) & (~align);
}

/******************************************************************************
 * Allocates huge pages
 ******************************************************************************/
void *
pixie_alloc_huge(size_t size, int *err)
{
    
#if defined(WIN32)
    void *result;
    
    /* On Windows, the user account needs privileges to use huge pages, which
     * it doesn't by default */
    //Privilege(TEXT("SeLockMemoryPrivilege"), TRUE);
    
    /* Attempt the allocation */
    result = VirtualAlloc(
                          0, /* have the OS assign an address */
                          size,
                          MEM_LARGE_PAGES | MEM_COMMIT, 
                          PAGE_READWRITE);
    
    /* Handle permission error  */
    if (result == NULL && GetLastError() == ERROR_PRIVILEGE_NOT_HELD) {
        *err = HugeErr_NoPermissions;
        return result;
    }
    
    /* Handle fragmented error */
    if (presult == NULL && GetLastError() == 3) {
        *err = HugeErr_MemoryFragmented;
        return result;
    }
    
    /* Handle any other error */
    if (result == NULL && GetLastError() == 3) {
        *err = HugeErr_Unknown;
        return result;
    }
    
    *err = 0;
    return result;
#elif defined(__APPLE__)
    int kr;
    mach_vm_address_t pointer = 0;
    
    

    kr = mach_vm_allocate(
                          mach_task_self(), 
                          &pointer, 
                          size, 
                          VM_FLAGS_ANYWHERE | VM_FLAGS_SUPERPAGE_SIZE_ANY);
    
    /* Handle fragmented error */
    if (kr == ESRCH) {
        *err = HugeErr_MemoryFragmented;
        return 0;
    }
    
    /* Handle any other error */
    if (pointer == 0) {
        *err = HugeErr_Unknown;
        return 0;
    }

    return (void*)pointer;
#else
    void *result;
    
    result = mmap(NULL, /* no existing memory */
                  size,
                  PROT_READ | PROT_WRITE, /* normal read/write */
                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE | MAP_HUGETLB, 
                  -1,   /* no file descriptor */
                  0     /* no offset */
                  );
#endif
}

/******************************************************************************
 ******************************************************************************/
void
pixie_free_huge(void *p, size_t size)
{
#if defined (WIN32)
    VirtualFree(p, 0, MEM_RELEASE);
#elif defined (__APPLE__)
    mach_vm_deallocate(mach_task_self(), (mach_vm_address_t)p, size);
#else
    munmap(p, size);
#endif
}


/******************************************************************************
 * Returns the size of physical memory (RAM) in bytes.
 ******************************************************************************/
size_t
pixie_get_memsize(void)
{
    /*
     * Author:  David Robert Nadeau
     * Site:    http://NadeauSoftware.com/
     * License: Creative Commons Attribution 3.0 Unported License
     *          http://creativecommons.org/licenses/by/3.0/deed.en_US
     * Downloaded from:
     *  http://nadeausoftware.com/articles/2012/09/c_c_tip_how_get_physical_memory_size_system
     */
#if defined(_WIN32) && (defined(__CYGWIN__) || defined(__CYGWIN32__))
	/* Cygwin under Windows. ------------------------------------ */
	/* New 64-bit MEMORYSTATUSEX isn't available.  Use old 32.bit */
	MEMORYSTATUS status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatus( &status );
	return (size_t)status.dwTotalPhys;
    
#elif defined(_WIN32)
	/* Windows. ------------------------------------------------- */
	/* Use new 64-bit MEMORYSTATUSEX, not old 32-bit MEMORYSTATUS */
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx( &status );
	return (size_t)status.ullTotalPhys;
    
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
	/* UNIX variants. ------------------------------------------- */
	/* Prefer sysctl() over sysconf() except sysctl() HW_REALMEM and HW_PHYSMEM */
    
#if defined(CTL_HW) && (defined(HW_MEMSIZE) || defined(HW_PHYSMEM64))
	int mib[2];
	mib[0] = CTL_HW;
#if defined(HW_MEMSIZE)
	mib[1] = HW_MEMSIZE;            /* OSX. --------------------- */
#elif defined(HW_PHYSMEM64)
	mib[1] = HW_PHYSMEM64;          /* NetBSD, OpenBSD. --------- */
#endif
	int64_t size = 0;               /* 64-bit */
	size_t len = sizeof( size );
	if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 )
		return (size_t)size;
	return 0L;			/* Failed? */
    
#elif defined(_SC_AIX_REALMEM)
	/* AIX. ----------------------------------------------------- */
	return (size_t)sysconf( _SC_AIX_REALMEM ) * (size_t)1024L;
    
#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
	/* FreeBSD, Linux, OpenBSD, and Solaris. -------------------- */
	return (size_t)sysconf( _SC_PHYS_PAGES ) *
    (size_t)sysconf( _SC_PAGESIZE );
    
#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGE_SIZE)
	/* Legacy. -------------------------------------------------- */
	return (size_t)sysconf( _SC_PHYS_PAGES ) *
    (size_t)sysconf( _SC_PAGE_SIZE );
    
#elif defined(CTL_HW) && (defined(HW_PHYSMEM) || defined(HW_REALMEM))
	/* DragonFly BSD, FreeBSD, NetBSD, OpenBSD, and OSX. -------- */
	int mib[2];
	mib[0] = CTL_HW;
#if defined(HW_REALMEM)
	mib[1] = HW_REALMEM;		/* FreeBSD. ----------------- */
#elif defined(HW_PYSMEM)
	mib[1] = HW_PHYSMEM;		/* Others. ------------------ */
#endif
	unsigned int size = 0;		/* 32-bit */
	size_t len = sizeof( size );
	if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 )
		return (size_t)size;
	return 0L;			/* Failed? */
#endif /* sysctl and sysconf variants */
    
#else
	return 0L;			/* Unknown OS. */
#endif
}
