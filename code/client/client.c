#include "header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT   5193
#define BUFLEN 512


char ** tokenize_string(char *buffer,char *delimiter){

    int i = 0;
    char **token_vector = dynamic_allocation(BUFLEN * sizeof(char*));

    token_vector[i] = strtok(buffer,delimiter);
    while(token_vector[i]!= NULL) {
        i++;
        token_vector[i] = strtok(NULL,delimiter);
    }

    return token_vector;
}

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

    char message[BUFLEN];

    char **token_vector;

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

    printf("Welcome to our server! IP: %s PORT: %d\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
    RESET:
    printf("\n+----------------------------------------------------------------------------------+\n|"
           "ELENCO  COMANDI:""                                                                  |");
    printf("\n+----------------------------------------------------------------------------------+\n"
           "| 1) list: elenco dei file presenti nel Server                                     |\n"
           "| 2) get <Filename>: Download del file                                             |\n"
           "| 3) put <Filename>: Upload del file                                               |\n"
           "| 4) exit                                                                          |\n"
           "+----------------------------------------------------------------------------------+");

    printf("\nENTER MESSAGE:\n>> ");

    if (fgets(message, BUFLEN, stdin) == NULL && errno != EINTR) {
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    int len = (int) strlen(message)-1;


    if (message[len] == '\n') {
        message[len] = '\0';
    }

    token_vector = tokenize_string(message, " ");


    if (strncmp(token_vector[0], "list", 5) == 0) {

        printf("Received LIST command...\n");
        goto RESET;
    }

    if (strncmp(token_vector[0], "get", 4) == 0) {

        printf("Received GET command...\n");
        goto RESET;
    }

    if (strncmp(token_vector[0], "put", 4) == 0) {

        printf("Received GET command...\n");

        int fd = open_file(PATH, O_RDONLY);

        send_file(sockfd, (struct sockaddr *) &servaddr, fd); //invia file al buffer circolare

        close_file(fd);

        goto RESET;
    }

    if (strncmp(token_vector[0], "exit", 5) == 0) {

        printf("Processing exit from server...\n");
        usleep_for(1000000);
        printf("Bye\n");

        exit(EXIT_SUCCESS);
    }

    if (strncmp(token_vector[0], "list", 5) != 0 && strncmp(token_vector[0], "get", 4) != 0 &&
        strncmp(token_vector[0], "exit", 5) && strncmp(token_vector[0], "put", 4) != 0) {

        fprintf(stderr, "Please insert a valid command.\n");
        goto RESET;
    }


    return EXIT_SUCCESS;
}