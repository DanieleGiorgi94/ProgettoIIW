#ifndef _RELIABLE_UDP_H_
#error You must not include this sub-header file directly
#endif

#define BUFFER_SIZE 400
#define WINDOW_SIZE 100
#define TIMEOUT     2000000
#define LOSS_PROB   0

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
	struct buf_node cb_node[BUFFER_SIZE];
};
