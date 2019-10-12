#include "posix_library.h"

void sleep_for(unsigned int seconds)
{
    while (seconds > 0)
        seconds = sleep(seconds);
}
void usleep_for(useconds_t usec)
{
    int r =  usleep(usec);
    print_error("usleep()", &r);
}
void nanosleep_for(long nanoseconds)
{
    int r;
    struct timespec *req = (struct timespec *)
                    dynamic_allocation(sizeof(struct timespec *));
    struct timespec *rem = (struct timespec *)
                    dynamic_allocation(sizeof(struct timespec *));

    req->tv_nsec = nanoseconds;
    do {
        r = nanosleep(req, rem);
        print_error("nanosleep()", &r);
    } while (rem->tv_sec > 0);
}
