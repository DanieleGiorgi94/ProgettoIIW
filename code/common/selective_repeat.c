#include "reliable_udp.h"

static void create_mutex(pthread_mutex_t *);
static void destroy_mutex(pthread_mutex_t *);
static void lock_buffer(struct circular_buffer *);
static void unlock_buffer(struct circular_buffer *);

static void *split_file(void *);
static void *sender(void *);
static void *receive_ack(void *);
static void send_pkt(int, pkt_t *, const struct sockaddr *);
static pkt_t create_pkt(int, u64);

static void *merge_file(void *);
static void *receiver(void *);
static void send_ack(int, struct sockaddr, u64, char type);
static char sorted_buf_insertion(struct circular_buffer *, struct buf_node,
                                                                        u64);

//  ************ MUTEX ************
static void create_mutex(pthread_mutex_t *mtx) {
    if (pthread_mutex_init(mtx, NULL) != 0) {
        perror("pthread_mutex_init() failed");
        exit(EXIT_FAILURE);
    }
}
static void destroy_mutex(pthread_mutex_t *mtx) {
    pthread_mutex_destroy(mtx);
//    if (pthread_mutex_destroy(mtx) != 0) {
//        perror("pthread_mutex_destroy() failed");
//        exit(EXIT_FAILURE);
//    }
}
static void lock_buffer(struct circular_buffer *cb) {
    if (pthread_mutex_lock(&cb->mtx) != 0){
        perror("Errore in pthread_mutex_lock()\n");
        exit(EXIT_FAILURE);
    }
}
static void unlock_buffer(struct circular_buffer *cb) {
    if (pthread_mutex_unlock(&cb->mtx) != 0){
        perror("Errore in pthread_mutex_lock()\n");
        exit(EXIT_FAILURE);
    }
}

