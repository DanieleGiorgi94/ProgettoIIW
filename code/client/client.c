#include "header.h"

static void print_banner(char *, u32);
static void main_task(client_info *);
static void create_service_thread(char *, char *, client_info *, char);
static char **tokenize_string(char *, char *);
static void fill_array(char *, char *, int);

static void main_task(client_info *c_info)
{
    char command[BUFLEN];
    char **token_vector;
    char cmd[MAX_CMD_LENGTH], token[BUFLEN - MAX_CMD_LENGTH];

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
    fill_array(cmd, token_vector[0], MAX_CMD_LENGTH);

    if (strncmp(cmd, "list", 5) == 0) {
        create_service_thread(cmd, token, c_info, LIST_REQ);
        goto RESET;
    }

    if (strncmp(cmd, "get", 4) == 0) {
        fill_array(token, token_vector[1], BUFLEN - MAX_CMD_LENGTH);
        create_service_thread(cmd, token, c_info, GET_REQ);
        goto RESET;
    }

    if (strncmp(cmd, "put", 4) == 0) {
        fill_array(token, token_vector[1], BUFLEN - MAX_CMD_LENGTH);
        create_service_thread(cmd, token, c_info, PUT_REQ);
        goto RESET;
    }

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
static void create_service_thread(char *cmd, char *token, client_info *c_info,
        char type)
{
    struct service_thread s_thread;

    //printf("creating new thread\n");

    void *(*function_pointer)(void *);
    if (type == LIST_REQ)
        function_pointer = list_command_handler;
    else if (type == GET_REQ)
        function_pointer = get_command_handler;
    else if (type == PUT_REQ)
        function_pointer = put_command_handler;
    else
        return;

    s_thread.c_info = c_info;
    strncpy(s_thread.cmd, cmd, MAX_CMD_LENGTH);
    strncpy(s_thread.token, token, BUFLEN - MAX_CMD_LENGTH);

    if (pthread_create(&s_thread.tid, NULL, function_pointer,
            &s_thread) != 0) {
        perror("pthread_create() failed");
    }
}
static void fill_array(char *out_array, char *in_array, int length)
{
    int i = 0;
    for (i = 0; i < length; i++) {
        out_array[i] = 0;
    }
    for (i = 0; i < length; i++) {
        if (in_array[i] == 0 || in_array[i] == ' ' || in_array[i] == '\n')
            break;
        out_array[i] = in_array[i];
    }
}
static char **tokenize_string(char *buffer, char *delimiter)
{
    int i = 0;
    char **token_vector = dynamic_allocation(2 * sizeof(char *));

    token_vector[i] = strtok(buffer, delimiter);
    while (token_vector[i] != NULL) {
        i++;
        token_vector[i] = strtok(NULL, delimiter);
    }

    return token_vector;
}
static void print_banner(char *ip, u32 port)
{
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
int main(int argc, char **argv)
{
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

    print_banner(inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
    main_task(c_info);

    return EXIT_SUCCESS;
}
