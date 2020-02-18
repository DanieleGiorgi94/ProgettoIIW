#include "../header.h"

static void send_ack(client_info *, char, int);

int create_connection(client_info *c_info)
{
    int sockfd = c_info->sockfd;
    u64 new_port;
    int new_sockfd;
    struct sockaddr_in servaddr = c_info->servaddr;
    struct sockaddr_in cliaddr;
    char *argv = c_info->argv;
    u32 slen = sizeof(struct sockaddr);

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
    while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
            (struct sockaddr *) &cliaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() while waiting SYN-ACK");
            exit(EXIT_FAILURE);
        }
    }
    //printf("Received SYN-ACK with ack=%d and nseq=%lu\n", req->ACK,
     //                                                   req->initial_n_seq);

    if (req->SYN == 1 && req->ACK == client_isn + 1 && req->FIN == 0) {

        new_port = req->port_number;
        client_isn = req->ACK;
        svr_isn = req->initial_n_seq;

        //printf("%d, %d\n", cliaddr.sin_port, cliaddr.sin_addr.s_addr);
        //printf("%d, %d\n", servaddr.sin_port, servaddr.sin_addr.s_addr);
        //printf("%lu\n", new_port);

        //creo la nuova socket privata per l'invio dei dati
        create_new_socket(&new_sockfd, &cliaddr, new_port, argv);

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
int close_connection(int sockfd, struct sockaddr_in servaddr){

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    u32 slen = sizeof(struct sockaddr);

    srand(time(NULL) + getpid());
    u64 client_isn = rand() % 100;
    //Invia FIN

    SEND_FIN:

    req->initial_n_seq = client_isn;
    req->FIN = 1;
    req->SYN = 0;
    req->ACK = 0;
    req->type = EXIT_REQ;

    if (sendto(sockfd, (void *)req, sizeof(request_t), 0, (struct sockaddr *) &servaddr,
               sizeof(servaddr)) < 0) {
        free_allocation(req);
        perror("Errore in sendto: invio del pacchetto request_t");
        exit(EXIT_FAILURE);
    }

    //printf("Sent FIN %d, client_isn: %lu\n", req->FIN, req->initial_n_seq);

    // Aspetta ACK

    clock_t tspan;
    tspan = clock();

    while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                    (struct sockaddr *) &servaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() (ricezione del pacchetto request_t)");
            exit(EXIT_FAILURE);
        }
        if (clock() - tspan > 1000){
            goto SEND_FIN;
        }
    }
    //printf("Received ACK %d\n",req->ACK);

    if (req->ACK == 2){
        // Aspetta FIN del server
        while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                        (struct sockaddr *) &servaddr, &slen) < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom() (ricezione del pacchetto request_t)");
                exit(EXIT_FAILURE);
            }
        }
        if (req->FIN){
            // invia ACK
            req->initial_n_seq = client_isn+1;
            if (sendto(sockfd, (void *)req, sizeof(request_t), 0, (struct sockaddr *) &servaddr,
                       sizeof(servaddr)) < 0) {
                free_allocation(req);
                perror("Errore in sendto: invio del pacchetto request_t");
                exit(EXIT_FAILURE);
            }
            //printf("Sent ACK %d\n", req->ACK);

            return 1;

        }else{
            printf("FIN from server not received.\n");

            return 0;
        }
    }else{
        printf("ACK not correctly received.\n");

        return 0;
    }
}
static void send_ack(client_info *c_info, char svr_isn, int sockfd)
{
    struct sockaddr_in servaddr = c_info->servaddr;
    struct sockaddr_in cliaddr = c_info->cliaddr;

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    req->initial_n_seq = c_info->client_isn;
    req->type = SOCK_START;

//    if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
//               (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
//        free_allocation(req);
//        perror("sendto() while sending SYN");
//        exit(EXIT_FAILURE);
//    }
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
