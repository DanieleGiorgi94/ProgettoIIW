#include "../header.h"

void get_command_handler(int sockfd, struct sockaddr_in servaddr,
                         char *filename, char *path) {


    syn_t *syn = (syn_t *) dynamic_allocation(sizeof(syn_t));

    //cerco il file nella lista FILES

    if (check_file(filename, list_dir(path))){ //file presente

        syn->flag = FILEON;
        if (sendto(sockfd, (void *)syn, sizeof(syn_t), 0, (struct sockaddr *) &servaddr,
                   sizeof(servaddr)) < 0) {
            free_allocation(syn);
            perror("Errore in sendto: invio dell'ack");
            exit(EXIT_FAILURE);
        }
        int fd = open_file(strncat(path, filename, strlen(filename)), O_RDONLY);
        send_file(sockfd, (struct sockaddr *) &servaddr, fd);
        close_file(fd);
    }
    else{ //file non presente

        syn->flag = FILEOFF;

        if (sendto(sockfd, (void *)syn, sizeof(syn_t), 0, (struct sockaddr *) &servaddr,
                   sizeof(servaddr)) < 0) {
            free_allocation(syn);
            perror("Errore in sendto: invio dell'ack");
            exit(EXIT_FAILURE);
        }
    }
}

char *list_dir(char *PATH) {
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
int check_file(char *file, char *list) {

    if (strstr(list, file) == NULL)
        return 0;
    else
        return 1;
}

