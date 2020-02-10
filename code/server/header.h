#ifndef _HEADER_H_
#define _HEADER_H_

#define TEST 0

#include "../common/reliable_udp.h"
#include "utilities/utilities.h"
#include "list_command/list_request.h"
#include "get_command/get_request.h"
#include "put_command/put_request.h"
#include "exit_command/exit_request.h"
#include "connection/connection_handler.h"

void *create_connection(void *);
void create_mtx(pthread_mutex_t *mtx);
void destroy_mtx(pthread_mutex_t *mtx);
void lock_mtx(pthread_mutex_t *mtx);
void unlock_mtx(pthread_mutex_t *mtx);

#endif
