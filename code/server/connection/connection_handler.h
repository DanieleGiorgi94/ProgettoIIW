#ifndef _HEADER_H_
#error You must not include this sub-header directly
#endif

#define MAX_CONNECTIONS 10

struct service_thread {
    int sockfd;
    struct sockaddr_in servaddr;
    char *no_connections;
    char *path;
    pthread_t tid;
};

void *create_connection(void *);
