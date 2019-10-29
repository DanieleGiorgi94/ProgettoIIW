#include "reliable_udp.h"
#include <pthread.h>

#define BUFFER_SIZE 1200
#define SENDER 1
#define RECEIVER 2

u64 N; // Ampiezza della finestra
u64 send_base; // Piu' vecchio pacchetto che non ha ricevuto ack
u64 recv_base; //
u64 nextseqnum; // numero di sequenza del prossimo pachetto da inviare

struct SR_thread_data { // dati della selective repeat
	char type; // sender o receiver
	int sockfd; // descrittore della socket
	struct sockaddr_in *servaddr; // indirizzo del server
	struct circular_buffer *cb;
	pthread_t tid;
};

struct sender_thread_data { // dati del client/server
	int fd; // descrittore del file da leggere
	struct circular_buffer *cb;
	pthread_t tid;
}

struct buf_node {
	pkt_t pkt;
	char acked;
};

struct circular_buffer {
	int E;
	int S;
	struct buf_node cb_node[BUFFER_SIZE];
};

void send_file(int sockfd, sockaddr_in *servaddr, int fd)
{
	struct SR_thread_data SR_td;
	struct sender_thread_data sender_td;
	pthread_t self_tid = pthread_self();

	struct circular_buffer *cb;
	cb = (struct circular_buffer *)dynamic_allocation(sizeof(struct circular_buffer));

	// inizializzazione dei thread
	SR_td.cb = cb;
	sender_td.cb = cb;
	SR_td.sockfd = sockfd;
	SR_td.servaddr = servaddr;
	sender_td.fd = fd;

	if ((pthread_create(sender_td.tid, NULL, split_file, sender_td) != 0) ||
			(pthread_create(SR_td.tid, NULL, selective_repeat_sender, SR_td) != 0)){
			perror("Errore in pthread_create()");
			exit(EXIT_FAILURE);
	}

	if ((pthread_join(SR_td.tid, NULL) || 
				pthread_join(sender_td.tid, NULL))!= 0){
		perror("Errore in pthread_join()");
		exit(EXIT_FAILURE);
	}

}

void receive_file(int sockfd, sockaddr_in *servaddr, int fd)
{
	struct SR_thread_data SR_td;
	struct sender_thread_data receiver_td;
	pthread_t self_tid = pthread_self();

	struct circular_buffer *cb;
	cb = (struct circular_buffer *)dynamic_allocation(sizeof(struct circular_buffer));

	// inizializzazione dei thread
	SR_td.cb = cb;
	receiver_td.cb = cb;
	SR_td.sockfd = sockfd;
	SR_td.servaddr = servaddr;
	receiver_td.fd = fd;

	if ((pthread_create(receiver_td.tid, NULL, merge_file, receiver_td) != 0) ||
			(pthread_create(SR_td.tid, NULL, selective_repeat_receiver, SR_td) != 0)){
			perror("Errore in pthread_create()");
			exit(EXIT_FAILURE);
	}

	if ((pthread_join(SR_td.tid, NULL) || 
				pthread_join(sender_td.tid, NULL))!= 0){
		perror("Errore in pthread_join()");
		exit(EXIT_FAILURE);
	}

}


void *selective_repeat_sender(void *arg)
{
	struct SR_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;

	u64 N, base, nextseqnum;

	// TODO: creo thread invio/ricezione

	for(;;){
		while (cb->S == cb->E){
			/* buffer circolare vuoto */
			usleep(100000);
		}

		pkt_t pkt = cb->cb_node[cb->S].pkt;

		if (window_not_full()){ // TODO: window_not_full da implementare
			if (sendto(sockfd, &pkt, sizeof(pkt), 0, &servaddr, sizeof(sockaddr_in)) < 0){
				perror("Errore in sendto()");
				exit(EXIT_FAILURE);
			}
		}
	}

	return NULL;
}


