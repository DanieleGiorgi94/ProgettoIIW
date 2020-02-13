#include "../header.h"

void *list_command_handler(void *arg)
{
    struct service_thread *ptd = (struct service_thread *) arg;

    char cmd[MAX_CMD_LENGTH];
    char token[BUFLEN - MAX_CMD_LENGTH];
    client_info *c_info = ptd->c_info;
    struct sockaddr_in servaddr = c_info->servaddr;
    u32 slen = sizeof(struct sockaddr);

    strncpy(cmd, ptd->cmd, MAX_CMD_LENGTH);
    strncpy(token, ptd->token, BUFLEN - MAX_CMD_LENGTH);

    //printf("%s %s\n", cmd, token);

    //three-way handshake using this new socket
    if (create_connection(c_info)) {
        c_info->connected = 1;
        //printf("connessione stabilita\n");
        request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

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
            printf("\n");
            printf("--- FILE PRESENTI NEL SERVER: --- \n\n");
            printf("%s\n", (req->payload));
            printf("---------------------------------\n");
            printf(">> ");
        } else {
            printf("\n");
            printf("Nessun file presente nel server.\n");
            printf(">> ");
        }
        fflush(stdout);

        free(req);
        close_file(c_info->new_sockfd);
    } else printf("Connessione fallita\n");
    return NULL;
}
