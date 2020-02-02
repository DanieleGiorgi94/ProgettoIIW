#ifndef _HEADER_H_
#error You must not include this sub-header directly
#endif

int create_connection(int, struct sockaddr_in, char *cmd, char *token);

int close_connection(int, struct sockaddr_in);
