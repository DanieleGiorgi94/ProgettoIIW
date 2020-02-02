#include "../header.h"

void exit_command_handler(int sockfd, int fin, char *no_connections, struct sockaddr_in servaddr){

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    u32 slen = sizeof(struct sockaddr);

    srand(time(NULL));
    u64 server_isn = rand() % 100;

    printf("Received FIN\n");

    // Invia ACK
    req->initial_n_seq = server_isn;
    req->ACK = fin + 1;

    if (sendto(sockfd, (void *) req, sizeof(request_t), 0, (struct sockaddr *) &servaddr,
               sizeof(servaddr)) < 0) {
        free_allocation(req);
        perror("Errore in sendto: invio del pacchetto request_t");
        exit(EXIT_FAILURE);
    }
    printf("Sent ACK %d\n", req->ACK);

    // Invia FIN
    req->initial_n_seq = server_isn + 1;
    req->FIN = 1;
    if (sendto(sockfd, (void *) req, sizeof(request_t), 0, (struct sockaddr *) &servaddr,
               sizeof(servaddr)) < 0) {
        free_allocation(req);
        perror("Errore in sendto: invio del pacchetto request_t");
        exit(EXIT_FAILURE);
    }
    printf("Sent FIN %d\n", req->FIN);

    // aspetta ACK
    while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                    (struct sockaddr *) &servaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() (ricezione del pacchetto request_t)");
            exit(EXIT_FAILURE);
        }
    }
    if (req->ACK == 2) {

        *no_connections -= 1;
    } else {
        printf("ACK from client not received.\n");
        exit(EXIT_FAILURE);
        //TODO Anche qui dovremmo metttere un timeout di ri tx!
    }
}

