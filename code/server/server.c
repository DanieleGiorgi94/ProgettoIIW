#include "header.h"

static void main_task(int, struct sockaddr_in);
static void create_service_thread(int, struct sockaddr_in, char *, char *, u64);

static void main_task(int sockfd, struct sockaddr_in servaddr) {
    char *no_connections = dynamic_allocation(sizeof(*no_connections));
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

    char *path = obtain_path(NULL, NULL, 1);
    u32 slen = sizeof(struct sockaddr);

    *no_connections = 0;

RESET:

    /* Waiting for a SYN from a client to start communication */
    while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                                (struct sockaddr *) &servaddr, &slen) < 0) {

        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() failed");
            free(no_connections);
            free(req);
            free(path);
            return;
        }
    }

    if (req->SYN != 1 ) //three-way handshake starts with SYN!
        goto RESET;


    if (*no_connections < MAX_CONNECTIONS) {

        create_service_thread(sockfd, servaddr, no_connections, path, req->initial_n_seq);
        *no_connections += 1;

    }

    goto RESET;
}
static void create_service_thread(int sockfd, struct sockaddr_in servaddr,
        char *no_connections, char *path, u64 client_isn) {
    struct service_thread s_thread;


    s_thread.sockfd = sockfd;
    s_thread.servaddr = servaddr;
    s_thread.no_connections = no_connections;
    s_thread.path = path;
    s_thread.isn = client_isn;

    printf("Ciao sono il client n. %d\n", (char) client_isn);


    if (pthread_create(&s_thread.tid, NULL, create_connection,
                                                &s_thread) != 0) {
        perror("pthread_create() failed");
    }
}
int main() {
    struct sockaddr_in servaddr;
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    memset((void *) &servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("errore in bind");
        exit(EXIT_FAILURE);
    }

    if (TEST == 0) {
        main_task(sockfd, servaddr);
        close(sockfd);
    } else {
        char *path = obtain_path(NULL, NULL, 1);
        int fd = open_file(strncat(path, "prova.txt", 9), O_WRONLY | O_CREAT);

        receive_file(sockfd, (struct sockaddr *) &servaddr, fd);
    }

    exit(EXIT_SUCCESS);
}
