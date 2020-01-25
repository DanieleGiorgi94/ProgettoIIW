#include "../header.h"

void get_command_handler(char *cmd, char *token, int sockfd,
                         struct sockaddr_in servaddr) {
    if (token == NULL) {
        fprintf(stderr, "Usage: get <filename.format>\n");
    } else {

        //Manda al server il pacchetto con il nome del file da cercare
        request_t *rqst = (request_t *) dynamic_allocation(sizeof(request_t));

        u32 slen = sizeof(struct sockaddr);

        if (create_connection(sockfd, servaddr)) {
            /* Threeway handshake completed */

            rqst->type = GET_REQ;
            strncpy(rqst->filename, token, BUFLEN);
            printf("Sending GET_REQ.\n");
            if (sendto(sockfd, rqst, sizeof(request_t), 0,
                       (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
                perror("errore in sendto");
                exit(EXIT_FAILURE);
            }

            printf("Waiting for FILEON from server.\n");
            while (recvfrom(sockfd, (void *) rqst, sizeof(request_t), MSG_DONTWAIT,
                            (struct sockaddr *) &servaddr, &slen) < 0
                                    && rqst->type <= 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("recvfrom() (ricezione del pacchetto request_t)");
                    exit(EXIT_FAILURE);
                }
            }

            if (rqst->type == FILEON) {

                printf("Il file è presente\n");


                int fd = open_file(token, O_WRONLY | O_CREAT);
                /* Inizio ricezione file */
                receive_file(sockfd, (struct sockaddr *) &servaddr, fd);
                close_file(fd);

                return;
            }
            if (rqst->type == FILEOFF) {
                printf("Il file %s non è presente nel server.\n", token);
                return;
            }
        }

    }

}