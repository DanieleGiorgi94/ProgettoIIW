#include "../header.h"

/*
    Funzione che richiede un file al server ed attende sua risposta
*/
int request_file(void)
{
    printf("request_file function\n");
    send_pkt();
    return 1;
}
