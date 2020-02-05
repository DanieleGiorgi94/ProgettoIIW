#include "../header.h"

static void release_resources(request_t *, char *, int);

void *create_connection(void *arg) {
    struct service_thread *ptd = (struct service_thread *) arg;

    server_info *srv_info = ptd->srv_info;
    int sockfd = srv_info->sockfd;
    int new_sockfd = srv_info->new_sockfd;
    struct sockaddr_in servaddr = srv_info->servaddr;
    char *no_connections = srv_info->no_connections;
    char client_isn = srv_info->client_isn;

    srand(time(NULL) + getpid());

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    char server_isn = rand() % 100;
    u32 slen = sizeof(struct sockaddr);


    SYNACK:

    //SYN-ACK (con invio del port number)
    req->initial_n_seq = server_isn;
    req->SYN = 1;
    req->ACK = client_isn + 1;
    req->FIN = 0;
    req->port_number = srv_info->port_number;

    if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
               (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        release_resources(req, no_connections, sockfd);
        perror("Errore in sendto: invio del pacchetto twh_request_t");
        exit(EXIT_FAILURE);
    }
    printf("Sent SYN-ACK with n_seq=%lu, %d\n", req->initial_n_seq, req->ACK);


    // Waiting for SOCK_START
    while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                    (struct sockaddr *) &servaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() (ricezione del pacchetto request_t)");
            exit(EXIT_FAILURE);
        }
    }
    if ( req->type != SOCK_START ) {
        printf("%d\n", req->type);
        goto SYNACK;
    }

    printf("SOCK_START received.\n");

    /* Qua ho fatto in modo che ascolto solo se è stata effettivamente
     * creata la socket nel client. Dovremmo fare una recvfrom
     * preventiva sulla socket vecchia aspettando un qualche type
     * speciale che indichi che è stata creata.
     * OVviamente forse è da migliorare
     */

    //ACK (attesa sulla nuova socket)
    while (recvfrom(new_sockfd, (void *) req, sizeof(request_t),
                    MSG_DONTWAIT, (struct sockaddr *) &servaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() failed");
            release_resources(req, no_connections, sockfd);
            return NULL;
        }
    }
    printf("Received ACK with n_seq=%lu\n", req->initial_n_seq);

    if (req->SYN == 0 && req->ACK == server_isn + 1) {
        printf("SI\n");
        release_resources(req, no_connections, sockfd);
        return NULL;
    } else {
        release_resources(req, no_connections, sockfd);
        return NULL;
    }
}

static void release_resources(request_t *req, char *no_connections, int sockfd)
{
    free(req);
    *no_connections -= 1;
    close(sockfd);
}
