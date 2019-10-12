#include "header.h"

int main(void)
{
    printf("Server!\n");

    int r = send_file_to_client();
    print_error("send_file_to_client:", &r);
    r = send_filelist_to_client();
    print_error("send_filelist_to_client:", &r);
    r = receive_file_from_client();
    print_error("receive_file_from_client:", &r);

    return EXIT_SUCCESS;
}
