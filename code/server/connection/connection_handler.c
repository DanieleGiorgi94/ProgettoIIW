#include "../header.h"

void *create_connection(void *arg) {
    struct service_thread *ptd = (struct service_thread *) arg;

    printf("1\n");

    int sockfd = ptd->sockfd;
    struct sockaddr_in servaddr = ptd->servaddr;
    char *no_connections = ptd->no_connections;
    char *path = ptd->path;

    printf("2\n");


    u32 slen = sizeof(struct sockaddr);
    printf("3\n");

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    printf("4\n");

    syn_t *syn = (syn_t *) dynamic_allocation(sizeof(syn_t));

    printf("5\n");


    //attendi il comando del client
    while (recvfrom(sockfd, (void *) syn, sizeof(syn_t), MSG_DONTWAIT,
                                    (struct sockaddr *) &servaddr, &slen) < 0) {
        printf("6\n");

        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() failed");
            free(syn);
            return NULL;
        }
    }
    printf("syn received.\n");
    if (syn->SYN > 0){ //SYN received
        syn->initial_n_seq = 2; // ?
        syn->ACK = syn->SYN + 1;
        syn->SYN = syn->initial_n_seq;
        syn->FIN = 0;

        if (sendto(sockfd, (void *) syn, sizeof(syn_t), 0, (struct sockaddr *) &servaddr,
                   sizeof(servaddr)) < 0) {
            free_allocation(syn);
            perror("Errore in sendto: invio del pacchetto syn_t");
            exit(EXIT_FAILURE);
        }
        printf("Sent SYN-ACK %d %d\n", syn->SYN, syn->ACK);

        while (recvfrom(sockfd, (void *) syn, sizeof(syn_t), MSG_DONTWAIT, //waiting for ACK
                        (struct sockaddr *) &servaddr, &slen) < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom() failed");
                free(syn);
                return NULL;
            }
        }

        if (syn->ACK == syn->SYN + 1){

            if (req->type == GET_REQ)
                get_command_handler(sockfd, servaddr, req->filename, path);
            else if (req->type == PUT_REQ)
                put_command_handler(sockfd, servaddr, req->filename);
            else if (req->type == LIST_REQ)
                list_command_handler(sockfd, servaddr);

        }
    }

    printf("cazzo\n");

    free(req);
    *no_connections -= 1;
    return NULL;
}
