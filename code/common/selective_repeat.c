#include "reliable_udp.h"

u64 N; // Ampiezza della finestra
u64 send_base; // Piu' vecchio pacchetto che non ha ricevuto ack
u64 recv_base; //
u64 nextseqnum; // numero di sequenza del prossimo pachetto da inviare

static void *selective_repeat_sender(void *);
static void *selective_repeat_receiver(void *);
static pkt_t create_pkt(int, int);
static void *split_file(void *);
static void *merge_file(void *);
static u64 get_ack_seqnum(ack_t);
static u64 update_window(u64, u64, u64); 
static int window_not_full(u64, u64, u64);

static void *selective_repeat_sender(void *arg)
{
	struct SR_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
    int sockfd = ptd->sockfd;
    const struct sockaddr *servaddr = ptd->servaddr;

	u64 N, base, nextseqnum;

	// TODO: creo thread invio/ricezione

	for(int i = 0; i < 10000; i++){
		while (cb->S == cb->E){
			/* buffer circolare vuoto */
			usleep(100000);
		}
		pkt_t pkt = cb->cb_node[cb->S].pkt;

		if (window_not_full(0, 0, 0)){ // TODO: window_not_full da implementare
			if (sendto(sockfd, &pkt, sizeof(pkt), 0, servaddr,
                    sizeof(struct sockaddr)) < 0){
				perror("Errore in sendto()");
				exit(EXIT_FAILURE);
			}
		}
	}

	return NULL;
}

static void *selective_repeat_receiver(void *arg)
{
	struct SR_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;

	for(;;){
		pkt_t *pkt = (pkt_t *) dynamic_allocation(sizeof(pkt_t));
		int nE;

		// TODO: pkt = recvfrom
		nE = (cb->E + 1) % BUFFER_SIZE;
		while (nE == cb->S){
			/* buffer circolare pieno */
			usleep(100000);
		}

		struct buf_node cbn;
		cbn.pkt = *pkt;
		cbn.acked = 1;

		// TODO: send_ack();
		// TODO: scrittura ordinata sul buffer!!!
		cb->cb_node[cb->E] = cbn;
		cb->E = nE;
	}
}

static pkt_t create_pkt(int fd, int nseq)
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
	
	for(int i = 0;1;i++){
		pkt_t pkt = create_pkt(fd, i);
		int nE;

		//printf("pkt %d created\n", i);

		nE = (cb->E + 1) % BUFFER_SIZE;
		while (nE == cb->S){
			/* buffer circolare pieno */
			usleep(100000);
		}

		struct buf_node cbn;
		cbn.pkt = pkt;
		cbn.acked = 0;

		cb->cb_node[cb->E] = cbn;
		cb->E = nE;
	}
}

static void *merge_file(void *arg)
{
	struct sender_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
	int fd = ptd->fd;
    int acked;
    u64 written_byte;

	for(;;){
		while (cb->S == cb->E){
			/* buffer circolare vuoto */
			usleep(100000);
		}

		acked = cb->cb_node[cb->S].acked;
		if (acked != 0){
			pkt_t pkt = cb->cb_node[cb->S].pkt;

			//printf("Reading pkt %d from cb\n", pkt.header.n_seq);

			written_byte = write_block(fd, pkt.payload, MAX_PAYLOAD_SIZE);

			if (written_byte < MAX_PAYLOAD_SIZE)
			    pthread_exit(NULL);

			cb->S = (cb->S + 1) % BUFFER_SIZE;
		}
	}

}

/********************** ACK ************************/

void send_ack(int sockfd, struct sockaddr servaddr, u64 seqnum){
	ack_t ack;
	/* inizializzazione area di memoria per ack */
	memset((void *)&ack, 0, sizeof(ack));
	ack.n_seq = seqnum;

	/* invia il pacchetto contenente l'ack */
	if (sendto(sockfd, &ack, sizeof(ack), 0, &servaddr, sizeof(servaddr)) < 0){
		perror("Errore in sendto: invio dell'ack");
		exit(-1); 
	}
}

ack_t recv_ack(int sockfd, struct sockaddr servaddr){
	int n;
	ack_t ack;
	ack_t *buff = dynamic_allocation(sizeof(ack_t));

	n = recvfrom(sockfd, buff, sizeof(ack_t), 0, &servaddr, NULL);
	if (n < 0) {
		perror("Errore in recvfrom: ricezione dell'ack");
		exit(-1);
	}
	else {
        if (n > 0) {
    		ack = *buff;
    	}
    	else {
    		memset((void *) &ack, 0, sizeof(ack_t));
    		ack.n_seq = -1;
    	}
    }
	return ack;	
}

