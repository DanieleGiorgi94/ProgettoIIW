#include "header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFLEN 512

int main(int argc, char **argv)
{
    struct sockaddr_in send_servaddr, rcv_servaddr; //IPv4 address
    int send_sockfd, rcv_sockfd, n;

    if (argc < 2 ) { /* controlla numero degli argomenti */
        fprintf(stderr, "utilizzo: ./client <indirizzo IP server>\n");
        exit(EXIT_FAILURE);
    }

    if ((send_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("errore in socket");
        exit(EXIT_FAILURE);
    }
    if ((rcv_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("errore in socket");
        exit(EXIT_FAILURE);
    }


    /* Socket struct initialization */
    memset((char *) &send_servaddr, 0, sizeof(send_servaddr));
    send_servaddr.sin_family = AF_INET; //assegna tipo di indirizzo IPv4
    send_servaddr.sin_port = htons(SEND_PORT); //assegna porta server
    u64 slen = sizeof(send_servaddr);
    if (inet_pton(AF_INET, argv[1], &send_servaddr.sin_addr) <= 0) {
        fprintf(stderr, "errore in inet_pton per %s", argv[1]);
        exit(EXIT_FAILURE);
    }
    memset((char *) &rcv_servaddr, 0, sizeof(rcv_servaddr));
    rcv_servaddr.sin_family = AF_INET; //assegna tipo di indirizzo IPv4
    rcv_servaddr.sin_port = htons(RCV_PORT); //assegna porta server
    slen = sizeof(rcv_servaddr);
    if (inet_pton(AF_INET, argv[1], &rcv_servaddr.sin_addr) <= 0) {
        fprintf(stderr, "errore in inet_pton per %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    char *filepath = "/home/frank/Desktop/magistrale/iiw/progetto/ProgettoIIW/code/client/FILES/divina_commedia.txt";
    int fd = open_file(filepath, O_RDONLY);
    send_file(send_sockfd, rcv_sockfd, (struct sockaddr *) &send_servaddr,
        (struct sockaddr *) &rcv_servaddr, fd, "divina_commedia.txt");
    close_file(fd);

    return EXIT_SUCCESS;
}
