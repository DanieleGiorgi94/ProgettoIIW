#include "reliable_udp.h"

// MUTEX
static void lock_buffer(struct circular_buffer *);
static void unlock_buffer(struct circular_buffer *);

// SENDER
static void *receive_ack(void *);
char sym_lost_pkt(void);
static void send_pkt(int, pkt_t *,const struct sockaddr *);
static void *selective_repeat_sender(void *);
static pkt_t create_pkt(int, u64);
static void *split_file(void *);
static void *timeout_handler(void *);

// RECEIVER
static char sorted_buf_insertion(struct circular_buffer *, struct buf_node, u64);
static void send_ack(int, struct sockaddr, u64);
static void *selective_repeat_receiver(void *);
static void *merge_file(void *);

/* ********** MUTEX ********** */

static void lock_buffer(struct circular_buffer *cb)
{
	if (pthread_mutex_lock(&cb->mtx) != 0){
		perror("Errore in pthread_mutex_lock()\n");
		exit(EXIT_FAILURE);
	}
}

static void unlock_buffer(struct circular_buffer *cb)
{
	if (pthread_mutex_unlock(&cb->mtx) != 0){
		perror("Errore in pthread_mutex_lock()\n");
		exit(EXIT_FAILURE);
	}
}

/* ********** SENDER ********** */

static void *receive_ack(void *arg)
{
	struct ackrec_thread_data *ackrec = arg;
        u64 i;
	ack_t *ack = (ack_t *)dynamic_allocation(sizeof(ack_t));
	unsigned int slen = sizeof(struct sockaddr);
	
	int sockfd = ackrec->sockfd;
	struct sockaddr *servaddr = ackrec->servaddr;
	struct circular_buffer *cb = ackrec->cb;
	u32 I; // indice temporaneo per ricerca nel buffer
	u64 seqnum;
	
	while(1) {
		if (recvfrom(sockfd, (void *)ack, sizeof(ack_t), 0, servaddr, &slen) < 0) {
			perror("Errore in recvfrom: ricezione dell'ack");
			exit(EXIT_FAILURE);
		} 
	        
		lock_buffer(cb);			

		if (cb->S > cb->E){
			I = cb->E + BUFFER_SIZE;
		} else {
			I = cb->E;
		}

		i = ack->n_seq % BUFFER_SIZE;
		seqnum = cb->cb_node[i].pkt.header.n_seq;

		if (cb->S <= i && i < I && seqnum == ack->n_seq){
			cb->cb_node[i].acked = 1;
			printf("ack %ld ricevuto\n", ack->n_seq);
		}

		while (cb->cb_node[cb->S].acked == 1){
			if (cb->S == cb->E) break;
			cb->S = (cb->S + 1) % BUFFER_SIZE;	
		}

		unlock_buffer(cb);
    	}

	return NULL;	
}

char sym_lost_pkt(void)
{
	/* simula perdita pacchetti con probabilita' LOSS_PROB */
	int i = rand() % 101; // genera numero da 1 a 100
	return i <= LOSS_PROB;
}

static void send_pkt(int sockfd, pkt_t *pkt, const struct sockaddr *servaddr)
{
	if (!sym_lost_pkt()){	
		if (sendto(sockfd, pkt, sizeof(pkt_t), 0, (struct sockaddr *)servaddr,
				       sizeof(struct sockaddr)) < 0) {
			perror("Errore in sendto()");
			exit(EXIT_FAILURE);
		}
		printf("pkt %ld inviato\n", pkt->header.n_seq);
	} else {
		printf("pkt %ld perduto\n", pkt->header.n_seq);
	}
}

static void *selective_repeat_sender(void *arg)
{
	struct SR_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
	int sockfd = ptd->sockfd;
    	const struct sockaddr *servaddr = ptd->servaddr;

	for(;;){
		lock_buffer(cb);

		while (cb->S == cb->E){
			/* buffer circolare vuoto */
			unlock_buffer(cb);
			usleep(100000);
			lock_buffer(cb);
		}

		if (cb->N < cb->S + WINDOW_SIZE){
			/* finestra non piena */
			if (cb->N != cb->E){
				/* cb->N non supera cb->E */
				pkt_t pkt = cb->cb_node[cb->N].pkt;
				send_pkt(sockfd, &pkt, servaddr); // invio pacchetto 
				cb->cb_node[cb->N].timer = clock(); // start timer
				cb->N = (cb->N + 1) % BUFFER_SIZE;
			}
		}
		unlock_buffer(cb);
	}

	return NULL;
}

static pkt_t create_pkt(int fd, u64 nseq)
{
	char buff[MAX_PAYLOAD_SIZE];

    	u64 read_byte = read_block(fd,buff, MAX_PAYLOAD_SIZE); 
    	//Legge dal file e crea pacchetti di dim MAX_PAYLOAD_SIZE

        printf("*******%u\n", lseek(fd, 0, SEEK_CUR));

    	if (read_byte < MAX_PAYLOAD_SIZE){
        	pthread_exit(NULL);
    	}

    	header_t header;
    	header.n_seq = nseq;
    	header.length = (u32) read_byte;
    	header.rwnd = 0;
    	header.type = 0;

    	pkt_t pkt;
    	pkt.header = header;
    	strncpy(pkt.payload, buff, read_byte);

    	return pkt;
}

