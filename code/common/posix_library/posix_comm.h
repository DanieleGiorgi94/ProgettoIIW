#ifndef __POSIX_LIBRARY_H_
#error You must not include this sub-header file directly
#endif

/*******ALLOCAZIONE DINAMICA*******/
/* Fa malloc() e memset() per allocare e pulire la memoria */
void *dynamic_allocation(size_t);
/* Fa solamente la free */
void free_allocation(void *);

/*******STAMPA ERRORI + TERMINAZIONE PROGRAMMA*******/
/*
    Passare come primo parametro la stringa che si vuole stampare e al secondo
    la variabile di cui si vuole controllare se vi è stato un errore: funziona
    solo per funzioni che restituiscono NULL o -1 in caso di errore. Esempio:
        print_error("malloc()", memset(buf, 0, size));
    esegue la memset() e, in caso di errore, stampa il messaggio e fa la exit().
*/
void print_error(char *, void *);

/*******EXECVE*******/
/*
    Primo argomento deve essere un array del seguente tipo:
        char * const argv[] = {"/bin/ls", "-l", "."};
    Il secondo argomento è il numero di argomenti del comando (per l'esempio di
    prima sarebbe 2).
    Il terzo argomento è il path di un file qualunque (eventualmente anche
    STDOUT_FILENO, ...).
*/
int execute_command_redirecting_output(char **, int, char *);
int execute_command(char **, int); 

/*******GESTIONE FILE*******/
/*
    Primo argomento: file descriptor
    Secondo argomento: puntatore ai byte da scrivere
    Terzo argomento: quantità byte da scrivere
*/
unsigned long write_block(int, void *, unsigned long);
/* Simile al precedente; restituisce il numero di byte letti */
unsigned long read_block(int, void *, unsigned long);
/*
    Primo argomento: pathname
    Secondo argomento: flags
*/
int open_file(char *, int);
void close_file(int);
/*
    Primo argomento: file descriptor
    Secondo argomento: uno tra
        'E' (end) -> fine del file + offset
        'C' (current) -> posizione corrente + offset
        'S' (set) -> inizio del file + offset
    Terzo argomento: offset
*/
void move_offset(int, char, off_t);

/*******LISTA COLLEGATA*******/
/*
    Per creare una lista collegata passare un puntatore ad un node inizializzato
    e un doppio puntatore ad un node non inizializzato.

    Il nodo del primo parametro viene aggiunto dopo il nodo passato al secondo.
*/
void add_node(struct node *, struct node **);
struct node *get_node(struct node *, int);
struct node *get_last_node(struct node *);
struct node *remove_node(struct node **, int);
void free_linked_list(struct node **);

/*******CONVERSIONI*******/
/* Utile per conversione di un argomento numerico passato da riga di comando */
int string_to_int(char *);

/*******SLEEP*******/
void sleep_for(unsigned int);   //secondi
void usleep_for(useconds_t);    //microsecondi
void nanosleep_for(long);       //nanosecondi

/*******SEGNALI*******/
/* PID del processo a cui si vuole inviare il segnale e numero di segnale */
void send_signal(pid_t, int);
