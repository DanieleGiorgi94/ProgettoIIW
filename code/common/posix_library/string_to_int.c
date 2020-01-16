#include "posix_library.h"

int string_to_int(char *str)
{
    int v;
    char *errptr;
    errno = 0;
    v = (int) strtol(str, &errptr, 0);
    if (errno != 0 || *errptr != '\0'){
        fprintf(stderr, "Not a valid number: %s \n", str);
        exit(EXIT_FAILURE);
    }
    return v;
}
