#ifndef _HEADER_H_
#error You must not include this sub-header directly
#endif

#define MAX_CMD_LENGTH 5

int create_connection(client_info *);
int close_connection(int, struct sockaddr_in);
void send_request(char *, char *, client_info *);
void send_ack(client_info *, char, int);

struct service_thread {
    client_info *c_info;
    char cmd[MAX_CMD_LENGTH];
    char token[BUFLEN - MAX_CMD_LENGTH];
    pthread_t tid;
};
