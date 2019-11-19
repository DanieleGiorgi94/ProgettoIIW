#include "reliable_udp.h"

static void *selective_repeat_sender(void *);
static void *selective_repeat_receiver(void *);
static pkt_t create_pkt(int, u64);
static void *split_file(void *);
static void *merge_file(void *);
static void send_pkt(int, pkt_t *,const struct sockaddr *);
static void send_ack(int, struct sockaddr, u64);
static void *receive_ack(void *);
static void sorted_buf_insertion(struct circular_buffer *, struct buf_node *, u64);
static void lock_buffer(struct circular_buffer *);
static void unlock_buffer(struct circular_buffer *);
void alarm_handler(int);

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

static void *selective_repeat_sender(void *arg)
{
	struct SR_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
	int sockfd = ptd->sockfd;
    	const struct sockaddr *servaddr = ptd->servaddr;
	struct ackrec_thread_data ackrec;
	u32 n;

	ackrec.sockfd = sockfd;

	/* ricezione dell'ack */
	if (pthread_create(&ackrec.tid, NULL, receive_ack, &ackrec) != 0){
		perror("Errore in pthread_create()\n");
		exit(EXIT_FAILURE);
	}
	
	lock_buffer(cb);
	n = cb->S; // inizializzazione indice del paccheto da inviare
	unlock_buffer(cb);

	for(;;){
		lock_buffer(cb);

		while (cb->S == cb->E){
			/* buffer circolare vuoto */
			unlock_buffer(cb);
			usleep(100000);
			lock_buffer(cb);
		}

		if (cb->nextseqnum < cb->base + WINDOW_SIZE){
			/* finestra non piena */
			if (n != cb->E){
				/* n non supera cb->E */
				pkt_t pkt = cb->cb_node[n].pkt;
				send_pkt(sockfd, &pkt, servaddr);
				n = (n + 1) % BUFFER_SIZE;
				cb->nextseqnum = cb->cb_node[n].pkt.header.n_seq;
			}
		}
		unlock_buffer(cb);
	}

	if (pthread_join(ackrec.tid, NULL) != 0){
		perror("Errore in pthread_join()\n");
		exit(EXIT_FAILURE);
	}

	return NULL;
}

static void send_pkt(int sockfd, pkt_t *pkt, const struct sockaddr *servaddr)
{
	if (sendto(sockfd, pkt, sizeof(pkt_t), 0, (struct sockaddr *)servaddr,
			       sizeof(struct sockaddr)) < 0) {
		perror("Errore in sendto()");
		exit(EXIT_FAILURE);
	}
	
	printf("pkt %ld inviato\n", pkt->header.n_seq);
	//alarm(TIMEOUT);
}

void alarm_handler(int sig)
{
	(void)sig;
	// send_pkt();
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
	int n;
	pkt_t *pkt;

	for(;;){
		pkt = (pkt_t *)dynamic_allocation(sizeof(pkt_t));
		// ricezione pacchetto
		n = recvfrom(sockfd, (void *)pkt, sizeof(pkt_t), 0,
			       	(struct sockaddr *)servaddr, &slen);
		if (n < 0) {
			perror("Errore in recvfrom()");
			exit(EXIT_FAILURE);
		}

		lock_buffer(cb);

		nE = (cb->E + 1) % BUFFER_SIZE;
		while (nE == cb->S){
			/* buffer circolare pieno */
			unlock_buffer(cb);
			usleep(100000);
			lock_buffer(cb);
		}	

		struct buf_node cbn;
		cbn.pkt = *pkt;
		cbn.acked = 1;
		seqnum = pkt->header.n_seq;

		printf("ricevuti %d byte! n_seq = %ld\n", n, seqnum);

		// invio ack
		send_ack(sockfd, *servaddr, seqnum);

		// ordinamento dei pacchetti ricevuti
		sorted_buf_insertion(cb, &cbn, seqnum);

		unlock_buffer(cb);
	}	
}

static void send_ack(int sockfd, struct sockaddr servaddr, u64 seqnum){
	ack_t *ack = (ack_t *)dynamic_allocation(sizeof(ack_t));
	ack->n_seq = seqnum;

	/* invia il pacchetto contenente l'ack */
	if (sendto(sockfd, (void *)ack, sizeof(ack_t), 0, &servaddr, sizeof(servaddr)) < 0){
		perror("Errore in sendto: invio dell'ack");
		exit(EXIT_FAILURE); 
	}
}

