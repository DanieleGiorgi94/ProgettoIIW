#ifndef _HEADER_H_
#error You must not include this sub-header directly
#endif

int create_connection(client_info *);
int close_connection(int, struct sockaddr_in);
