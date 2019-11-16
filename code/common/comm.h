#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#include <dirent.h>

char *list_dir(char *PATH);
int check_file(char *file, char *list);
void send_file(int, struct sockaddr *, int);
void receive_file(int, struct sockaddr *, int);
/*void send_ack(int, struct sockaddr, u64);
ack_t recv_ack(int, struct sockaddr);*/
