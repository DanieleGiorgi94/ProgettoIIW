#include "../header.h"

void list_command_handler(char *cmd, char *token, int sockfd, struct sockaddr_in servaddr,
        char *conn, u64 *server_isn) {

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    u32 slen = sizeof(struct sockaddr);

    if (create_connection(sockfd, servaddr, cmd, token, conn, server_isn)) {

        printf("Waiting for file list from server...\n");
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

        //Se la lista dei file non è vuota
        if (strlen(req->payload)){
            printf("--- FILE PRESENTI NEL SERVER: --- \n\n");

                printf("%s\n", (req->payload));

            printf("---------------------------------\n");
        }
        else printf("Nessun file presente nel server.\n");

        return;
    }
}
