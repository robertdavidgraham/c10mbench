#ifndef PIXIE_STRERROR_H
#define PIXIE_STRERROR_H
#include <string.h>

#if defined(WIN32)
const char *
pixie_strerror(unsigned err);
#else
#define pixie_strerror(n) strerror(n)
#endif




#endif
