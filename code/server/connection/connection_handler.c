#include "../header.h"

void *create_connection(void *arg) {
    struct service_thread *ptd = (struct service_thread *) arg;

    int sockfd = ptd->sockfd;
    struct sockaddr_in servaddr = ptd->servaddr;
    char *no_connections = ptd->no_connections;
    char *path = ptd->path;

    u32 slen = sizeof(struct sockaddr);
    request_t *req = (request_t *) dynamic_allocation(sizeof(request_t));
    syn_t *syn = (syn_t *) dynamic_allocation(sizeof(syn_t));

//il SYN qua già lo hai ricevuto: lo ricevi nel main_task e a seguito del
//SYN_ACK il thread principale crea un thread secondario che invia SYN_ACK e
//continua il three-way handshake. Nel frattempo (se vedi la main_task in
//server.c), il thread principale si mette in attesa di un altro eventuale SYN
//di qualche altro client per esempio. In più avevi commmentato il goto RESET
//alla fine di server.c e quindi, siccome il thread principale terminava, anche
//i thread figli morivano e tutto si bloccava.

//Quindi non devi metterti in attesa del SYN ma devi
//inviare direttamente il syn-ack. Non voglio mettere mano sul codice per non
//combinare casini ma sicuramente devi togliere la recvfrom che segue e quella
//condizione "if (sin->SYN >0)".

    printf("Attendo SYN\n");
    //attendi il comando del client
    while (recvfrom(sockfd, (void *) syn, sizeof(syn_t), MSG_DONTWAIT,
                                    (struct sockaddr *) &servaddr, &slen) < 0) {

        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom() failed");
            free(syn);
            return NULL;
        }
    }
    printf("syn received.\n");
    if (syn->SYN > 0){ //SYN received
        syn->initial_n_seq = 2; // ?
        syn->ACK = syn->SYN + 1;
        syn->SYN = syn->initial_n_seq;
        syn->FIN = 0;

        if (sendto(sockfd, (void *) syn, sizeof(syn_t), 0, (struct sockaddr *) &servaddr,
                   sizeof(servaddr)) < 0) {
            free_allocation(syn);
            perror("Errore in sendto: invio del pacchetto syn_t");
            exit(EXIT_FAILURE);
        }
        printf("Sent SYN-ACK %d %d\n", syn->SYN, syn->ACK);

        while (recvfrom(sockfd, (void *) syn, sizeof(syn_t), MSG_DONTWAIT, //waiting for ACK
                        (struct sockaddr *) &servaddr, &slen) < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom() failed");
                free(syn);
                return NULL;
            }
        }

        if (syn->ACK == syn->SYN + 1){

            if (req->type == GET_REQ)
                get_command_handler(sockfd, servaddr, req->filename, path);
            else if (req->type == PUT_REQ)
                put_command_handler(sockfd, servaddr, req->filename);
            else if (req->type == LIST_REQ)
                list_command_handler(sockfd, servaddr);

        }
    }

    printf("cazzo\n");

    free(req);
    *no_connections -= 1;
    return NULL;
}
