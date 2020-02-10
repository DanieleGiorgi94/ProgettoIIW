#include "../header.h"

void send_request(char *cmd, char *token, client_info *c_info)
{
    int sockfd = c_info->new_sockfd;
    struct sockaddr_in cliaddr = c_info->cliaddr;
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

    if (strncmp(cmd, "get", 4) == 0) {
        req->type = GET_REQ;
        strncpy(req->payload, token, BUFLEN);
    }
    if (strncmp(cmd, "put", 4) == 0) {
        req->type = PUT_REQ;
        strncpy(req->payload, token, BUFLEN);
    }
    if (strncmp(cmd, "list", 5) == 0)
        req->type = LIST_REQ;

    if (sendto(sockfd, req, sizeof(request_t), 0, (struct sockaddr *) &cliaddr,
            sizeof(cliaddr)) < 0) {
        perror("errore in sendto");
        exit(EXIT_FAILURE);
    }
    //printf("Sent request\n");
    free_allocation(req);
}
