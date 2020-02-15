#ifndef _RELIABLE_UDP_H_
#define _RELIABLE_UDP_H_

typedef unsigned int u32;
typedef unsigned long u64;

#define BUFLEN 512

#include "posix_library/posix_library.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sched.h>
#include "udp.h"
#include "comm.h"
#include "selective_repeat.h"
#include "connection.h"

#endif
