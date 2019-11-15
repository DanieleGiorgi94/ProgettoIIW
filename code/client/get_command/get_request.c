#include "../header.h"

/*
    Funzione che richiede un file al server ed attende sua risposta
*/
int request_file(struct sockaddr_in servaddr, int sockfd)
{
    printf("Received GET command...\n");

    int fd = open_file(PATH, O_RDONLY);

    //invia file al buffer circolare
    send_file(sockfd, (struct sockaddr *) &servaddr, fd);

    close_file(fd);
    return 1;
}
