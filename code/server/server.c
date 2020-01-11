#include "header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    struct sockaddr_in servaddr;
    int sockfd;
    socklen_t len;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error");
    }

    memset((void *) &servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("errore in bind");
        exit(EXIT_FAILURE);
    }

    int fd = open_file("/home/frank/Desktop/magistrale/iiw/progetto/ProgettoIIW/code/server/FILES/prova.txt", O_WRONLY | O_CREAT);
    //int fd = open_file("/Users/Daniele-Giorgi/CLionProjects/ProgettoIIW/code/server/FILES/prova.txt", O_WRONLY | O_CREAT);


    receive_file(sockfd, (struct sockaddr *) &servaddr, fd);

    /*while(1){
        usleep_for(100000);
    }*/

    exit(EXIT_SUCCESS);
}
