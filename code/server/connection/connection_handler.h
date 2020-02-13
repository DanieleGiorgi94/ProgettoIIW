#ifndef _HEADER_H_
#error You must not include this sub-header directly
#endif

#define MAX_CONNECTIONS 10

struct service_thread {
    server_info *srv_info;
    pthread_t tid;
};

struct available_ports {
    int port_numbers[MAX_CONNECTIONS];
    char available[MAX_CONNECTIONS];
};
