#include "../header.h"

/*
    Invia il filelist ad un client
*/
void list_command_handler(int sockfd, struct sockaddr_in servaddr, char *path) {

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    srand(time(NULL) + getpid());
    u64 server_isn = rand() % 100;

    strncpy(req->payload, list_dir(path), sizeof((req->payload)));

    req->initial_n_seq = server_isn;

    printf("Invio lista dei miei file a client.\n");
    if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
            (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        free_allocation(req);
        perror("Errore in sendto: invio del pacchetto twh_request_t");
        exit(EXIT_FAILURE);
    }
    return;
}
