#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#include <dirent.h>

char *list_dir(char *PATH);
int check_file(char *file, char *list);
void send_file(int, int, struct sockaddr *, struct sockaddr *, int, char *);
void receive_file(int, int, struct sockaddr *, struct sockaddr *, int);
