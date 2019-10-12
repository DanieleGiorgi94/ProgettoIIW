#ifndef __POSIX_LIBRARY_H_
#error You must not include this sub-header file directly
#endif

struct node {
    char value;
    struct node *next;
};
