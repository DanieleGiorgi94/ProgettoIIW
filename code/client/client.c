#include "header.h"

void print_banner(char *, u32);
static void main_task(client_info *);
char **tokenize_string(char *, char *);

static void main_task(client_info *c_info) {
    char command[BUFLEN];
    char **token_vector;
    char *cmd, *token;

    RESET:
    printf(">> ");

    if (fgets(command, BUFLEN, stdin) == NULL && errno != EINTR) {
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    int len = (int) strlen(command) - 1;
    if (len < 3){
        fprintf(stderr, "Please insert a valid command.\n");
        goto RESET;
    }

    if (command[len] == '\n') {
        command[len] = '\0';
    }

    token_vector = tokenize_string(command, " ");
    cmd = token_vector[0];
    token = token_vector[1];

    if (strncmp(cmd, "list", 5) == 0) {
        list_command_handler(cmd, token, c_info);
        goto RESET;
    }

//    if (strncmp(cmd, "get", 4) == 0) {
//        get_command_handler(cmd, token, sockfd, servaddr, connected, server_isn);
//        goto RESET;
//    }
//
//    if (strncmp(cmd, "put", 4) == 0) {
//        put_command_handler(cmd, token, sockfd, servaddr, connected, server_isn);
//        goto RESET;
//    }

    if (strncmp(cmd, "exit", 5) == 0) {
        printf("Processing exit from server...\n");
        exit_command_handler(c_info); //gestione terminazione connessione
    }

    if (strncmp(cmd, "list", 5) != 0 && strncmp(cmd, "get", 4) != 0 &&
            strncmp(cmd, "exit", 5) && strncmp(cmd, "put", 4) != 0) {
        fprintf(stderr, "Please insert a valid command.\n");
        goto RESET;
    }
}
char **tokenize_string(char *buffer, char *delimiter) {
    int i = 0;
    char **token_vector = dynamic_allocation(BUFLEN * sizeof(char *));

    token_vector[i] = strtok(buffer, delimiter);
    while(token_vector[i]!= NULL) {
        i++;
        token_vector[i] = strtok(NULL, delimiter);
    }

    return token_vector;
}
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
    client_info *c_info = dynamic_allocation(sizeof(client_info));

    if (argc < 2 ) { /* controlla numero degli argomenti */
        fprintf(stderr, "utilizzo: ./client <indirizzo IP server>\n");
        exit(EXIT_FAILURE);
    }

    /* Creazione prima socket per handshake */
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

    c_info->sockfd = sockfd;
    c_info->servaddr = servaddr;
    c_info->argv = argv[1];

    if (TEST == 0) {
        print_banner(inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
        main_task(c_info);
    } else {
        //put_command_handler("put", "divina_commedia.txt", sockfd, servaddr);
    }

    return EXIT_SUCCESS;
}
