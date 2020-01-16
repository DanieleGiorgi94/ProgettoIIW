#include "../header.h"

void get_command_handler(char *cmd, char *token, int sockfd,
                                        struct sockaddr_in servaddr) {
    if (token == NULL) {
        fprintf(stderr, "Usage: get <filename.format>\n");
    } else {
        char *filepath = obtain_path(cmd, token, 0);
        if (create_connection(sockfd, servaddr) == 1) {
            //verifica esistenza file prima!
            int fd = open_file(filepath, O_WRONLY);
            receive_file(sockfd, (struct sockaddr *) &servaddr, fd);
            close_file(fd);
        }
    }
}