//  ************ SENDER ************
static void send_pkt(int sockfd, pkt_t *pkt, const struct sockaddr *servaddr) {
    if (sendto(sockfd, pkt, sizeof(pkt_t), 0, (struct sockaddr *) servaddr,
           sizeof(struct sockaddr)) < 0) {
        perror("Errore in sendto()");
        exit(EXIT_FAILURE);
    }
    printf("pkt %ld inviato\n", pkt->header.n_seq);
}
static pkt_t create_pkt(int fd, u64 nseq) {
    char buff[MAX_PAYLOAD_SIZE];
    pkt_t pkt;

    //Legge dal file e crea pacchetti di dim MAX_PAYLOAD_SIZE
    u64 read_byte = read_block(fd, buff, MAX_PAYLOAD_SIZE);

    if (read_byte == 0) {
        header_t header;
        header.n_seq = nseq;
        header.length = (u32) read_byte;
        header.rwnd = 0;
        header.type = END_OF_FILE;

        pkt.header = header;
    } else {
        header_t header;
        header.n_seq = nseq;
        header.length = (u32) read_byte;
        header.rwnd = 0;
        header.type = NORM_PKT;

        pkt.header = header;
        strncpy(pkt.payload, buff, read_byte);
    }

    return pkt;
}
static void *receive_ack(void *arg) {
    struct comm_thread *ptd = (struct comm_thread *) arg;

    int sockfd = ptd->sockfd;
    struct sockaddr *servaddr = ptd->servaddr;
    struct circular_buffer *cb = ptd->cb;

    ack_t *ack = (ack_t *) dynamic_allocation(sizeof(ack_t));
    u32 slen = sizeof(struct sockaddr);
    u64 I, i, index;

    while(1) {
        //waiting for ACK
        printf("attendo ACK\n");
        while (recvfrom(sockfd, (void *) ack, sizeof(ack_t), MSG_DONTWAIT,
                                                    servaddr, &slen) < 0) {
            //printf("attendo ACK\n");
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Errore in recvfrom: ricezione dell'ack");
            	exit(EXIT_FAILURE);
            }
        }
 
        //printf("sender attempt lock in receive_ack\n");
        lock_buffer(cb);
        //printf("sender lock in receive_ack\n");

        i = ack->n_seq % BUFFER_SIZE;
        index = i;
        //if cb->S > cb->E, then:
        //  ---------------------------------
        //  |   |   | E |   |   |   | S |   |
        //  ---------------------------------
        //so consider
        //  ----------------------------------------------
        //  |   |   | E |   |   |   | S |   ||   |   | I |
        //  ----------------------------------------------
        //else consider
        //  ---------------------------------
        //  |   |   | I |   |   |   | S |   |
        //  ---------------------------------
        if (cb->S > cb->E){
            I = cb->E + BUFFER_SIZE;
            if (i < cb->E)
                index = i + BUFFER_SIZE;
        } else {
            I = cb->E;
            index = i;
        }

        //TODO: cosa succede se i è minore di E ma E è minore di S? se non
        //sbaglio la condizione seguente non funziona più
        //if 'i' falls between 'S' and 'I', ACK refers to a pkt in the window
        if (cb->S <= index && index < I){
            cb->cb_node[index].acked = 1;
            printf("ack %ld ricevuto\n", ack->n_seq);
        }

        //move window's base to the first non-acked pkt of the window
        while (cb->cb_node[cb->S].acked == 1){
            if (cb->S == cb->E) break;
            cb->S = (cb->S + 1) % BUFFER_SIZE;
        }

        if (ack->type == END_OF_FILE) {
            //TODO: return something special
            unlock_buffer(cb);
            return NULL;
        }

        //printf("sender unlock in receive_ack\n");
        unlock_buffer(cb);
    }
    return NULL;
}
static void *sender(void *arg) {
    struct comm_thread *ptd = arg;

    struct circular_buffer *cb = ptd->cb;
    int sockfd = ptd->sockfd;
    struct sockaddr *servaddr = ptd->servaddr;

    for(;;) {
        //printf("%d, %d, %d\n", cb->S, cb->N, cb->E);

        //printf("sender attempt lock\n");
        lock_buffer(cb);
        //printf("sender lock\n");

        while (cb->S == cb->E) {
            /* empty circular buffer (no pkts to send) */
            //printf("sender unlock\n");
            unlock_buffer(cb);
            usleep_for(100);
            //printf("sender attempt lock\n");
            lock_buffer(cb);
            //printf("sender lock\n");
        }

        //check receive_ack() function to understand why if this condition is
        //met then window's not full
        if (cb->N + BUFFER_SIZE * (cb->S > cb->N) - cb->S <= WINDOW_SIZE) {
            //printf("window's not full\n");
            if (cb->N != cb->E) {
                // nextseqnum must not overpass cb->E
                pkt_t pkt = cb->cb_node[cb->N].pkt;
                //if (pkt.header.type == END_OF_FILE) {
                //    send_pkt(sockfd, &pkt, servaddr);
                //    return NULL;
                //}
                send_pkt(sockfd, &pkt, servaddr);
                cb->cb_node[cb->N].timer = clock(); // start timer
                cb->N = (cb->N + 1) % BUFFER_SIZE;
            }
        }
        //printf("sender unlock\n");
        unlock_buffer(cb);
    }
    return NULL;
}
static void *split_file(void *arg) {
    struct comm_file_thread *ptd = arg;
    struct circular_buffer *cb = ptd->cb;
    int fd = ptd->fd;

    move_offset(fd, SET, 0);
    
    for (u64 i = 0; 1; i++) {
        pkt_t pkt = create_pkt(fd, i);

        u32 nE;

        //printf("pkt %ld created\n", i);

        lock_buffer(cb);

        nE = (cb->E + 1) % BUFFER_SIZE;
        while (nE == cb->S) {
        	/* circular buffer's full */
        	unlock_buffer(cb);
        	usleep(100000);
        	lock_buffer(cb);
        }

        struct buf_node cbn;
        cbn.pkt = pkt;

        cbn.acked = 0;
        cb->cb_node[cb->E] = cbn;
        cb->E = nE;

        if (pkt.header.type == END_OF_FILE) {
            unlock_buffer(cb);
            return NULL;
        }

        unlock_buffer(cb);
    }

    return NULL;
}
void send_file(int sockfd, struct sockaddr *servaddr, int fd) {
    struct comm_file_thread sf_thread;
    struct comm_thread snd_thread, rca_thread;
    struct circular_buffer *cb;

    cb = (struct circular_buffer *)
        dynamic_allocation(sizeof(struct circular_buffer));
    cb->E = 0;
    cb->S = 0;
    cb->N = 0;

    create_mutex(&(cb->mtx));

    sf_thread.cb = cb;
    sf_thread.fd = fd;

    snd_thread.sockfd = sockfd;
    snd_thread.cb = cb;
    snd_thread.servaddr = servaddr;

    rca_thread.sockfd = sockfd;
    rca_thread.cb = cb;
    rca_thread.servaddr = servaddr;

    if ((pthread_create(&sf_thread.tid, NULL, split_file, &sf_thread) ||
            pthread_create(&snd_thread.tid, NULL, sender, &snd_thread) ||
            pthread_create(&rca_thread.tid, NULL, receive_ack, &rca_thread))
                                                                        != 0) {
        perror("pthread_create() failed");
        exit(EXIT_FAILURE);
    }

    if ((pthread_join(sf_thread.tid, NULL) ||
            pthread_join(snd_thread.tid, NULL)) != 0) {
        destroy_mutex(&(cb->mtx));
        free_allocation(cb);
        perror("Errore in pthread_join()");
        exit(EXIT_FAILURE);
    }

    destroy_mutex(&(cb->mtx));
    free_allocation(cb);
}

