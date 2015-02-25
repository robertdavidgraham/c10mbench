
#if defined(_MSC_VER)
#include <windows.h>
const char *
pixie_strerror(unsigned dwError)
{
    __declspec( thread ) static char buffer[65536];
    char *p = buffer;

    
    FormatMessageA(
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dwError,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  p, 0, NULL);

    return buffer;
}

#endif
