#include "../header.h"

void *create_connection(void *arg) {
    struct service_thread *ptd = (struct service_thread *) arg;

    int sockfd = ptd->sockfd;
    struct sockaddr_in servaddr = ptd->servaddr;
    char *no_connections = ptd->no_connections;

    u32 slen = sizeof(struct sockaddr);
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

    //invia il SYNACK
    //attendi l'ACK
    //attendi il comando del client
    while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                                    (struct sockaddr *) &servaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() failed");
            free(req);
            return NULL;
        }
    }
    if (req->type == GET_REQ)
        get_command_handler(sockfd, servaddr, req->filename);
    else if (req->type == PUT_REQ)
        put_command_handler(sockfd, servaddr, req->filename);
    else if (req->type == LIST_REQ)
        list_command_handler(sockfd, servaddr);

    free(req);
    *no_connections -= 1;
    return NULL;
}
