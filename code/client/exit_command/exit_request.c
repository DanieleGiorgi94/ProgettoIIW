#include "../header.h"

void exit_command_handler(char *cmd, int sockfd, struct sockaddr_in servaddr) {

    if ( terminate_connection(cmd, sockfd, servaddr) ){

        printf("Disconnected from the server succesfully.\n");
    }
    printf("Unexpected error while disconnecting from server.\n");
}