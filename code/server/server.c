#include "header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    /* printf("Server!\n");

     int r = send_file_to_client();
     print_error("send_file_to_client:", &r);
     r = send_filelist_to_client();
     print_error("send_filelist_to_client:", &r);
     r = receive_file_from_client();
     print_error("receive_file_from_client:", &r);

     return EXIT_SUCCESS;*/

    int send_sockfd;
    int rcv_sockfd;
    socklen_t len;
    struct sockaddr_in send_addr;
    struct sockaddr_in rcv_addr;


    if ((send_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("errore in socket");
        exit(EXIT_FAILURE);
    }
    if ((rcv_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("errore in socket");
        exit(EXIT_FAILURE);
    }

    memset((void *) &send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    send_addr.sin_port = htons(SEND_PORT); /* numero di porta del server */
    if (bind(send_sockfd, (struct sockaddr *) &send_addr,
                                    sizeof(send_addr)) < 0) {
        perror("errore in bind");
        exit(EXIT_FAILURE);
    }
    memset((void *) &rcv_addr, 0, sizeof(rcv_addr));
    rcv_addr.sin_family = AF_INET;
    rcv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rcv_addr.sin_port = htons(RCV_PORT); /* numero di porta del server */
    if (bind(rcv_sockfd, (struct sockaddr *) &rcv_addr,
                                    sizeof(rcv_addr)) < 0) {
        perror("errore in bind");
        exit(EXIT_FAILURE);
    }

    int fd = open_file("/home/frank/Desktop/magistrale/iiw/progetto/ProgettoIIW/code/server/FILES/prova.txt", O_WRONLY | O_CREAT);//file in scrittura pkt ricevuti da cb

    receive_file(send_sockfd, rcv_sockfd, (struct sockaddr *) &send_addr,
                            (struct sockaddr *) &rcv_addr, fd);

    /*while(1){
        usleep_for(100000);
    }*/

    exit(EXIT_SUCCESS);
}
