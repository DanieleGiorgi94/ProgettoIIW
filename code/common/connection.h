#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

/*typedef struct {
    u64 initial_n_seq;
    char SYN;
    char ACK;
    char FIN;
} syn_t;*/

#define GET_REQ 0
#define PUT_REQ 1
#define LIST_REQ 2
#define FILEON 3
#define FILEOFF 4

typedef struct {
    u64 initial_n_seq;
    char SYN;
    char ACK;
    char FIN;
    char type; //GET_REQ, PUT_REQ or LIST_REQ
    char filename[BUFLEN];
} request_t;
