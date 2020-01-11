#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#define BUFFER_SIZE 400
#define WINDOW_SIZE 400
#define TIMEOUT     2000000
#define LOSS_PROB   0

#define PORT   5193

struct SR_thread_data { // dati della selective repeat
	int sockfd; // descrittore della socket
	struct sockaddr *servaddr; // indirizzo del server
	struct circular_buffer *cb;
    pthread_mutex_t *mtx;
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
    pthread_mutex_t *mtx;
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
	char acked; // ack ricevuto (invio) o nodo occupato (ricezione)
	clock_t timer; // timer (invio)
};

struct circular_buffer {
	u32 E; // indice della fine del buffer (end)
	u32 S; // indice dell'inizio del buffer (start/base)
	u32 N; // indice del pacchetto da inviare (nextseqnum)
	pthread_mutex_t mtx;
	struct buf_node cb_node[BUFFER_SIZE];
};
