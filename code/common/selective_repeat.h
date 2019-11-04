#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#define BUFFER_SIZE 1200
#define SENDER 1
#define RECEIVER 2

struct SR_thread_data { // dati della selective repeat
	char type; // sender o receiver
	int sockfd; // descrittore della socket
	struct sockaddr *servaddr; // indirizzo del server
	struct circular_buffer *cb;
	pthread_t tid;
};

struct sender_thread_data { // dati del client/server
	int fd; // descrittore del file da leggere
	struct circular_buffer *cb;
	pthread_t tid;
};

struct buf_node {
	pkt_t pkt;
	char acked;
};

struct circular_buffer {
	int E;
	int S;
	struct buf_node cb_node[BUFFER_SIZE];
};
