#include "../header.h"

char *list_dir(char *PATH)
{
    DIR *d;
    struct dirent *dir;
    char *buff = dynamic_allocation(sizeof(char) * 100);
    int v = 0;

    d = opendir(PATH);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strstr(".", dir->d_name) == NULL &&
                strstr("..", dir->d_name) == NULL) //elimina le due root dir
                v += sprintf(buff+v, "%s\n", dir->d_name);
        }
        closedir(d);
    } else {
        /* could not open directory */
        perror ("Error in opendir");
        exit(EXIT_FAILURE);
    }

    return buff;
}
int check_file(char *file, char *list)
{
    if (strstr(list, file) == NULL)
        return 0;
    else
        return 1;
}
void create_new_socket(int *new_sockfd, struct sockaddr_in *cliaddr,
        int new_port_number)
{
    if ((*new_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    //printf("nseq: %lu\n", req->initial_n_seq);
    memset((void *) cliaddr, 0, sizeof(*cliaddr));
    cliaddr->sin_family = AF_INET;
    cliaddr->sin_addr.s_addr = htonl(INADDR_ANY);
    cliaddr->sin_port = htons(new_port_number);
    if (bind(*new_sockfd, (struct sockaddr *) cliaddr,
                                sizeof(*cliaddr)) < 0) {
        perror("errore in bind");
        exit(EXIT_FAILURE);
    }
}
