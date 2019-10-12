#include "../header.h"

/*
    Funzione che richiede la filelist al server ed attende risposta dal server
*/
int request_filelist(void)
{
    printf("request_filelist function\n");
    send_pkt();
    return 1;
}



