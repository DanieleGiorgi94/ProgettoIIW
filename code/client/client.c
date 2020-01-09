#include "header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT   5193
#define BUFLEN 512

int main(int argc, char **argv)
{
    struct sockaddr_in servaddr; //IPv4 address
    int   sockfd, n;


    if (argc < 2 ) { /* controlla numero degli argomenti */
        fprintf(stderr, "utilizzo: ./client <indirizzo IP server>\n");
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { /* crea il socket */
        perror("errore in socket");
        exit(1);
    }


    /* Socket struct initialization */
    memset((char *) &servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; //assegna tipo di indirizzo IPv4
    servaddr.sin_port = htons(PORT); //assegna porta server
    // htons permette di tener conto automaticamente della possibile differenza fra l’ordinamento usato sul computer
    // e quello che viene utilizzato nella trasmissione sulla rete
    u64 slen = sizeof(servaddr);

    /* inet_aton, unlike inet_pton, allow the more general numbers-and-dots notation (hexadecimal, etc)
     * but it does not recognize IPv6 addresses */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        /* inet_pton (p=presentation) vale anche per indirizzi IPv6 */
        fprintf(stderr, "errore in inet_pton per %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    char *filepath = "/home/frank/Desktop/magistrale/iiw/progetto/ProgettoIIW/code/client/FILES/divina_commedia.txt";
    int fd = open_file(filepath, O_RDONLY);
    send_file(sockfd, &servaddr, fd, "divina_commedia.txt");
    close_file(fd);

    return EXIT_SUCCESS;
}
