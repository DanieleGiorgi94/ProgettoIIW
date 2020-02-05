#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#define GET_REQ 0
#define PUT_REQ 1
#define LIST_REQ 2
#define FILEON 3
#define FILEOFF 4
#define EXIT_REQ 5

typedef struct {
    u64 initial_n_seq;
    char SYN;
    char ACK;
    char FIN;
    char payload[BUFLEN];
    char type; //GET_REQ, PUT_REQ, LIST_REQ, FILEON, FILEOFF, EXIT_REQ, C_START
    u64 port_number;
} request_t;

typedef struct {
    int sockfd;
    int new_sockfd;
    struct sockaddr_in servaddr;
    char *argv;
    char connected;
    u64 port_number;
} client_info;

typedef struct {
    int sockfd;
    int new_sockfd;
    struct sockaddr_in servaddr;
    char *path;
    u64 port_number;
    char *no_connections;
    char client_isn;
} server_info;