static u64 get_ack_seqnum(ack_t ack){
	return ack.n_seq;
}

static u64 update_window(u64 seqnum, u64 base, u64 nextseqnum){
	if (seqnum == base){
		/* aggiorna base */
		for (u64 i = base; i < nextseqnum;  i++) {
		     /* temp = pkt[i]; 
			if (temp.ack_received == 0) 
                        {
				base = temp.n_seq;
				break; 
			} */ 
		}
	}
	return base;
} 

static int window_not_full(u64 base, u64 nextseqnum, u64 N){
	return (nextseqnum < base + N);
}

void send_file(int sockfd, struct sockaddr *servaddr, int fd)
{
	struct SR_thread_data SR_td;
	struct sender_thread_data sender_td;
	pthread_t self_tid = pthread_self();

	struct circular_buffer *cb;
	cb = (struct circular_buffer *)
        dynamic_allocation(sizeof(struct circular_buffer));

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

}

void receive_file(int sockfd, struct sockaddr *servaddr, int fd)
{
	struct SR_thread_data SR_td;
	struct sender_thread_data receiver_td;
	pthread_t self_tid = pthread_self();

	struct circular_buffer *cb;
	cb = (struct circular_buffer *)
        dynamic_allocation(sizeof(struct circular_buffer));

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

}

/****************************************************/

//void send_pkt(int sockfd, struct sockaddr servaddr, struct pkt_t pkt)
//{
//	// Invio pacchetto con udp
//	if (sendto(sockfd, &pkt, sizeof(pkt),0,&servaddr,sizeof(servaddr)) < 0){
//		perror("Errore in sendto: invio del pacchetto");
//		exit(-1);
//	}
//    	printf("sending pkt...\n");
//}
//
//void selective_repeat(){
//
//	/* apri thread che ascolta pacchetti in ingresso */
//	while(1){
//		/* aspetta invocazione dall'alto */
//		if (window_not_full(base, nextseqnum, N)){
//			/* genera pacchetto (prendilo dal buffer)*/ 
//			send_pkt(sockfd, servaddr, pkt);
//		}
//	}
//}
//
//void timeout_handler(){
//	/* invia pacchetto */
//}


//void event_rdt_send(void)
//{
//	/* controllo che la finestra non sia piena */
//    	if (nextseqnum < base + N){
//		// funzione che crea il pacchetto da inviare
//		// send_pkt(char * pacchetto creato);
//		/* controllo che tutti i pacchetti inviati abbiano ricevuto l'ack */
//		if (base == nextseqnum){
//			// start_timer(pacchetto);
//		}
//		nextseqnum++;
//    	}
//	else {
//		// rifiuto il dato oppure lo mantengo in una lista collegata 
//		// in attesa che si liberi la finestra
//	}
//}



/*
CLIENT:

rdt_send(data)
timeout
rdt_rcv(rcvpkt) && notcorrupt(rcvpkt)
rdt_rcv(rcvpkt) && corrupt(rcvpkt)

SERVER:

rdt_rcv(rcvpkt) && notcorrupt(rcvpkt) && hasseqnum(rcvpkt,expectedseqnum)
default
*/

/**************************************************************************
                 PSEUDOCODICE PER ORGANIZZARE IL LAVORO
**************************************************************************/
/*

void send_pkt(pkt);      // invio pacchetto
struct pkt rcv_pkt();    // ricezione pacchetto (qualunque tipo, ack compreso)
void start_timer(pkt_num); // timer relativo allo specifico pacchetto
void timeout_handler();  // Gestore dell'interruzione rerlativa al timeout

send_pkt {
	controlla che la finestra non sia piena
	estrai copia del pacchetto dal buffer
	invia il pacchetto con udp
	fai partire il timer relativo (ev. in un altro processo?)
	
	

}*/

//int SR_main_sender(void)
//{
///*** main del processo che si occupa della comunicazione affidabile ***/
//	struct circular_buffer *cb;
//
//	/* implementare funzione che crea e gestisce memoria condivisa:
//	 * cb = get_shared_memory();  */
//
//	for(;;) {
//		while (cb->S == cb->E) {
//			/* buffer circolare vuoto */
//			usleep(100000);
//		}
//		/* gestione finestra*/
//		send_pkt(sockfd, cb->buf[cb->S]);
//		/* controllo pacchetti in arrivo (se ack):
//		 * if (rcvfrom (...) != 0) {//estrai seqnum da ack} */
//		
//	}
//}
