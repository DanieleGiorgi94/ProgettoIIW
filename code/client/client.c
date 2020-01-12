#include "header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFLEN 512

void print_banner(char *, u32);
void main_task(void);

void print_banner(char *ip, u32 port) {
    printf(WELCOME_STRING, ip, port);
    printf("\n"SPACER);
    printf(FIRST_LINE);
    printf("\n"SPACER);
    printf(LIST_LINE);
    printf(GET_LINE);
    printf(PUT_LINE);
    printf(EXIT_LINE);
    printf(SPACER);
}
int main(int argc, char **argv) {
    struct sockaddr_in servaddr; //IPv4 address
    int sockfd;

    if (argc < 2 ) { /* controlla numero degli argomenti */
        fprintf(stderr, "utilizzo: ./client <indirizzo IP server>\n");
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    memset((void *) &servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "errore in inet_pton per %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    print_banner(inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

    char *filepath = "/home/frank/Desktop/magistrale/iiw/progetto/ProgettoIIW/code/client/FILES/divina_commedia.txt";
    //char *filepath = "/Users/Daniele-Giorgi/CLionProjects/ProgettoIIW/code/client/FILES/divina_commedia.txt";

    int fd = open_file(filepath, O_RDONLY);
    send_file(sockfd, (struct sockaddr *) &servaddr, fd);
    close_file(fd);

    return EXIT_SUCCESS;
}
