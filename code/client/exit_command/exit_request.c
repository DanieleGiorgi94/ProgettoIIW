#include "../header.h"

void exit_command_handler(client_info *c_info) {
    int sockfd = c_info->sockfd;
    //struct sockaddr_in servaddr = c_info->servaddr;


    close_file(sockfd);
    printf("Disconnected from the server successfully.\n");
    exit(EXIT_SUCCESS);

    printf("Unexpected error while disconnecting from server.\n");
    close_file(sockfd);
    exit(EXIT_FAILURE);
    //TODO Invece dovremmo ritrasmettere teoricamente gestendo un timeout
}