static void *split_file(void *arg)
{
	struct sender_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
	int fd = ptd->fd;
	
	for(u64 i = 0;1;i++){
		pkt_t pkt = create_pkt(fd, i);
		u32 nE;

		printf("pkt %ld created\n", i);
		
		lock_buffer(cb);

		nE = (cb->E + 1) % BUFFER_SIZE;
		while (nE == cb->S){
			/* buffer circolare pieno */
			unlock_buffer(cb);
			usleep(100000);
			lock_buffer(cb);
		}

		struct buf_node cbn;
		cbn.pkt = pkt;
		cbn.acked = 0;

		cb->cb_node[cb->E] = cbn;
		cb->E = nE;
		
		unlock_buffer(cb);
	}

	return NULL;
}

static void *timeout_handler(void *arg)
{
	struct timer_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;	
	int sockfd = ptd->sockfd;
	struct sockaddr *servaddr = ptd->servaddr;
	u32 I; // indice temporaneo per ricerca nel buffer
	clock_t tspan; 
	pkt_t pkt;

	for (;;){
		lock_buffer(cb);

		while (cb->S == cb->E){
			/* buffer circolare vuoto */
			unlock_buffer(cb);
			usleep(1000000);
			lock_buffer(cb);
		}

		if (cb->S > cb->N){
			I = cb->N + BUFFER_SIZE;
		} else {
			I = cb->N;
		}

		for (u32 i = cb->S; i < I; i++){
			if (cb->cb_node[i % BUFFER_SIZE].acked == 0){
				tspan = clock() - cb->cb_node[i % BUFFER_SIZE].timer;
				if (tspan >= TIMEOUT){
					pkt = cb->cb_node[i % BUFFER_SIZE].pkt;
					send_pkt(sockfd, &pkt, servaddr);
					printf("inviato per timeout\n");
					cb->cb_node[i % BUFFER_SIZE].timer = clock();
				}
			}
		}

		unlock_buffer(cb);
	}

	return NULL;
}

void send_file(int sockfd, struct sockaddr *servaddr, int fd, char *filename)
{
	struct SR_thread_data SR_td; // thread che segmenta il file in pacchetti
	struct sender_thread_data sender_td; // thread che invia i pacchetti
	struct ackrec_thread_data ackrec; // thread che riceve gli ack
	struct timer_thread_data tmr_td; // thread che gestisce il timeout

	struct circular_buffer *cb;
	cb = (struct circular_buffer *)
        dynamic_allocation(sizeof(struct circular_buffer));
	cb->E = 0;
	cb->S = 0;
	cb->N = 0;

	srand(time(NULL)); // seed per simulare probabilita' di perdita pacchetti

	if (pthread_mutex_init(&(cb->mtx), NULL) != 0){
		perror("Errore in pthread_mutex_init()\n");
		exit(EXIT_FAILURE);
	}

	// inizializzazione dei thread
	SR_td.cb = cb;
	sender_td.cb = cb;
	ackrec.cb = cb;
	tmr_td.cb = cb;

	SR_td.sockfd = sockfd;
	ackrec.sockfd = sockfd;
	tmr_td.sockfd = sockfd;

	SR_td.servaddr = servaddr;
	ackrec.servaddr = servaddr;
	tmr_td.servaddr = servaddr;

	sender_td.fd = fd;

	if ((pthread_create(&sender_td.tid, NULL, split_file, &sender_td) ||
		pthread_create(&SR_td.tid, NULL, selective_repeat_sender, &SR_td) || 
		pthread_create(&ackrec.tid, NULL, receive_ack, &ackrec) ||
		pthread_create(&tmr_td.tid, NULL, timeout_handler, &tmr_td)) != 0) {
		perror("Errore in pthread_create()");
		exit(EXIT_FAILURE);
	}

	if ((pthread_join(SR_td.tid, NULL) || 
			pthread_join(sender_td.tid, NULL) ||
			pthread_join(ackrec.tid, NULL) ||
			pthread_join(tmr_td.tid, NULL)) != 0){
		perror("Errore in pthread_join()");
		exit(EXIT_FAILURE);
	}

	pthread_exit(EXIT_SUCCESS);
}

/* ********** RECEIVER ********** */

