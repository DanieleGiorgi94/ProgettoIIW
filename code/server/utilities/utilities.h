#ifndef _HEADER_H_
#error You must not include this sub-header file directly
#endif

char *list_dir(char *);
int check_file(char *, char *);

typedef struct {
    int sockfd;
    int new_sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    char *path;
    u64 port_number;
    char *no_connections;
    char client_isn;
    struct available_ports *ports;
} server_info;
