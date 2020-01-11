#include "posix_library.h"

pid_t execute_command_redirecting_output(char **cmd, int no_arguments,
                                                                char *pathname)
{
    pid_t new_pid = fork();
    print_error("fork()", &new_pid);
    if (new_pid == 0) {
        int fd = open_file(pathname, O_WRONLY);
        int res = dup2(fd, STDOUT_FILENO);
        print_error("dup2()", &res);
        close_file(fd);

        char * argv[1+no_arguments+1];
        for (int i = 0; i < 1+no_arguments; ++i)
            argv[i] = cmd[i];
        argv[2+no_arguments] = NULL;
        execve(argv[0], argv, argv+no_arguments+1);
        fprintf(stderr, "Cannot execute %s\n", cmd[0]);
        print_error("execve()", NULL);
    }
    return new_pid;
}
pid_t execute_command(char **cmd, int no_arguments)
{
    pid_t new_pid = fork();
    print_error("fork()", &new_pid);
    if (new_pid == 0) {
        char * argv[1+no_arguments+1];
        for (int i = 0; i < 1+no_arguments; ++i)
            argv[i] = cmd[i];
        argv[2+no_arguments] = NULL;
        execve(argv[0], argv, argv+no_arguments+1);
        fprintf(stderr, "Cannot execute %s\n", cmd[0]);
        print_error("execve()", NULL);
    }
    return new_pid;
}
