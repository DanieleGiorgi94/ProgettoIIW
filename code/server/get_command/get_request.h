#ifndef _HEADER_H_
#error You must not include this sub-header file directly
#endif

void get_command_handler(int, struct sockaddr_in, char *, char *);

char *list_dir(char *PATH);

int check_file(char *file, char *list);
