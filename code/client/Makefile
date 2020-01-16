CC=gcc
CFLAGS=-Wall -Wextra -O2 -pthread
CFILES=$(shell ls *.c)
LIBCFILES=$(shell ls ../common/posix_library/*.c)
COMMCFILES=$(shell ls ../common/*.c)
LISTC=$(shell ls list_command/*.c)
GETC=$(shell ls get_command/*.c)
PUTC=$(shell ls put_command/*.c)
CONNC=$(shell ls connection/*.c)
PROGS=$(CFILES:%.c=%)

# NOME: PREREQUISITO
# [TAB] COMANDO

all: $(PROGS)

%: %.c $(LIBCFILES) $(COMMCFILES) $(LISTC) $(GETC) $(PUTC) $(CONNC)
	$(CC) $(CFLAGS) -o $@ $^ 

clean:
	rm -f $(PROGS) *.o
