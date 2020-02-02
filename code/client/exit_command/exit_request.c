#include "../header.h"

void exit_command_handler(int sockfd, struct sockaddr_in servaddr) {

    if ( close_connection(sockfd, servaddr) ){

        close_file(sockfd);
        printf("Disconnected from the server succesfully.\n");
        exit(EXIT_SUCCESS);
    }
    printf("Unexpected error while disconnecting from server.\n");
    exit(EXIT_FAILURE);
    //TODO Invece dovremmo ritrasmettere teoricamente gestendo un timeout
}