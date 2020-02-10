#ifndef _HEADER_H_
#error You must not include this sub-header file directly
#endif

typedef struct {
    int sockfd;
    int new_sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    char *argv;
    char connected;
    u64 port_number;
    char client_isn;
} client_info;

#define MAX_CMD_LENGTH 5

void send_request(char *, char *, client_info *);

struct service_thread {
    client_info *c_info;
    char cmd[MAX_CMD_LENGTH];
    char token[BUFLEN - MAX_CMD_LENGTH];
    pthread_t tid;
};
