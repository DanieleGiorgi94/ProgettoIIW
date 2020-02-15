#include "../header.h"

void *put_command_handler(void *arg)
{
    struct service_thread *ptd = (struct service_thread *) arg;

    char *cmd = ptd->cmd;
    char *token = ptd->token;
    client_info *c_info = ptd->c_info;
    struct sockaddr_in servaddr = c_info->servaddr;
    u32 slen = sizeof(struct sockaddr);

    if (token == NULL) {
        fprintf(stderr, "Usage: put <filename.format>\n");
    } else {
        char *path = obtain_path(cmd, token, 0);
        request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

        if (create_connection(c_info)) {
            /* Threeway handshake completed */
            //printf("Waiting for FILEON from server.\n");
            send_request(cmd, token, c_info);
            while (recvfrom(c_info->new_sockfd, (void *) req, sizeof(request_t),
                MSG_DONTWAIT, (struct sockaddr *) &servaddr, &slen) < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("recvfrom() (ricezione del pacchetto request_t)");
                    exit(EXIT_FAILURE);
                }
            }

            if (req->type == FILEON) {
                printf("\n");
                printf("Il file è già presente nel server.\n");
                //TODO Mettere possibilità di inserire una copia di un
                // file esistente (Es: prova(1).txt
            }
            else if (req->type == FILEOFF) {
                /* Inizio invio file */
                int fd = open_file(path, O_RDONLY);
                send_file(c_info->new_sockfd, (struct sockaddr *) &servaddr,
                    fd);
                close_file(fd);
                free(req);
                close_file(c_info->new_sockfd);
                printf("\n");
                printf("File %s correttamente caricato nel server!\n", token);
            }
        }
        printf(">> ");
        fflush(stdout);
    }
    return NULL;
}
