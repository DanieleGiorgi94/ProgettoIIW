#include "../header.h"

int create_connection(client_info *c_info)
{
    int sockfd = c_info->sockfd;
    int new_port;
    int new_sockfd;
    struct sockaddr_in servaddr = c_info->servaddr;
    struct sockaddr_in cliaddr;
    char *argv = c_info->argv;

    srand(time(NULL) + getpid());

    // Three-way handshake
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

    char client_isn = rand() % 100;
    char svr_isn;

    //SYN
    req->initial_n_seq = client_isn;
    req->SYN = 1;
    req->ACK = 0;
    req->FIN = 0;

    //printf("Created new client_isn: %d\n", client_isn);

    if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
            (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        free_allocation(req);
        perror("sendto() while sending SYN");
        exit(EXIT_FAILURE);
    }
    //printf("Sent SYN with n_seq=%lu\n", req->initial_n_seq);

    //SYN-ACK (qua riceve la nuova port number)
    while (recvfrom(sockfd, (void *) req, sizeof(request_t),
            MSG_DONTWAIT, NULL, NULL) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() while waiting SYN-ACK");
            exit(EXIT_FAILURE);
        }
    }
    //printf("Received SYN-ACK with ack=%d and nseq=%lu\n", req->ACK,
    //                                                    req->initial_n_seq);

    if (req->SYN == 1 && req->ACK == client_isn + 1 && req->FIN == 0) {

        new_port = req->port_number;
        client_isn = req->ACK;
        svr_isn = req->initial_n_seq;

        //creo la nuova socket privata per l'invio dei dati
        if ((new_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("socket() failed");
            exit(EXIT_FAILURE);
        }
        memset((void *) &cliaddr, 0, sizeof(cliaddr));
        cliaddr.sin_family = AF_INET;
        cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        cliaddr.sin_port = htons(new_port);
        if (inet_pton(AF_INET, argv, &cliaddr.sin_addr) <= 0) {
            fprintf(stderr, "errore in inet_pton per %s", argv);
            exit(EXIT_FAILURE);
        }

        c_info->client_isn = client_isn;
        c_info->cliaddr = cliaddr;
        c_info->port_number = new_port;
        c_info->new_sockfd = new_sockfd;

        send_ack(c_info, svr_isn, sockfd);

        return 1;
    } else {
        return 0;
    }
}
int close_connection(int sockfd, struct sockaddr_in servaddr)
{
    return 0;
}
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
void send_ack(client_info *c_info, char svr_isn, int sockfd)
{
    struct sockaddr_in servaddr = c_info->servaddr;
    struct sockaddr_in cliaddr = c_info->cliaddr;

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    req->initial_n_seq = c_info->client_isn;
    req->type = SOCK_START;

    if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
               (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        free_allocation(req);
        perror("sendto() while sending SYN");
        exit(EXIT_FAILURE);
    }
    //printf("SOCK_START sent.\n");

    //ACK (sulla nuova socket)
    free_allocation(req);
    req = (request_t *) dynamic_allocation(sizeof(request_t));

    req->initial_n_seq = c_info->client_isn;
    req->ACK = svr_isn + 1;
    req->SYN = 0;
    req->FIN = 0;

    if (sendto(c_info->new_sockfd, req, sizeof(request_t), 0,
               (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0) {
        perror("errore in sendto");
        exit(EXIT_FAILURE);
    }
    //printf("Sent ACK with n_seq=%lu\n", req->initial_n_seq);
}
