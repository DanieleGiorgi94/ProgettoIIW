#include "../header.h"

void *list_command_handler(void *arg)
{
    struct service_thread *ptd = (struct service_thread *) arg;

    char cmd[MAX_CMD_LENGTH];
    char token[BUFLEN - MAX_CMD_LENGTH];
    client_info *c_info = ptd->c_info;
    struct sockaddr_in servaddr = c_info->servaddr;
//    u32 slen = sizeof(struct sockaddr);

    strncpy(cmd, ptd->cmd, MAX_CMD_LENGTH);
    strncpy(token, ptd->token, BUFLEN - MAX_CMD_LENGTH);

    //printf("%s %s\n", cmd, token);

    //three-way handshake using this new socket
    if (create_connection(c_info)) {
        c_info->connected = 1;
        //printf("connessione stabilita\n");
        request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

        send_request(cmd, token, c_info);
//        while (recvfrom(c_info->new_sockfd, (void *) req, sizeof(request_t),
//                MSG_DONTWAIT, (struct sockaddr *) &servaddr, &slen) < 0) {
//            if (errno != EAGAIN && errno != EWOULDBLOCK) {
//                perror("recvfrom() (ricezione del pacchetto request_t)");
//                exit(EXIT_FAILURE);
//            }
//        }
        int fd = open_file("filelist", O_WRONLY | O_CREAT | O_TRUNC);

        /* Inizio ricezione file */
        if (TEST == 1) {
            int get_fd = open_file("log", O_WRONLY | O_CREAT);
            lseek(get_fd, 0, SEEK_END);

            struct timeval stop, start;
            gettimeofday(&start, NULL);
            receive_file(c_info->new_sockfd, (struct sockaddr *) &servaddr,
                         fd);

            gettimeofday(&stop, NULL);
            if (dup2(get_fd, STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            printf("%lu\n", (stop.tv_sec - start.tv_sec) *
                            1000000 + stop.tv_usec - start.tv_usec);
            if (dup2(STDOUT_FILENO, get_fd) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }

            close(fd);
        }else{

            receive_file(c_info->new_sockfd, (struct sockaddr *) &servaddr,
                         fd);
            close(fd);

            fd = open_file("filelist", O_RDONLY);

            printf("\n");
            printf("--- FILE PRESENTI NEL SERVER: --- \n\n");
            char s;
            while (read_block(fd, &s, sizeof(char) > 0)) {
                printf("%c", s);
            }
            printf("---------------------------------\n");
            printf(">> ");
        }


//        //Se la lista dei file non è vuota
//        if (strlen(req->payload)) {
//            printf("\n");
//            printf("--- FILE PRESENTI NEL SERVER: --- \n\n");
//            printf("%s\n", (req->payload));
//            printf("---------------------------------\n");
//            printf(">> ");
//        } else {
//            printf("\n");
//            printf("Nessun file presente nel server.\n");
//            printf(">> ");
//        }
        fflush(stdout);

        close(fd);
        free(req);
//        close_file(c_info->new_sockfd);
    } else printf("Connessione fallita\n");
    return NULL;
}
