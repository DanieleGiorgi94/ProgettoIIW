#include "posix_library.h"

void print_error(char *s, void *r_value)
{
    int *r_value_int = (int *) r_value;
    if (r_value == NULL || *r_value_int == -1) {
        fprintf(stderr, "%s failed!\n", s);
        perror("error");
        exit(EXIT_FAILURE);
    }
}