static char sorted_buf_insertion(struct circular_buffer *cb, struct buf_node cbn, u64 seqnum)
{
	/* restituisce 1 se il pacchetto viene accettato, 0 se scartato */
	u64 i = seqnum % BUFFER_SIZE;

	/* scarto il pacchetto se il nodo e' occupato */
	if ((cb->cb_node[i].acked == 1) || 
			(i == (cb->S + BUFFER_SIZE - 1) % BUFFER_SIZE)) {
		printf("Scarto pacchetto %ld\n", seqnum);
		return 0;
	} 

	cb->cb_node[i] = cbn;
	printf("inserisco pacchetto %ld\n", cbn.pkt.header.n_seq); 	

	if (cb->S <= cb->E){
		if (i > cb->E) cb->E = (i + 1) % BUFFER_SIZE;
	} else {
		if (i > cb->E && i < cb->S - 1) cb->E = (i + 1) % BUFFER_SIZE;
	}

	return 1;
}

static void send_ack(int sockfd, struct sockaddr servaddr, u64 seqnum)
{
	ack_t *ack = (ack_t *)dynamic_allocation(sizeof(ack_t));
	ack->n_seq = seqnum;

	/* invia il pacchetto contenente l'ack */
	if (sendto(sockfd, (void *)ack, sizeof(ack_t), 0, &servaddr, sizeof(servaddr)) < 0){
		perror("Errore in sendto: invio dell'ack");
		exit(EXIT_FAILURE); 
	}
}

static void *selective_repeat_receiver(void *arg)
{
	struct SR_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
	int sockfd = ptd->sockfd;
    	const struct sockaddr *servaddr = ptd->servaddr;
	unsigned int slen = sizeof(struct sockaddr);
	u64 seqnum;
	u32 nE;
	pkt_t *pkt;

	for(;;){
		pkt = (pkt_t *)dynamic_allocation(sizeof(pkt_t));
		// ricezione pacchetto
		if (recvfrom(sockfd, (void *)pkt, sizeof(pkt_t), 0,
			       	(struct sockaddr *)servaddr, &slen) < 0){
			perror("Errore in recvfrom()");
			exit(EXIT_FAILURE);
		}

		struct buf_node cbn;
		cbn.pkt = *pkt;
		cbn.acked = 1;
		seqnum = pkt->header.n_seq;

		lock_buffer(cb);

		nE = (cb->E + 1) % BUFFER_SIZE;
		while (nE == cb->S){
			/* buffer circolare pieno */
			unlock_buffer(cb);
			usleep(100000);
			lock_buffer(cb);
		}	

		printf("ricevuto pkt %ld\n", seqnum);

		// inserimento ordinato del pacchetto ricevuto
		if (sorted_buf_insertion(cb, cbn, seqnum)){
			// invio ack
			unlock_buffer(cb);
			send_ack(sockfd, *servaddr, seqnum);
			lock_buffer(cb);
		}

		unlock_buffer(cb);
	}	
}

static void *merge_file(void *arg)
{
	struct sender_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
	int fd = ptd->fd;
  	char acked;
    	u64 written_byte;

	for(;;){
		lock_buffer(cb);

		while (cb->S == cb->E){
			/* buffer circolare vuoto */
			unlock_buffer(cb);
			usleep(100000);
			lock_buffer(cb);
		}

		acked = cb->cb_node[cb->S].acked;
		while (acked == 1){
			pkt_t pkt = cb->cb_node[cb->S].pkt;
			cb->cb_node[cb->S].acked = 0;

			printf("Reading pkt %ld from cb\n", pkt.header.n_seq);

			written_byte = write_block(fd, pkt.payload, MAX_PAYLOAD_SIZE);

			if (written_byte < MAX_PAYLOAD_SIZE){
			    perror("Error in write_block()\n");
                pthread_exit(NULL);
			}

			cb->S = (cb->S + 1) % BUFFER_SIZE;
			acked = cb->cb_node[cb->S].acked;
		}

		unlock_buffer(cb);
	}

}

void receive_file(int sockfd, struct sockaddr *servaddr, int fd)
{
	struct SR_thread_data SR_td; // thread che riceve i pacchetti
	struct sender_thread_data receiver_td; // thread che scrive su file 
					       // i pacchetti ricevuti

	struct circular_buffer *cb;
	cb = (struct circular_buffer *)
        dynamic_allocation(sizeof(struct circular_buffer));
	cb->S = 0;
	cb->E = 0;
	cb->N = 0;
	
	if (pthread_mutex_init(&(cb->mtx), NULL) != 0){
		perror("Errore in pthread_mutex_init()\n");
		exit(EXIT_FAILURE);
	}

	// inizializzazione dei thread
	SR_td.cb = cb;
	receiver_td.cb = cb;
	SR_td.sockfd = sockfd;
	SR_td.servaddr = servaddr;
	receiver_td.fd = fd;

	if ((pthread_create(&receiver_td.tid, NULL, merge_file, &receiver_td) ||
	    pthread_create(&SR_td.tid, NULL, selective_repeat_receiver, &SR_td)) 
	     != 0){
		perror("Errore in pthread_create()");
		exit(EXIT_FAILURE);
	}

	if ((pthread_join(SR_td.tid, NULL) || 
			pthread_join(receiver_td.tid, NULL))!= 0){
		perror("Errore in pthread_join()");
		exit(EXIT_FAILURE);
	}

	pthread_exit(EXIT_SUCCESS);
}

