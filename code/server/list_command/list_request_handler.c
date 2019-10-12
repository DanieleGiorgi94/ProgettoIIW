#include "../header.h"

/*
    Invia il filelist ad un client
*/
int send_filelist_to_client(void)
{
    printf("send_filelist_to_client function\n");
    send_pkt();
    return 1;
}
