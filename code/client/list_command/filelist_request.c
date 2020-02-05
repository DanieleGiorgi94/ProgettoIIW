#include "../header.h"

void list_command_handler(char *cmd, char *token, client_info *c_info)
{
    struct sockaddr_in servaddr = c_info->servaddr;

    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    u32 slen = sizeof(struct sockaddr);

    printf("Socket created\n");

    //three-way handshake using this new socket
    if (create_connection(cmd, token, c_info)) {
        printf("connessione stabilita\n");
        request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

        return;
        send_request(cmd, token, c_info);

        while (recvfrom(c_info->new_sockfd, (void *) req, sizeof(request_t),
                MSG_DONTWAIT, (struct sockaddr *) &servaddr, &slen) < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom() (ricezione del pacchetto request_t)");
                exit(EXIT_FAILURE);
            }
        }

        //Se la lista dei file non è vuota
        if (strlen(req->payload)) {
            printf("--- FILE PRESENTI NEL SERVER: --- \n\n");
            printf("%s\n", (req->payload));
            printf("---------------------------------\n");
        }
        else printf("Nessun file presente nel server.\n");

        return;
    }
}
