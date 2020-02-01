#include "../header.h"

int create_connection(int sockfd, struct sockaddr_in servaddr, char *cmd, char *token) {

    /* 3Way Handshake
     * TODO Rivedere bene pag. 509 Gapil per l'intestazione IP + TCP del pacchetto.
     * Qui noi forse utilizziamo solo un numero di sequenza e flag ma dovremmo avere anche entrambi gli header */

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    u32 slen = sizeof(struct sockaddr);

    srand(time(NULL));

    u64 client_isn = rand() % 100;

    req->initial_n_seq = client_isn;
    req->SYN = 1;
    req->ACK = 0;
    req->FIN = 0;

    // SYN
    if (sendto(sockfd, (void *)req, sizeof(request_t), 0, (struct sockaddr *) &servaddr,
               sizeof(servaddr)) < 0) {
        free_allocation(req);
        perror("Errore in sendto: invio del pacchetto request_t");
        exit(EXIT_FAILURE);
    }
    printf("Sent SYN %d, client_isn: %lu\n", req->SYN, req->initial_n_seq);

    while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                    (struct sockaddr *) &servaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() (ricezione del pacchetto request_t)");
            exit(EXIT_FAILURE);
        }
    }
    printf("Received SYN-ACK %d %d\n", req->SYN, req->ACK);

    if (req->SYN == 1  && req->ACK == (char) client_isn+1 && req->FIN == 0){  // SYNACK


        u64 client_isn = req->ACK;

        req->ACK = (char) req->initial_n_seq + 1; // server_isn+1
        req->initial_n_seq = client_isn; //client_isn +1
        req->SYN = 0;

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



        /* Mando 3 volte perché almeno sono quasi sicuro che il server almeno uno
         * lo riceve!!!
         *
         * TODO Mettere un timer di ritrasmissione ACK al posto di queste 3 chiamate
         * */
        if (sendto(sockfd, req, sizeof(request_t), 0,
                   (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            perror("errore in sendto");
            exit(EXIT_FAILURE);
        }
        if (sendto(sockfd, req, sizeof(request_t), 0,
                   (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            perror("errore in sendto");
            exit(EXIT_FAILURE);
        }if (sendto(sockfd, req, sizeof(request_t), 0,
                    (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            perror("errore in sendto");
            exit(EXIT_FAILURE);
        }

        printf("Sent ACK %d \n", req->ACK);


        return 1;

    }else{
        printf("Three way handshake failed.\n");
        return 0;
    }
}
