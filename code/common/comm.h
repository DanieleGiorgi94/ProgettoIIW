#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#include <dirent.h>

#define TEST 0
#define ADAPTIVE 0

void send_file(int, struct sockaddr *, int);
void receive_file(int, struct sockaddr *, int);
char *obtain_path(char *, char *, char);
