#include "reliable_udp.h"

static void create_mutex(pthread_mutex_t *);
static void destroy_mutex(pthread_mutex_t *);
static void lock_buffer(struct circular_buffer *);
static void unlock_buffer(struct circular_buffer *);

static void *split_file(void *);
static void *sender(void *);
static void *receive_ack(void *);
static void send_pkt(int, pkt_t *, const struct sockaddr *);
static char sym_lost_pkt(void);
static void *timeout_handler(void *);
static pkt_t create_pkt(int, u64);

static void *merge_file(void *);
static void *receiver(void *);
static void send_ack(int, struct sockaddr, u64, char type);
static char sorted_buf_insertion(struct circular_buffer *, struct buf_node,
                                 u64);

static unsigned long estimateTimeout(long *, long *, long);

//  ************ TIMEOUT **********
#define ALPHA 0.125
#define BETA 0.25

static unsigned long estimateTimeout(long *EstimatedRTT, long *DevRTT,
        long SampleRTT)
{
    *EstimatedRTT = (1 - ALPHA) * (*EstimatedRTT) + ALPHA * SampleRTT;
    *DevRTT = (1 - BETA) * (*DevRTT) + BETA * labs(SampleRTT - *EstimatedRTT);
    long timeoutInterval = (*EstimatedRTT + 4 * (*DevRTT));

    return timeoutInterval;
}

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
    if (pthread_mutex_lock(&cb->mtx) != 0) {
        perror("Errore in pthread_mutex_lock()\n");
        exit(EXIT_FAILURE);
    }
}
static void unlock_buffer(struct circular_buffer *cb) {
    if (pthread_mutex_unlock(&cb->mtx) != 0) {
        perror("Errore in pthread_mutex_lock()\n");
        exit(EXIT_FAILURE);
    }
}

