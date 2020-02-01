#include "../header.h"

void put_command_handler(char *cmd, char *token, int sockfd,
                         struct sockaddr_in servaddr) {
    if (token == NULL) {
        fprintf(stderr, "Usage: put <filename.format>\n");
    } else {

        char *path = obtain_path(cmd, token, 0);
        request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
        u32 slen = sizeof(struct sockaddr);


        if (create_connection(sockfd, servaddr, cmd, token) == 1) {

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

                printf("Il file è già presente nel server.\n");
                //TODO Mettere possibilità di inserire una copia di un
                // file esistente (Es: prova(1).txt

                return;
            }
            if (req->type == FILEOFF) {

                /* Inizio invio file */
                int fd = open_file(path, O_RDONLY);
                send_file(sockfd, (struct sockaddr *) &servaddr, fd);
                close_file(fd);

                return;
            }
        }

    }
}

