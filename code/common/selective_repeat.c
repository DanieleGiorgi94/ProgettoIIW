#include "reliable_udp.h"

unsigned int N; // Ampiezza della finestra
unsigned int base; // Piu' vecchio pacchetto che non ha ricevuto ack
unsigned int nextseqnum; // numero di sequenza del prossimo pachetto da inviare

/**********
Bisogna implementare un modo per ricavare il numero di sequenza
dal pacchetto ricevuto. Inoltre bisogna implementare un modo per
mettere il numero di sequenza nel pacchetto da inviare.

Bisogna implementare il buffer di invio e quello di ricezione
per riordinare i pacchetti che arrivano.
**********/

void send_pkt(char *sndpkt)
{
	// Creazione socket udp	
	if ((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
		perror("errore in socket()");
		exit(-1);
	}

	// Invio pacchetto con udp
	if (sendto(sockfd,NULL,0,0,indirizzoserver,sizeof(indirizzoserver)) < 0){
		perror("errore in sendto()");
		exit(-1);
	}
    	printf("sending pkt...\n");
}

/*
L'idea puo' essere quella di un buffer in cui vengono inseriti i pacchetti
da inviare. Se e' non vuoto, viene estratto il pacchetto dal buffer e poi
viene inviato un segnale al processo che gestisce il selective repeat, il
quale estrae il pacchetto dal buffer e lo invia rispondendo all'evento.
*/

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
	
	

}