//  ************ SENDER ************
static char sym_lost_pkt(void) {
    /* simula perdita pacchetti con probabilita' LOSS_PROB */
    int i = rand() % 101; // genera numero da 1 a 100
    return i <= LOSS_PROB;
}
static void send_pkt(int sockfd, pkt_t *pkt, const struct sockaddr *servaddr) {
    if (!sym_lost_pkt()) {
        if (sendto(sockfd, pkt, sizeof(pkt_t), 0, (struct sockaddr *) servaddr,
                   sizeof(struct sockaddr)) < 0) {
            perror("Errore in sendto()");
            exit(EXIT_FAILURE);
        }
//        printf("pkt %ld inviato\n", pkt->header.n_seq);
//    } else {
//        printf("pkt %ld perduto\n", pkt->header.n_seq);
    }
}
static void *timeout_handler(void *arg) {
    struct comm_thread *ptd = arg;

    int sockfd = ptd->sockfd;
    struct circular_buffer *cb = ptd->cb;
    struct sockaddr *servaddr = ptd->servaddr;
    unsigned long *timeout = ptd->timeout;

    u32 I;
    clock_t tspan;
    pkt_t pkt;

    while(1) {
        lock_buffer(cb);
        while (cb->S == cb->E){
            /* buffer circolare vuoto */
            unlock_buffer(cb);
            usleep(1000000);
            lock_buffer(cb);
        }

        //check receive_ack() function to understand these next instructions
        if (cb->S > cb->N) {
            I = cb->N + BUFFER_SIZE;
        } else {
            I = cb->N;
        }

        //check all pkts in the window and re-send non-acked pkts whose timer
        //expired
        for (u32 i = cb->S; i < I; i++) {
            if (cb->cb_node[i % BUFFER_SIZE].acked == 0) { //<--- non-acked pkts
                tspan = clock() - cb->cb_node[i % BUFFER_SIZE].timer;
                if (ADAPTIVE) {
                    if (tspan >= *timeout) { //<--- timer expired pkts
                        pkt = cb->cb_node[i % BUFFER_SIZE].pkt;
                        send_pkt(sockfd, &pkt, servaddr);
                        cb->cb_node[i % BUFFER_SIZE].pkt.header.retransmitted = 1;
                        cb->cb_node[i % BUFFER_SIZE].timer = clock();
                        //printf("inviato per timeout\n");
                    }
                } else { //not adaptive
                    if (tspan >= TIMEOUT) { //<--- timer expired pkts
                        pkt = cb->cb_node[i % BUFFER_SIZE].pkt;
                        send_pkt(sockfd, &pkt, servaddr);
                        cb->cb_node[i % BUFFER_SIZE].timer = clock();
                    }
                }
            }
        }
        unlock_buffer(cb);
    }
    return NULL;
}
static pkt_t create_pkt(int fd, u64 nseq) {
    char buff[MAX_PAYLOAD_SIZE];
    pkt_t pkt;

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
    struct circular_buffer *cb = ptd->cb;
    struct sockaddr *servaddr = ptd->servaddr;
    unsigned long *timeout = ptd->timeout;

    //char *filepath = obtain_path(cmd, token, 0);
    ack_t *ack = (ack_t *) dynamic_allocation(sizeof(ack_t));
    u32 slen = sizeof(struct sockaddr);
    u64 I, i, index;
    long estimatedRTT = 0;
    long devRTT = 0;

    while(1) {
        //waiting for ACK
//        printf("attendo ACK\n");
        while (recvfrom(sockfd, (void *) ack, sizeof(ack_t), MSG_DONTWAIT,
                        servaddr, &slen) < 0) {
//            printf("attendo ACK\n");
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom() (ricezione dell'ack)");
                exit(EXIT_FAILURE);
            }
        }

//        printf("sender attempt lock in receive_ack\n");
        lock_buffer(cb);
//        printf("sender lock in receive_ack\n");

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
        //  |   |   | S |   |   |   | E |   |
        //  ---------------------------------
        if (cb->S > cb->E){
            I = cb->E + BUFFER_SIZE;
            if (i < cb->E)
                index = i + BUFFER_SIZE;
        } else {
            I = cb->E;
            index = i;
        }

        //if 'i' falls between 'S' and 'I', ACK refers to a pkt in the window
        if (cb->S <= index && index < I){
            cb->cb_node[index].acked = 1;

            // Timeout non viene mai calcolato per segmenti ritrasmessi
            printf("%d\b", cb->cb_node[i % BUFFER_SIZE].pkt.header.retransmitted);
            if (cb->cb_node[i % BUFFER_SIZE].pkt.header.retransmitted == 0) {
                printf("est: %li dev: %li\n", estimatedRTT, devRTT);
                printf("timeout: %lu\n", *timeout);
                *timeout = estimateTimeout(&estimatedRTT, &devRTT,
                                          cb->cb_node[index].timer);
            }
//            printf("ack %ld ricevuto\n", ack->n_seq);
        }

        //move window's base to the first non-acked pkt of the window
        while (cb->cb_node[cb->S].acked == 1){
            if (cb->S == cb->E) break;
            cb->S = (cb->S + 1) % BUFFER_SIZE;
        }

        if (ack->type == END_OF_FILE) {
            //TODO: return something special
	    cb->end_transmission = 1;
            unlock_buffer(cb);
            return NULL;
        }

//        printf("sender unlock in receive_ack\n");
        unlock_buffer(cb);
    }
    return NULL;
}
static void *sender(void *arg) {
    struct comm_thread *ptd = arg;

    int sockfd = ptd->sockfd;
    struct circular_buffer *cb = ptd->cb;
    struct sockaddr *servaddr = ptd->servaddr;

    for(;;) {
//        printf("sender attempt lock\n");
        lock_buffer(cb);
//        printf("sender lock\n");

        while (cb->S == cb->E) {
            /* empty circular buffer (no pkts to send) */
//            printf("sender unlock\n");
            unlock_buffer(cb);
            usleep_for(100);
//            printf("sender attempt lock\n");
            lock_buffer(cb);
//            printf("sender lock\n");
        }

        //check receive_ack() function to understand why if this condition is
        //met then window's not full
        if (cb->N + BUFFER_SIZE * (cb->S > cb->N) - cb->S <= WINDOW_SIZE) {
//            printf("window's not full\n");
            if (cb->N != cb->E) {
                //nextseqnum must not overpass cb->E
                pkt_t pkt = cb->cb_node[cb->N].pkt;
                send_pkt(sockfd, &pkt, servaddr);
                cb->cb_node[cb->N].timer = clock(); //start timer
                cb->N = (cb->N + 1) % BUFFER_SIZE;
            }
        }
	if (cb->end_transmission == 1) return NULL;
//        printf("sender unlock\n");
        unlock_buffer(cb);
    }
    return NULL;
}
static void *split_file(void *arg) {
    struct comm_file_thread *ptd = arg;

    int fd = ptd->fd;
    struct circular_buffer *cb = ptd->cb;

    move_offset(fd, SET, 0);

    for (u64 i = 0; 1; i++) {
        pkt_t pkt = create_pkt(fd, i);

        u32 nE;

//        printf("pkt %ld created\n", i);

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
    struct comm_thread snd_thread, rca_thread, tmh_thread;
    struct circular_buffer *cb;
    unsigned long *timeout = (unsigned long *)
        dynamic_allocation(sizeof(*timeout));
    *timeout = TIMEOUT;

    cb = (struct circular_buffer *)
            dynamic_allocation(sizeof(struct circular_buffer));
    cb->E = 0;
    cb->S = 0;
    cb->N = 0;
    cb->end_transmission = 0;

    create_mutex(&(cb->mtx));

    //split_file's thread
    sf_thread.cb = cb;
    sf_thread.fd = fd;

    //sender's thread
    snd_thread.sockfd = sockfd;
    snd_thread.cb = cb;
    snd_thread.servaddr = servaddr;

    //receive_ack's thread
    rca_thread.sockfd = sockfd;
    rca_thread.cb = cb;
    rca_thread.servaddr = servaddr;
    rca_thread.timeout = timeout;

    //timeout_handler's thread
    tmh_thread.sockfd = sockfd;
    tmh_thread.cb = cb;
    tmh_thread.servaddr = servaddr;
    tmh_thread.timeout = timeout;

    if ((pthread_create(&sf_thread.tid, NULL, split_file, &sf_thread) ||
         pthread_create(&snd_thread.tid, NULL, sender, &snd_thread) ||
         pthread_create(&rca_thread.tid, NULL, receive_ack, &rca_thread) ||
         pthread_create(&tmh_thread.tid, NULL, timeout_handler, &tmh_thread))
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
    /* returns 1 when pkt is accepted, 0 when refused */
    u64 i = seqnum % BUFFER_SIZE;

    /* refuse pkt if node's busy */
    if (cb->cb_node[i].busy == 1) {
//            printf("Scarto pacchetto %ld\n", seqnum);
        return 0;
    }

    cb->cb_node[i] = cbn;
//    printf("inserisco pacchetto %ld\n", cbn.pkt.header.n_seq);
//    printf("Inserito in posizione %d\n", i);

    if (i > cb->E + (cb->S > cb->E) * BUFFER_SIZE)
        cb->E = i;

    return 1;
}
static void send_ack(int sockfd, struct sockaddr servaddr, u64 seqnum,
                     char type) {
    ack_t *ack = (ack_t *) dynamic_allocation(sizeof(ack_t));
    ack->n_seq = seqnum;
    ack->type = type;

//    printf("sending ack %ld\n", seqnum);
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
    pkt_t *pkt;

    for(;;) {
        pkt = (pkt_t *) dynamic_allocation(sizeof(pkt_t));
        header_t pkt_header = pkt->header;

//        printf("Attendo pacchetto\n");
        while (recvfrom(sockfd, (void *) pkt, sizeof(pkt_t), MSG_DONTWAIT,
                        (struct sockaddr *) servaddr, &slen) < 0) {
//            printf("Attendo pacchetto\n");
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Errore in recvfrom()");
                exit(EXIT_FAILURE);
            }
        }

        struct buf_node cbn;
        cbn.pkt = *pkt;
        cbn.busy = 1;
        seqnum = pkt->header.n_seq;

        free_allocation(pkt);

//        printf("receiver lock attempt\n");
        lock_buffer(cb);
//        printf("receiver lock\n");

//    	  printf("Ricevuto pkt %ld\n", seqnum);

        // inserimento ordinato del pacchetto ricevuto
        if (sorted_buf_insertion(cb, cbn, seqnum)) {
            // invio ack
//            printf("receiver unlock\n");
            unlock_buffer(cb);
            send_ack(sockfd, *servaddr, seqnum, pkt_header.type);
//            printf("receiver lock attempt\n");
            lock_buffer(cb);
//            printf("receiver lock\n");
        }
//        printf("receiver unlock\n");
	if (cb->end_transmission == 1) return NULL;  
        unlock_buffer(cb);
    }
}
static void *merge_file(void *arg) {
    struct comm_file_thread *ptd = arg;

    struct circular_buffer *cb = ptd->cb;
    int fd = ptd->fd;

    for(;;) {
//        printf("merge file lock attempt\n");
        lock_buffer(cb);
//        printf("merge file lock\n");

        while (cb->S == cb->E){
            /* circular buffer's empty */
//            printf("merge file unlock\n");
            unlock_buffer(cb);
            usleep(100000);
//            printf("merge file lock attempt\n");
            lock_buffer(cb);
//            printf("merge file lock\n");
        }

        //starting from the window's base, write all the pkts to file if
        //circular buffer's node is busy
        while (cb->cb_node[cb->S].busy == 1) {
            pkt_t pkt = cb->cb_node[cb->S].pkt;
            cb->cb_node[cb->S].busy = 0;

//            printf("Reading pkt %ld from cb\n", pkt.header.n_seq);

            if (pkt.header.type == END_OF_FILE) {
                //printf("------------------------------------");
                //printf("merge_file terminated!");
                //printf("------------------------------------\n");
                cb->end_transmission = 1;
		unlock_buffer(cb);
		return NULL;
            }
            write_block(fd, pkt.payload, pkt.header.length);

            cb->S = (cb->S + 1) % BUFFER_SIZE;
        }
//        printf("merge file unlock\n");
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
    cb->end_transmission = 0;

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
