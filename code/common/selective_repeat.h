#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#define BUFFER_SIZE 2048
#define WINDOW_SIZE 1024
#define TIMEOUT = 10

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

struct buf_node {
	pkt_t pkt;
	char acked;
};

struct circular_buffer {
	u32 E;
	u32 S;
	u64 base;
	u64 nextseqnum;
	pthread_mutex_t mtx;
	struct buf_node cb_node[BUFFER_SIZE];
};