//  ************ RECEIVER ************
static char sorted_buf_insertion(struct circular_buffer *cb,
                                        struct buf_node cbn, u64 seqnum) {
//TODO: i pacchetti non vengono mai scartati... non restituisce mai 0
    /* returns 1 when pkt is accepted, 0 when refused */
    u64 i = seqnum % BUFFER_SIZE;

    /* refuse pkt if node's busy */
    if ((cb->cb_node[i].busy == 1) || 
        (i == (cb->S + BUFFER_SIZE - 1) % BUFFER_SIZE)) {
            printf("Scarto pacchetto %ld\n", seqnum);
            return 1; // sends ACK anyway
    } 

    cb->cb_node[i] = cbn;
    printf("inserisco pacchetto %ld\n", cbn.pkt.header.n_seq);
    //printf("Inserito in posizione %d\n", i);

    if (cb->S <= cb->E) {
        if (i > cb->E) cb->E = (i + 1) % BUFFER_SIZE;
    } else {
        if (i > cb->E && i < cb->S - 1) cb->E = (i + 1) % BUFFER_SIZE;
    }

    return 1;
}
static void send_ack(int sockfd, struct sockaddr servaddr, u64 seqnum,
                                                        char type) {
    ack_t *ack = (ack_t *) dynamic_allocation(sizeof(ack_t));
    ack->n_seq = seqnum;
    ack->type = type;

    printf("sending ack %ld\n", seqnum);
    /* sends ACK */
    if (sendto(sockfd, (void *)ack, sizeof(ack_t), 0, &servaddr,
            sizeof(servaddr)) < 0) {
        free_allocation(ack);
        perror("Errore in sendto: invio dell'ack");
        exit(EXIT_FAILURE); 
    }

    free_allocation(ack);
}
static void *receiver(void *arg) {
    struct comm_thread *ptd = arg;

    struct circular_buffer *cb = ptd->cb;
    int sockfd = ptd->sockfd;
    const struct sockaddr *servaddr = ptd->servaddr;

    unsigned int slen = sizeof(struct sockaddr);
    u64 seqnum;
    u32 nE;
    pkt_t *pkt;

    for(;;) {
    	pkt = (pkt_t *) dynamic_allocation(sizeof(pkt_t));
        header_t pkt_header = pkt->header;

        printf("Attendo pacchetto\n");
    	while (recvfrom(sockfd, (void *) pkt, sizeof(pkt_t), 0,
                (struct sockaddr *) servaddr, &slen) < 0) {
            //printf("Attendo pacchetto\n");
            if (errno != EAGAIN) {
                perror("Errore in recvfrom()");
                exit(EXIT_FAILURE);
            }
    	}

    	struct buf_node cbn;
    	cbn.pkt = *pkt;
    	cbn.busy = 1;
    	seqnum = pkt->header.n_seq;

        free_allocation(pkt);

    	lock_buffer(cb);
        //printf("receiver lock\n");

    	nE = (cb->E + 1) % BUFFER_SIZE;
    	while (nE == cb->S){
            /* circular buffer's full */
            //printf("receiver unlock\n");
            unlock_buffer(cb);
            usleep(100000);
            lock_buffer(cb);
            //printf("receiver lock\n");
    	}	

    	printf("Ricevuto pkt %ld\n", seqnum);

        // inserimento ordinato del pacchetto ricevuto
        if (sorted_buf_insertion(cb, cbn, seqnum)) {
            // invio ack
            //printf("receiver unlock\n");
            unlock_buffer(cb);
            send_ack(sockfd, *servaddr, seqnum, pkt_header.type);
            lock_buffer(cb);
            //printf("receiver lock\n");
        }
        //printf("receiver unlock\n");
        unlock_buffer(cb);
    }
}
static void *merge_file(void *arg) {
    struct comm_file_thread *ptd = arg;

    struct circular_buffer *cb = ptd->cb;
    int fd = ptd->fd;

    u64 written_byte;

    for(;;) {
        lock_buffer(cb);
        //printf("merge file lock\n");

        while (cb->S == cb->E){
        /* circular buffer's empty */
            //printf("merge file unlock\n");
            unlock_buffer(cb);
            usleep(100000);
            lock_buffer(cb);
            //printf("merge file lock\n");
        }

        //stampa tutto il buffer circolare e poi termina
        //for (int i = 0; i < BUFFER_SIZE; i++) {
        //    printf("%p, %d\n", &cb->cb_node[i].pkt, cb->cb_node[i].acked);
        //}
        //return NULL;

        //starting from the window's base, write all the pkts to file if
        //circular buffer's node is busy
        while (cb->cb_node[cb->S].busy == 1) {
            pkt_t pkt = cb->cb_node[cb->S].pkt;
            cb->cb_node[cb->S].busy = 0;

            //printf("Reading pkt %ld from cb\n", pkt.header.n_seq);

            if (pkt.header.type == END_OF_FILE) {
                printf("------------------------------------");
                printf("merge_file terminated!");
                printf("------------------------------------\n");
                return NULL;
            }
            written_byte = write_block(fd, pkt.payload, pkt.header.length);

            cb->S = (cb->S + 1) % BUFFER_SIZE; 
        }
        //printf("merge file unlock\n");
        unlock_buffer(cb);
    }
}
void receive_file(int sockfd, struct sockaddr *servaddr, int fd) {

    struct comm_file_thread mf_thread;
    struct comm_thread rcv_thread;
    struct circular_buffer *cb;

    cb = (struct circular_buffer *)
        dynamic_allocation(sizeof(struct circular_buffer));
    cb->S = 0;
    cb->E = 0;
    cb->N = 0;

    create_mutex(&(cb->mtx));

    mf_thread.cb = cb;
    mf_thread.fd = fd;

    rcv_thread.sockfd = sockfd;
    rcv_thread.cb = cb;
    rcv_thread.servaddr = servaddr;

    if ((pthread_create(&mf_thread.tid, NULL, merge_file, &mf_thread) ||
        pthread_create(&rcv_thread.tid, NULL, receiver, &rcv_thread)) != 0) {
        perror("pthread_create() failed");
        exit(EXIT_FAILURE);
    }

    if ((pthread_join(mf_thread.tid, NULL) ||
            pthread_join(rcv_thread.tid, NULL)) != 0) {
        destroy_mutex(&(cb->mtx));
        free_allocation(cb);
        perror("Errore in pthread_join()");
        exit(EXIT_FAILURE);
    }

    destroy_mutex(&(cb->mtx));
    free_allocation(cb);
}
