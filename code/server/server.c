#include "header.h"

static void main_task(int, struct sockaddr_in);
static void create_service_thread(server_info *);
static int select_available_port(struct available_ports *);
static void set_port_numbers(int *);

static void main_task(int sockfd, struct sockaddr_in servaddr)
{
    char *no_connections = dynamic_allocation(sizeof(*no_connections));
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    struct available_ports *ports =
        dynamic_allocation(sizeof(struct available_ports));
    server_info *srv_info = (server_info *)
        dynamic_allocation(sizeof(server_info));

    char *path = obtain_path(NULL, NULL, 1);
    u32 slen = sizeof(struct sockaddr);
    struct sockaddr_in cliaddr;

    *no_connections = 0;

    srv_info->sockfd = sockfd;
    srv_info->path = path;
    srv_info->no_connections = no_connections;
    srv_info->ports = ports;
    set_port_numbers(ports->port_numbers);

RESET:

    //printf("Waiting SYN...\n");
    while (recvfrom(sockfd, (void *) req, sizeof(request_t), 0,
                                (struct sockaddr *) &servaddr, &slen) < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() failed");
            free(no_connections);
            free(req);
            free(path);
            return;
        }
    }
    srv_info->servaddr = servaddr;

    //three-way handshake starts with SYN!
    if (req->SYN != 1 || req->ACK != 0 || req->FIN != 0) {
        //printf("Was not a SYN...\n");
        usleep_for(1);
        goto RESET;
    }
    //printf("It's a SYN request!\n");

    if (*no_connections < MAX_CONNECTIONS) {
        //creating new socket for the new client
        int index = select_available_port(ports);
        u64 new_port_number = ports->port_numbers[index];
        int new_sockfd;

        for (int j = 0; j < MAX_CONNECTIONS; j++) {
            //printf("%d ", ports->available[j]);
        }
        //printf("\n");
        //printf("%d\n", index);
        
        create_new_socket(&new_sockfd, &cliaddr, new_port_number);
        printf("New sockfd created, port=%lu\n", new_port_number);

        //creating new thread
        srv_info->client_isn = req->initial_n_seq;
        srv_info->cliaddr = cliaddr;
        srv_info->new_sockfd = new_sockfd;
        srv_info->port_number = new_port_number;
        create_service_thread(srv_info);

        *no_connections += 1;
        ports->available[index] = 1;
    } else {
        printf("Server is now full.\n");
        goto RESET;
    }
    printf("Connection n. %d created!\n", *no_connections);
    goto RESET;
}
static void create_service_thread(server_info *srv_info)
{
    struct service_thread s_thread;

    s_thread.srv_info = srv_info;
    if (pthread_create(&s_thread.tid, NULL, create_connection,
                                                &s_thread) != 0) {
        perror("pthread_create() failed");
    }
}
static void set_port_numbers(int *port_numbers)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        port_numbers[i] = PORT + i + 1;
    }
}
static int select_available_port(struct available_ports *ports)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (ports->available[i] == 0)
            return i;
    }
    return -1;
}
int main()
{
    struct sockaddr_in servaddr;
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    memset((void *) &servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("errore in bind");
        exit(EXIT_FAILURE);
    }

    main_task(sockfd, servaddr);
    close(sockfd);

    exit(EXIT_SUCCESS);
}