static void *receive_ack(void *arg)
{
	struct ackrec_thread_data *ackrec = arg;
	int n;
        u64 i;
	ack_t *ack = (ack_t *)dynamic_allocation(sizeof(ack_t));
	u64 seqnum, tmp_seqnum;
	unsigned int slen = sizeof(struct sockaddr);
	
	int sockfd = ackrec->sockfd;
	struct sockaddr *servaddr = ackrec->servaddr;
	struct circular_buffer *cb = ackrec->cb;
	//cb = (struct circular_buffer *)dynamic_allocation(sizeof(struct circular_buffer));

	u32 I; // indice temporaneo per ricerca nel buffer
	
	while(1) {
		n = recvfrom(sockfd, (void *)ack, sizeof(ack_t), 0, servaddr, &slen);
		if (n < 0) {
			perror("Errore in recvfrom: ricezione dell'ack");
			exit(EXIT_FAILURE);
		} 
	        if (n > 0) {
			lock_buffer(cb);			

			if (cb->S < cb->E){
				I = cb->E + BUFFER_SIZE;
			} else {
				I = cb->E;
			}

    			tmp_seqnum = cb->cb_node[cb->S].pkt.header.n_seq;
			seqnum = ack->n_seq;
			
			//printf("ack %ld ricevuto\n", seqnum);

			i = seqnum - tmp_seqnum;
			if (cb->S + i < I){
				cb->cb_node[(cb->S + i) % BUFFER_SIZE].acked = 1;
			}

			while (cb->cb_node[cb->S].acked == 1){
				cb->S = (cb->S + 1) % BUFFER_SIZE;	
			}
			/* aggiorna base */
    			cb->base = cb->cb_node[cb->S].pkt.header.n_seq;

			unlock_buffer(cb);
		}
    	}

	return NULL;	
}

static void sorted_buf_insertion(struct circular_buffer *cb, struct buf_node *cbn, u64 seqnum)
{
	u64 tmp_seqnum;
	pkt_t current_pkt;
	u64 i;

	u32 I; // indice temporaneo per ricerca nel buffer
	if (cb->S < cb->E){
		I = cb->E + BUFFER_SIZE;
	} else {
		I = cb->E;
	}

	if (cb->S == cb->E){ // buffer circolare vuoto
		cb->cb_node[cb->E] = *cbn;
		cb->E = (cb->E + 1) % BUFFER_SIZE;
	} else {
		current_pkt = cb->cb_node[cb->S].pkt;
		tmp_seqnum = current_pkt.header.n_seq;
		i = seqnum - tmp_seqnum;

		if (i < BUFFER_SIZE){
			cb->cb_node[(cb->S + i) % BUFFER_SIZE] = *cbn;	
			if (i >= I - cb->S){
				cb->E = (cb->S + i + 1) % BUFFER_SIZE;
			}
		}
	}	
}


static pkt_t create_pkt(int fd, u64 nseq)
{
    char buff[MAX_PAYLOAD_SIZE];

    u64 read_byte = read_block(fd,buff, MAX_PAYLOAD_SIZE); //Legge dal file e crea pacchetti di dim MAX_PAYLOAD_SIZE

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

static void *merge_file(void *arg)
{
	struct sender_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
	int fd = ptd->fd;
  	char acked;
    	u64 written_byte;

	for(;;){
		while (cb->S == cb->E){
			/* buffer circolare vuoto */
			usleep(100000);
		}

		acked = cb->cb_node[cb->S].acked;
		if (acked == 1){
			pkt_t pkt = cb->cb_node[cb->S].pkt;
			cb->cb_node[cb->S].acked = 0;

		//	printf("Reading pkt %ld from cb\n", pkt.header.n_seq);

			written_byte = write_block(fd, pkt.payload, MAX_PAYLOAD_SIZE);

			if (written_byte < MAX_PAYLOAD_SIZE)
			    pthread_exit(NULL);

			cb->S = (cb->S + 1) % BUFFER_SIZE;
		}
	}

}

void send_file(int sockfd, struct sockaddr *servaddr, int fd)
{
	struct SR_thread_data SR_td;
	struct sender_thread_data sender_td;

	struct circular_buffer *cb;
	cb = (struct circular_buffer *)
        dynamic_allocation(sizeof(struct circular_buffer));
	cb->E = 0;
	cb->S = 0;
	cb->base = 0;
	cb->nextseqnum = 0;

	if (pthread_mutex_init(&(cb->mtx), NULL) != 0){
		perror("Errore in pthread_mutex_init()\n");
		exit(EXIT_FAILURE);
	}

	// inizializzazione dei thread
	SR_td.cb = cb;
	sender_td.cb = cb;
	SR_td.sockfd = sockfd;
	SR_td.servaddr = servaddr;
	sender_td.fd = fd;

	if ((pthread_create(&sender_td.tid, NULL, split_file, &sender_td) != 0) ||
		(pthread_create(&SR_td.tid, NULL, selective_repeat_sender, &SR_td) != 0)){
		perror("Errore in pthread_create()");
		exit(EXIT_FAILURE);
	}

	if ((pthread_join(SR_td.tid, NULL) || 
				pthread_join(sender_td.tid, NULL))!= 0){
		perror("Errore in pthread_join()");
		exit(EXIT_FAILURE);
	}

	pthread_exit(EXIT_SUCCESS);

}

void receive_file(int sockfd, struct sockaddr *servaddr, int fd)
{
	struct SR_thread_data SR_td;
	struct sender_thread_data receiver_td;

	struct circular_buffer *cb;
	cb = (struct circular_buffer *)
        dynamic_allocation(sizeof(struct circular_buffer));
	cb->S = 0;
	cb->E = 0;
	cb->base = 0;
	cb->nextseqnum = 0;
	
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

	if ((pthread_create(&receiver_td.tid, NULL, merge_file, &receiver_td) != 0) ||
	    (pthread_create(&SR_td.tid, NULL, selective_repeat_receiver, &SR_td) 
	     != 0)){
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

