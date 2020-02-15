#include "../header.h"

void *get_command_handler(void *arg)
{
    struct service_thread *ptd = (struct service_thread *) arg;

    char *cmd = ptd->cmd;
    char *token = ptd->token;
    client_info *c_info = ptd->c_info;
    struct sockaddr_in servaddr = c_info->servaddr;
    u32 slen = sizeof(struct sockaddr);

    if (token == NULL) {
        fprintf(stderr, "Usage: get <filename.format>\n");
    } else {
        //Manda al server il pacchetto con il nome del file da cercare
        char *path = obtain_path(cmd, token, 0);
        request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

        if (create_connection(c_info)) {
            /* Threeway handshake completed */
            //printf("Connessione stabilita\n");

            send_request(cmd, token, c_info);
            while (recvfrom(c_info->new_sockfd, (void *) req, sizeof(request_t),
                    MSG_DONTWAIT, (struct sockaddr *) &servaddr, &slen) < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("recvfrom() failed");
                    exit(EXIT_FAILURE);
                }
            }
  
            if (req->type == FILEON) {
                printf("\n");
                printf("Il file è presente\n");
                printf(">> ");
                fflush(stdout);

                int fd = open_file(path, O_WRONLY | O_CREAT);
                
                /* Inizio ricezione file */
                if (TEST == 1) {
                    int get_fd = open_file("log", O_WRONLY | O_CREAT);
                    lseek(get_fd, 0, SEEK_END);

                    struct timeval stop, start;
                    gettimeofday(&start, NULL);
                    receive_file(c_info->new_sockfd, (struct sockaddr *)
                            &servaddr, fd);
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
                    close_file(get_fd);
                } else {
                    receive_file(c_info->new_sockfd, (struct sockaddr *)
                            &servaddr, fd);
                    printf("\n");
                    printf("File %s correttamente ricevuto\n", token);
                    printf(">> ");
                }
                close_file(fd);
                free(req);
                close_file(c_info->new_sockfd);
            } else if (req->type == FILEOFF) {
                printf("\n");
                printf("Il file %s non è presente nel server.\n", token);
                printf(">> ");
                free(req);
            }
            fflush(stdout);
        }
    }
    return NULL;
}
