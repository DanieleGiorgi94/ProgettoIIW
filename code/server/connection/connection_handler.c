#include "../header.h"

static void release_resources(request_t *, server_info *);

void *create_connection(void *arg)
{
    struct service_thread *ptd = (struct service_thread *) arg;

    server_info *srv_info = ptd->srv_info;
    int sockfd = srv_info->sockfd;
    int new_sockfd = srv_info->new_sockfd;
    struct sockaddr_in servaddr = srv_info->servaddr;
    char client_isn = srv_info->client_isn;

    srand(time(NULL) + getpid());

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    char server_isn = rand() % 100;
    u32 slen = sizeof(struct sockaddr);

    //SYN-ACK (con invio del port number)
    req->initial_n_seq = server_isn;
    req->SYN = 1;
    req->ACK = client_isn + 1;
    req->FIN = 0;
    req->port_number = srv_info->port_number;

    if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
               (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        release_resources(req, srv_info);
        perror("Errore in sendto");
        exit(EXIT_FAILURE);
    }
    //printf("Sent SYN-ACK with n_seq=%lu, %d\n", req->initial_n_seq, req->ACK);

    // Waiting for SOCK_START
//    while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
//                    (struct sockaddr *) &servaddr, &slen) < 0) {
//        if (errno != EAGAIN && errno != EWOULDBLOCK) {
//            perror("recvfrom()");
//            exit(EXIT_FAILURE);
//        }
//    }
//    if (req->type != SOCK_START) {
//        //printf("%d\n", req->type);
//        goto SYNACK;
//    }

    //ACK (attesa sulla nuova socket)
    while (recvfrom(new_sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
            (struct sockaddr *) &srv_info->cliaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() failed");
            release_resources(req, srv_info);
            return NULL;
        }
    }
   // printf("Received ACK with n_seq=%lu\n", req->initial_n_seq);

    if ((req->SYN == 0 && req->ACK == server_isn + 1) || req->FIN > 0) {
        //printf(" ******* 3Way Handshake completed ******** \n");
        while (recvfrom(new_sockfd, (void *) req, sizeof(request_t),
                MSG_DONTWAIT, (struct sockaddr *) &srv_info->cliaddr, &slen)
                                                                        < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom() failed");
                release_resources(req, srv_info);
                return NULL;
            }
        }
        printf("Received request: ");
        printf("%s\n", req->payload);

        if (req->type == LIST_REQ)
            list_command_handler(new_sockfd, srv_info->cliaddr, srv_info->path);
        else if (req->type == GET_REQ)
            get_command_handler(new_sockfd, srv_info->cliaddr, req->payload,
                srv_info->path);
        else if (req->type == PUT_REQ)
            put_command_handler(new_sockfd, srv_info->cliaddr, req->payload,
                srv_info->path);

        release_resources(req, srv_info);
        return NULL;
    } else {
        release_resources(req, srv_info);
        return NULL;
    }
}
static void release_resources(request_t *req, server_info *srv_info)
{
    free(req);
    *(srv_info->no_connections) -= 1;
    close(srv_info->new_sockfd);
    srv_info->ports->available[srv_info->port_number - PORT - 1] = 0;
}
