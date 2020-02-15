#include "../header.h"

void get_command_handler(int sockfd, struct sockaddr_in servaddr,
        char *filename, char *path)
{
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

    //cerco il file nella lista FILES
    if (check_file(filename, list_dir(path))) { //file presente
        req->type = FILEON;

        if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
                (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            free_allocation(req);
            perror("Errore in sendto: invio del pacchetto request_t");
            exit(EXIT_FAILURE);
        }
        char *tmp = obtain_path(NULL, NULL, 1);
        strncat(tmp, filename, strlen(filename));
        int fd = open_file(tmp, O_RDONLY);
        printf("prima sendfile\n");
        send_file(sockfd, (struct sockaddr *) &servaddr, fd);
        printf("fine sendfile\n");
        close_file(fd);
    } else { //file non presente
        req->type = FILEOFF;

        if (sendto(sockfd, (void *)req, sizeof(request_t), 0,
                (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            free_allocation(req);
            perror("Errore in sendto: invio del pacchetto request_t");
            exit(EXIT_FAILURE);
        }
    }
}
