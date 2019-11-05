#include "header.h"

int main(int argc, char **argv)
{
    /*int r = request_filelist();
    print_error("request_filelist:", &r);
    r = request_file();
    print_error("request_file:", &r);
    r = send_file_to_server();
    print_error("send_file_to_server:", &r);

    char *list = list_dir(PATH);

    printf("\nList of files:\n%s", list);

    if (check_file("bibbia.txt",list))
        printf("This file is present on the server\n");
    else printf("This file is NOT present on the server\n");

    free(list);
    */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

    struct sockaddr_in servaddr; //IPv4 address
    int   sockfd, n;

    #define PORT   5193


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
    int fd = open_file(PATH, O_RDONLY);

    send_file(sockfd, (struct sockaddr *) &servaddr, fd); //invia file al buffer circolare

    close_file(fd);

    return EXIT_SUCCESS;
}