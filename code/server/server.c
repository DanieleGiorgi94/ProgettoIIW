#include "header.h"

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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

    int sockfd;
    socklen_t len;
    struct sockaddr_in addr;
    time_t ticks;

    #define PORT   5193


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { /* crea il socket */
        perror("errore in socket");
        exit(1);
    }

    memset((void *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); /* il server accetta pacchetti su una qualunque delle sue interfacce di rete */
    addr.sin_port = htons(PORT); /* numero di porta del server */

    /* assegna l'indirizzo al socket */
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("errore in bind");
        exit(1);
    }

    while(1){
        usleep_for(100000);
    }

    exit(EXIT_SUCCESS);

}
