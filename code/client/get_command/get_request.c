#include "../header.h"

void get_command_handler(char *cmd, char *token, int sockfd,
                         struct sockaddr_in servaddr) {
    if (token == NULL) {
        fprintf(stderr, "Usage: get <filename.format>\n");
    } else {

        //Manda al server il pacchetto con il nome del file da cercare
        request_t *rqst = (request_t *) dynamic_allocation(sizeof(request_t));
        syn_t *syn = (syn_t *) dynamic_allocation(sizeof(syn_t));
        rqst->type = GET_REQ;
        strncpy(rqst->filename, token, BUFLEN);


        u32 slen = sizeof(struct sockaddr);


        if (sendto(sockfd, rqst, sizeof(request_t), 0,
                   (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            perror("errore in sendto");
            exit(EXIT_FAILURE);
        }

        while (recvfrom(sockfd, (void *) syn, sizeof(syn_t), MSG_DONTWAIT,
                        (struct sockaddr *) &servaddr, &slen) < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom() (ricezione dell'ack)");
                exit(EXIT_FAILURE);
            }
        }

        if (syn->flag == FILEON){

            if (create_connection(sockfd, servaddr) == 1) {
                int fd = open_file(token, O_WRONLY|O_CREAT);
                receive_file(sockfd, (struct sockaddr *) &servaddr, fd);
                close_file(fd);
            }
        }
        if (syn->flag == FILEOFF){
            printf("Il file %s non è presente nel server.\n", token);

        }

    }

}
