#include "../header.h"

/*void put_command_handler(int sockfd, struct sockaddr_in servaddr,
                                                char *filename, char *path) {
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

    //cerco se il file nella lista FILES del server c'è già
    if (check_file(filename, list_dir(path))){ //file presente
        req->type = FILEON;
        if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
                (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            free_allocation(req);
            perror("Errore in sendto: invio del pacchetto twh_request_t");
            exit(EXIT_FAILURE);
        }
    } else { //file non presente
        req->type = FILEOFF;

        if (sendto(sockfd, (void *)req, sizeof(request_t), 0,
                (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            free_allocation(req);
            perror("Errore in sendto: invio del pacchetto twh_request_t");
            exit(EXIT_FAILURE);
        }
        int fd = open_file(strncat(path, filename, strlen(filename)),
                                                        O_WRONLY | O_CREAT);
       */ /* Inizio ricezione file */
//        receive_file(sockfd, (struct sockaddr *) &servaddr, fd);
//        close_file(fd);
//    }
//}
