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
