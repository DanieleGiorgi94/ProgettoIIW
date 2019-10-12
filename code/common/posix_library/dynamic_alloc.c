#include "posix_library.h"

void *dynamic_allocation(size_t size)
{
    void *buf = (void *) malloc(size);
    print_error("malloc()", buf);
    print_error("memset()", memset(buf, 0, size));
    return buf;
}
void free_allocation(void *s)
{
    free(s);
}
