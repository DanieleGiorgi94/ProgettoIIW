#include "reliable_udp.h"

char *obtain_path(char *cmd, char *token, char id) {
//id == 0 ---> client,  id != 0 ---> server
    char *cwd = dynamic_allocation(sizeof(char *) * BUFLEN);
    char *download_dir;
    char *upload_dir;

    if (id == 0) {
        download_dir = "/DOWNLOAD_FILES/";
        upload_dir = "/UPLOAD_FILES/";
    } else {
        download_dir = "/FILES/";
        upload_dir = "/FILES/";
    }

    u32 len_download_dir = strlen(download_dir);
    u32 len_upload_dir = strlen(upload_dir);

    if (getcwd(cwd, sizeof(char) * BUFLEN) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    if (id == 0) {
        if (strncmp(cmd, "get", 4) == 0)
            strncat(cwd, download_dir, len_download_dir);
        else if (strncmp(cmd, "put", 4) == 0)
            strncat(cwd, upload_dir, len_upload_dir);

        if (token != NULL)
            strncat(cwd, token, strlen(token));
    } else {
        strncat(cwd, download_dir, len_download_dir);
    }
    return cwd;
}
