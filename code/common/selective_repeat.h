#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#define BUFFER_SIZE 2048
#define WINDOW_SIZE 1024
#define TIMEOUT     3000000
#define LOSS_PROB   20

#define PORT   5193

struct comm_file_thread {
    int fd;
    struct circular_buffer *cb;
    pthread_t tid;
};

struct comm_thread {
    int sockfd;
    struct circular_buffer *cb;
    struct sockaddr *servaddr;
    unsigned long *timeout;
    pthread_t tid;
};

struct buf_node {
	pkt_t pkt;
    union {
	    char acked; // ack received (sending)
        char busy; //node busy (receiving)
    };
	clock_t timer; // timer (sending)
};

struct circular_buffer {
	u32 E; // buffer's end index (end)
	u32 S; // buffer's start index and window's base (start/base)
	u32 N; // pkt to send index (nextseqnum)
	pthread_mutex_t mtx;
	char end_transmission; // flag 
	struct buf_node cb_node[BUFFER_SIZE];
};
