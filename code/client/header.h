#ifndef _HEADER_H_
#define _HEADER_H_

#define TEST 0

#include "../common/reliable_udp.h"
#include "list_command/filelist_request.h"
#include "get_command/get_request.h"
#include "put_command/put_request.h"
#include "connection/connection_handler.h"

#define WELCOME_STRING "\nWelcome to our server! IP: %s PORT: %d\n"
#define SPACER "+-----------------------------------------" \
               "-----------------------------------------+\n"
#define FIRST_LINE "|ELENCO  COMANDI:                         " \
                   "                                         |"
#define LIST_LINE "| 1) list: elenco dei file presenti nel Server" \
                  "                                     |\n"
#define GET_LINE "| 2) get <Filename>: Download del file" \
                 "                                             |\n"
#define PUT_LINE "| 3) put <Filename>: Upload del file" \
                 "                                               |\n"
#define EXIT_LINE "| 4) exit                                 " \
                  "                                         |\n"

#endif
