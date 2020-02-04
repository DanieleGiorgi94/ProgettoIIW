#include "../header.h"

void get_command_handler(char *cmd, char *token, int sockfd,
                         struct sockaddr_in servaddr, char *conn, u64 *server_isn) {
    if (token == NULL) {
        fprintf(stderr, "Usage: get <filename.format>\n");
    } else {

        //Manda al server il pacchetto con il nome del file da cercare
        char *path = obtain_path(cmd, token, 0);
        request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

        u32 slen = sizeof(struct sockaddr);

        if (create_connection(sockfd, servaddr, cmd, token, conn, server_isn)) {

            /* Threeway handshake completed */

            printf("Waiting for FILEON from server.\n");

            clock_t tspan;
            tspan = clock();

            while (recvfrom(sockfd, (void *) req, sizeof(request_t), MSG_DONTWAIT,
                            (struct sockaddr *) &servaddr, &slen) < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("recvfrom() (ricezione del pacchetto request_t)");
                    exit(EXIT_FAILURE);
                }
                if (clock() - tspan > 1000){
                    req->ACK = *server_isn; // server_isn+1
                    send_request(cmd, token, req, sockfd, servaddr);
                    tspan = clock();
                }
            }

            if (req->type == FILEON) {

                printf("Il file è presente\n");

                int fd = open_file(path, O_WRONLY | O_CREAT);
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