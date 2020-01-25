#include "../header.h"

int create_connection(int sockfd, struct sockaddr_in servaddr) {

    /* 3Way Handshake
     * TODO Rivedere bene pag. 509 Gapil per l'intestazione IP + TCP del pacchetto.
     * Qui noi forse utilizziamo solo un numero di sequenza e flag ma dovremmo avere anche entrambi gli header */

    syn_t *syn = (syn_t *) dynamic_allocation(sizeof(syn_t));
    u32 slen = sizeof(struct sockaddr);

    srand(time(NULL));

    u64 client_isn = rand() % 100;

    syn->initial_n_seq = client_isn;
    syn->SYN = 1;
    syn->ACK = 0;
    syn->FIN = 0;

    // SYN
    if (sendto(sockfd, (void *)syn, sizeof(syn_t), 0, (struct sockaddr *) &servaddr,
               sizeof(servaddr)) < 0) {
        free_allocation(syn);
        perror("Errore in sendto: invio del pacchetto syn_t");
        exit(EXIT_FAILURE);
    }
    printf("Sent SYN %d, client_isn: %lu\n", syn->SYN, syn->initial_n_seq);

    while (recvfrom(sockfd, (void *) syn, sizeof(syn_t), MSG_DONTWAIT,
                    (struct sockaddr *) &servaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() (ricezione del pacchetto request_t)");
            exit(EXIT_FAILURE);
        }
    }
    printf("Received SYN-ACK %d %d\n", syn->SYN, syn->ACK);
    printf("Server_isn: %d\n", (char) syn->initial_n_seq);

    if (syn->SYN == 1  && syn->ACK == (char) client_isn+1 && syn->FIN == 0){  // SYNACK

        syn->ACK = (char) syn->initial_n_seq + 1; // server_isn+1
        syn->initial_n_seq = client_isn+1;
        syn->SYN = 0;

        if (sendto(sockfd, (void *)syn, sizeof(syn_t), 0, (struct sockaddr *) &servaddr,  //ACK
                   sizeof(servaddr)) < 0) {
            free_allocation(syn);
            perror("Errore in sendto: invio del pacchetto syn_t");
            exit(EXIT_FAILURE);
        }
        printf("Sent ACK %d\n", syn->ACK);
        return 1;

    }else{
        printf("Three way handshake failed.\n");
        return 0;
    }
}
