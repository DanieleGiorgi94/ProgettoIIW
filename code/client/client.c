#include "header.h"

int main(void)
{
    printf("Client\n");

    int r = request_filelist();
    print_error("request_filelist:", &r);
    r = request_file();
    print_error("request_file:", &r);
    r = send_file_to_server();
    print_error("send_file_to_server:", &r);

    char *list = list_dir(PATH);

    printf("\nLista dei file presenti: \n%s", list);
    free(list);

    return EXIT_SUCCESS;
}
