#include "../header.h"

void list_command_handler(char *cmd, int sockfd, struct sockaddr_in servaddr) {
    if (create_connection(sockfd, servaddr, cmd) == 0) {
        return;
    }
}
