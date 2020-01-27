#include "../header.h"

void *create_connection(void *arg) {
    struct service_thread *ptd = (struct service_thread *) arg;

    int sockfd = ptd->sockfd;
    struct sockaddr_in servaddr = ptd->servaddr;
    char *no_connections = ptd->no_connections;
    char *path = ptd->path;
    u64 client_isn = ptd->isn;

    srand(time(NULL) + getpid());

    u32 slen = sizeof(struct sockaddr);
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

    u64 server_isn = rand() % 100;


    req->initial_n_seq = server_isn;
    req->SYN = 1;
    req->ACK = client_isn + 1;
    req->FIN = 0;

    if (sendto(sockfd, (void *) req, sizeof(request_t), 0, (struct sockaddr *) &servaddr,
               sizeof(servaddr)) < 0) {
        free_allocation(req);
        perror("Errore in sendto: invio del pacchetto request_t");
        exit(EXIT_FAILURE);
    }
    //printf("Sent SYN-ACK %d %d, server_isn: %lu\n", req->SYN, req->ACK, req->initial_n_seq);


    while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT, //waiting for ACK + REQ
                    (struct sockaddr *) &servaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() failed");
            free(req);
            return NULL;
        }
    }
    printf("ACK %d received.\n", req->ACK);

    if (req->SYN == 0 && req->ACK == (char) server_isn + 1) {

        /******* 3Way Handshake completed ********/

      /*  //attendi il comando con il filename del client
        printf("Waiting for REQ from a client...\n");
        while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                        (struct sockaddr *) &servaddr, &slen) < 0
               && req->type <= 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom() failed");
                free(req);
                return NULL;
            }
        }
*/
        printf("%s\n", req->filename);

        if (req->type == GET_REQ)
            get_command_handler(sockfd, servaddr, req->filename, path);
        else if (req->type == PUT_REQ)
            put_command_handler(sockfd, servaddr, req->filename);
        else if (req->type == LIST_REQ)
            list_command_handler(sockfd, servaddr);


    }

    printf("ACK not correctly received.\n");

    free(req);
    *no_connections -= 1;
    return NULL;

}
