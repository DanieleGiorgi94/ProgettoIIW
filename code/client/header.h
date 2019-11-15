#ifndef _HEADER_H_
#define _HEADER_H_

#include "../common/reliable_udp.h"
#include "list_command/filelist_request.h"
#include "get_command/get_request.h"
#include "put_command/put_request.h"
#include "connection/connection_handler.h"

#define SEPARATION_LINE "+----------------------------------------" \
                        "----------------------------------------+"
#define FIRST_LINE      "|ELENCO COMANDI:                         " \
                        "                                        |"
#define LIST_LINE       "| 1) list: elenco dei file presenti nel s" \
                        "erver                                   |"
#define GET_LINE        "| 2) get <filename>: download del file   " \
                        "                                        |"
#define PUT_LINE        "| 3) put <filename>: upload del file     " \
                        "                                        |"
#define EXIT_LINE       "| 4) exit                                " \
                        "                                        |"
#define ENTER_LINE      "ENTER MESSAGE:"

char **tokenize_string(char *, char *);
void print_banner(void);

#endif
