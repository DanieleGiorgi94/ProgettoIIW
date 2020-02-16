#include "../header.h"

void list_command_handler(int sockfd, struct sockaddr_in cliaddr, char *path)
{
//    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
//    srand(time(NULL) + getpid());
//    u64 server_isn = rand() % 100;
//
//    strncpy(req->payload, list_dir(path), sizeof((req->payload)));
//
//    req->initial_n_seq = server_isn;
//
//    printf("Invio lista dei miei file a client.\n");
//    if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
//            (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0) {
//        free_allocation(req);
//        perror("Errore in sendto: invio del pacchetto twh_request_t");
//        exit(EXIT_FAILURE);
//    }
    int fd = open_file("filelist", O_WRONLY | O_CREAT | O_TRUNC);
    char *list = list_dir(path);
    u32 len_list = strlen(list);
    write_block(fd, list, len_list);
    close_file(fd);
    fd = open_file("filelist", O_RDONLY);
    send_file(sockfd, (struct sockaddr *) &cliaddr, fd);
    close_file(fd);
    free(list);
}
