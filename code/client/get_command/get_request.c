#include "../header.h"

void get_command_handler(char *cmd, char *token, int sockfd,
                         struct sockaddr_in servaddr) {
    if (token == NULL) {
        fprintf(stderr, "Usage: get <filename.format>\n");
    } else {

        //Manda al server il pacchetto con il nome del file da cercare
        request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

        u32 slen = sizeof(struct sockaddr);

        if (create_connection(sockfd, servaddr, token)) {
            /* Threeway handshake completed */

            printf("Waiting for FILEON from server.\n");
            while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                            (struct sockaddr *) &servaddr, &slen) < 0
                                    && req->type <= 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("recvfrom() (ricezione del pacchetto request_t)");
                    exit(EXIT_FAILURE);
                }
            }

            if (req->type == FILEON) {

                printf("Il file è presente\n");


                int fd = open_file(token, O_WRONLY | O_CREAT);
                /* Inizio ricezione file */
                receive_file(sockfd, (struct sockaddr *) &servaddr, fd);
                close_file(fd);

                return;
            }
            if (req->type == FILEOFF) {
                printf("Il file %s non è presente nel server.\n", token);
                return;
            }
        }

    }

}