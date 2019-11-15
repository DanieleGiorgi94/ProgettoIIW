#include "header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT   5193
#define BUFLEN 512

int main(int argc, char **argv)
{
    char message[BUFLEN];
    char **token_vector;
    struct sockaddr_in servaddr; //IPv4 address
    int sockfd, n, len;


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
    //htons permette di tener conto automaticamente della possibile differenza
    //fra l’ordinamento usato sul computer e quello che viene utilizzato nella
    //trasmissione sulla rete
    u64 slen = sizeof(servaddr);

    /* inet_aton, unlike inet_pton, allow the more general numbers-and-dots
    notation (hexadecimal, etc) * but it does not recognize IPv6 addresses */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        /* inet_pton (p=presentation) vale anche per indirizzi IPv6 */
        fprintf(stderr, "errore in inet_pton per %s", argv[1]);
        exit(EXIT_FAILURE);
    }
    /*------------------------------*/

    /* BANNER */
    printf("Welcome to our server! IP: %s PORT: %d\n",
        inet_ntoa(servaddr.sin_addr),
        ntohs(servaddr.sin_port));
    RESET:
    print_banner();
    /*--------*/

    /* ATTESA COMANDO */
    if (fgets(message, BUFLEN, stdin) == NULL && errno != EINTR) {
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    /* PARSING COMANDO */
    len = (int) strlen(message) - 1;

    if (len < 4) {
    	fprintf(stderr, "Please insert a valid command.\n");
    	goto RESET;
    }

    if (message[len] == '\n') {
        message[len] = '\0';
    }
    token_vector = tokenize_string(message, " ");

    /* ESEGUE IL COMANDO */
    if (strncmp(token_vector[0], "list", 5) == 0) {
        int r = request_filelist();
        goto RESET;
    }

    if (strncmp(token_vector[0], "get", 4) == 0) {
        int r = request_file(servaddr, sockfd);
        goto RESET;
    }

    if (strncmp(token_vector[0], "put", 4) == 0) {
        printf("Received PUT command...\n");

        goto RESET;
    }

    if (strncmp(token_vector[0], "exit", 5) == 0) {
        printf("Processing exit from server...\n");
        usleep_for(1000000);
        printf("Bye\n");

        exit(EXIT_SUCCESS);
    }

    fprintf(stderr, "Please insert a valid command.\n");
    goto RESET;
    /*-------------------*/

    return EXIT_SUCCESS;
}

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

void print_banner(void) {
    printf("\n");
    printf(SEPARATION_LINE "\n");
    printf(FIRST_LINE "\n");
    printf(SEPARATION_LINE "\n");
    printf(LIST_LINE "\n");
    printf(GET_LINE "\n");
    printf(PUT_LINE "\n");
    printf(EXIT_LINE "\n");
    printf(SEPARATION_LINE "\n");
    printf(ENTER_LINE "\n");
}
