#include "../header.h"

void get_command_handler(int sockfd, struct sockaddr_in servaddr,
        char *filename, char *path)
{
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));

    //cerco il file nella lista FILES
    if (check_file(filename, list_dir(path))) { //file presente
        req->type = FILEON;

        if (sendto(sockfd, (void *) req, sizeof(request_t), 0,
                (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            free_allocation(req);
            perror("Errore in sendto: invio del pacchetto request_t");
            exit(EXIT_FAILURE);
        }
        char *tmp = obtain_path(NULL, NULL, 1);
        strncat(tmp, filename, strlen(filename));
        int fd = open_file(tmp, O_RDONLY);
        send_file(sockfd, (struct sockaddr *) &servaddr, fd);
        close_file(fd);
    } else { //file non presente
        req->type = FILEOFF;

        if (sendto(sockfd, (void *)req, sizeof(request_t), 0,
                (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            free_allocation(req);
            perror("Errore in sendto: invio del pacchetto request_t");
            exit(EXIT_FAILURE);
        }
    }
}
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
