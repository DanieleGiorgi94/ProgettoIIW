#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#define BUFFER_SIZE 2048
#define WINDOW_SIZE 1024
#define TIMEOUT     3000000
#define LOSS_PROB   10 

struct SR_thread_data { // dati della selective repeat
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

struct ackrec_thread_data {
	int sockfd; // descrittore della socket
	struct sockaddr *servaddr;
	struct circular_buffer *cb;
	pthread_t tid;
};

struct timer_thread_data {
	int sockfd; // descrittore della socket
	struct sockaddr *servaddr;
	struct circular_buffer *cb;
	pthread_t tid;
};

struct buf_node {
	pkt_t pkt;
	char acked;
	clock_t timer;
};

struct circular_buffer {
	u32 E;
	u32 S;
	u32 N; // indice del pacchetto da inviare
	u64 base; // n_seq del piu' vecchio pacchetto non acked
	u64 nextseqnum; // n_seq del pacchetto da inviare
	pthread_mutex_t mtx;
	struct buf_node cb_node[BUFFER_SIZE];
};
