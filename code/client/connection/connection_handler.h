#ifndef _HEADER_H_
#error You must not include this sub-header directly
#endif

int create_connection(int, struct sockaddr_in, char *cmd, char *token,
        char *connected, u64 *server_isn);

int close_connection(int, struct sockaddr_in);

void send_request(char *cmd, char *token, request_t *req, int sockfd, struct sockaddr_in servaddr);
