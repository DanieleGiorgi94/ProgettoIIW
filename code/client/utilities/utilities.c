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
void create_new_socket(int *new_sockfd, struct sockaddr_in *cliaddr,
        int new_port, char *argv)
{
    if ((*new_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }
    memset((void *) cliaddr, 0, sizeof(*cliaddr));
    cliaddr->sin_family = AF_INET;
    cliaddr->sin_addr.s_addr = htonl(INADDR_ANY);
    cliaddr->sin_port = htons(new_port);
    if (inet_pton(AF_INET, argv, &(cliaddr->sin_addr)) <= 0) {
        fprintf(stderr, "errore in inet_pton");
        exit(EXIT_FAILURE);
    }
}
