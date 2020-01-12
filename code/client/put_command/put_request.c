#include "../header.h"

static char *obtain_path(char *, char *);

void put_command_handler(char *cmd, char *token, int sockfd,
        struct sockaddr_in servaddr) {
    if (token == NULL){
        fprintf(stderr, "Usage: put <filename.format>\n");
    } else {
        char *filepath = obtain_path(cmd, token);
        int fd = open_file(filepath, O_RDONLY);

        send_file(sockfd, (struct sockaddr *) &servaddr, fd);
        close_file(fd);
    }
}
static char *obtain_path(char *cmd, char *token) {
    char *cwd = dynamic_allocation(sizeof(char *) * BUFLEN);
    char *download_dir = "/FILES/";
    char *upload_dir = "/FILES/";

    int len_cwd;
    int len_download_dir = strlen(download_dir);
    int len_upload_dir = strlen(upload_dir);

    if (getcwd(cwd, sizeof(char) * BUFLEN) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    len_cwd = strlen(cwd);

    if (strncmp(cmd, "get", 4) == 0)
        strncat(cwd, download_dir, len_cwd + len_download_dir);
    else if (strncmp(cmd, "put", 4) == 0)
        strncat(cwd, upload_dir, len_cwd + len_upload_dir);
    strncat(cwd, token, strlen(token));
    printf("Current path -> %s\n", cwd);

    return cwd;
}
