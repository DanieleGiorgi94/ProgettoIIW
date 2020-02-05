#include "posix_library.h"

void send_signal(pid_t pid, int sig)
{
    int r = kill(pid, sig);
    print_error("kill()", &r);
}
