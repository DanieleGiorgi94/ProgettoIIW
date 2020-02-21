#include "../header.h"

void list_command_handler(int sockfd, struct sockaddr_in cliaddr, char *path)
{

    int fd = open_file("filelist", O_WRONLY | O_CREAT | O_TRUNC);
    char *list = list_dir(path);
    u32 len_list = strlen(list);
    write_block(fd, list, len_list);
    close_file(fd);
    fd = open_file("filelist", O_RDONLY);
    send_file(sockfd, (struct sockaddr *) &cliaddr, fd);
    close_file(fd);
    free(list);
}
