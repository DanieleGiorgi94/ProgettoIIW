#include "../header.h"

void exit_command_handler(client_info *c_info) {
    int sockfd = c_info->sockfd;
    int new_sockfd = c_info->new_sockfd;
    struct sockaddr_in servaddr = c_info->servaddr;

    if (close_connection(sockfd, servaddr) ){
        close_file(sockfd);
        close_file(new_sockfd);
        printf("Disconnected from the server successfully.\n");
        exit(EXIT_SUCCESS);
    }
    printf("Unexpected error while disconnecting from server.\n");
    close_file(sockfd);
    close_file(new_sockfd);
    exit(EXIT_FAILURE);
    //TODO Invece dovremmo ritrasmettere teoricamente gestendo un timeout
}
