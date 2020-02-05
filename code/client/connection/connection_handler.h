#ifndef _HEADER_H_
#error You must not include this sub-header directly
#endif

int create_connection(char *, char *, client_info *, char);
int close_connection(int, struct sockaddr_in);
void send_request(char *, char *, client_info *);
void send_ack(client_info *, char, int);