void *selective_repeat_receiver(void *arg)
{
	struct SR_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;

	for(;;){
		pkt_t *pkt = *dynamic_allocation(sizeof(pkt_t));
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

	return NULL;
}

pkt_t create_pkt(int fd)
{
 // TODO: implementare read dal file e scrittura nel pacchetto
}

void *split_file(void *arg)
{
	struct sender_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
	int fd = ptd->fd;
	
	for(;;){
		pkt_t pkt = create_pkt(fd);
		int nE;

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

	return NULL;
}

void *merge_file(void *arg)
{
	struct sender_thread_data *ptd = arg;
	struct circular_buffer *cb = ptd->cb;
	int fd = ptd->fd;

	for(;;){
		while (cb->S == cb->E){
			/* buffer circolare vuoto */
			usleep(100000);
		}

		acked = cb->cb_node[cb->S].acked;
		if (acked != 0){
			pkt_t pkt = cb->cb_node[cb->S].pkt;
			// TODO: scrivere dati del pacchetto sul file
			cb->S = (cb->S + 1) % BUFFER_SIZE;
		}
	}

	return NULL;

}





















/********************** ACK ************************/

void send_ack(int sockfd, struct sockaddr servaddr, u64 seqnum){
	ack_t ack;
	/* inizializzazione area di memoria per ack */
	memset((void *)&ack, 0, sizeof(ack));
	ack.seq_num = seqnum;

	/* invia il pacchetto contenente l'ack */
	if (sendto(sockfd, &ack, sizeof(ack), 0, &servaddr, sizeof(servaddr)) < 0){
		perror("Errore in sendto: invio dell'ack");
		exit(-1); 
	}
}

ack_t recv_ack(int sockfd, struct sockaddr servaddr){
	int n;
	ack_t ack;
	void *buff = malloc(sizeof(ack));

	n = recvfrom(sockfd, buff, sizeof(ack), 0, &servaddr, sizeof(servaddr));
	if (n < 0){
		perror("Errore in recvfrom: ricezione dell'ack");
		exit(-1);
	}
	if (n > 0){
		ack = *(ack_t *)buff;
	}
	if (n = 0){
		memset((void *)&ack, 0, sizeof(ack));
		ack.n_seq = -1;
	}
	return ack;	
}

u64 get_ack_seqnum(ack_t ack){
	return ack.n_seq;
}

u64 update_window(u64 seqnum, u64 base, u64 nextseqnum){
	if (seqnum == base){
		/* aggiorna base */
		for (i = base; i < nextseqnum;  i++) {
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

int window_not_full(u64 base, u64 nextseqnum, u64 N){
	return(nextseqnum < base + N)
}

/****************************************************/

/********************* PACCHETTO ********************/

void send_pkt(int sockfd, struct sockaddr servaddr, struct pkt_t pkt)
{
	// Invio pacchetto con udp
	if (sendto(sockfd, &pkt, sizeof(pkt),0,&servaddr,sizeof(servaddr)) < 0){
		perror("Errore in sendto: invio del pacchetto");
		exit(-1);
	}
    	printf("sending pkt...\n");
}

/****************************************************/

/**************** SELECTIVE REPEAT ******************/

void selective_repeat(){

	/* apri thread che ascolta pacchetti in ingresso */
	while(1){
		/* aspetta invocazione dall'alto */
		if (window_not_full(base, nextseqnum, N)){
			/* genera pacchetto (prendilo dal buffer)*/ 
			send_pkt(sockfd, servaddr, pkt);
		}
	}
}

void timeout_handler(){
	/* invia pacchetto */
}

/****************************************************/












void event_rdt_send(void)
{
	/* controllo che la finestra non sia piena */
    	if (nextseqnum < base + N){
		// funzione che crea il pacchetto da inviare
		// send_pkt(char * pacchetto creato);
		/* controllo che tutti i pacchetti inviati abbiano ricevuto l'ack */
		if (base == nextseqnum){
			// start_timer(pacchetto);
		}
		nextseqnum++;
    	}
	else {
		// rifiuto il dato oppure lo mantengo in una lista collegata 
		// in attesa che si liberi la finestra
	}
}



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

int SR_main_sender(void)
{
/*** main del processo che si occupa della comunicazione affidabile ***/
	struct circular_buffer *cb;

	/* implementare funzione che crea e gestisce memoria condivisa:
	 * cb = get_shared_memory();  */

	for(;;) {
		while (cb->S == cb->E) {
			/* buffer circolare vuoto */
			usleep(100000);
		}
		/* gestione finestra*/
		send_pkt(sockfd, cb->buf[cb->S]);
		/* controllo pacchetti in arrivo (se ack):
		 * if (rcvfrom (...) != 0) {//estrai seqnum da ack} */
		
	}
}

