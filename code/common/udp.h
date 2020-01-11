#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#define MAX_PAYLOAD_SIZE 1460

#define NORM_PKT 0
#define END_OF_FILE 1

typedef struct {
    u64 n_seq;
    u32 length;
    u32 rwnd;
/*Il campo type credo debba distinguere i pacchetti per la connessione tra
client e server, per i comandi get, put e list, e, siccome il server deve
rispondere con l'esito delle operazioni richieste dal client, deve distinguere
l'esito positivo/negativo per ciascuna operazione (ma forse quest'ultima cosa è
meglio farla usando un ACK: in questo caso il campo type forse potrebbe servire
per distinguere a quale operazione si riferisce) */
    char type;
} header_t;

typedef struct {
    header_t header;
    char payload[MAX_PAYLOAD_SIZE];
} pkt_t;

typedef struct {
    u64 n_seq;
    char type;
} ack_t;
