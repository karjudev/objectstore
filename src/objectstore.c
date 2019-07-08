/**
 * @file client.c
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Implementazione del server object store.
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#define _POSIX_C_SOURCE 199506L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>

#include <assertmacros.h>
#include <shared.h>

#include <socket/safeio.h>
#include <socket/socket.h>
#include <workers/workers.h>
#include <pthread_list/pthread_list.h>

#include <shared.h>

// Variabile globale che indica la terminazione
static int terminated = 0;

/**
 * @brief Invia al client il messaggio 'KO <errno>'
 * 
 * @param client_fd File descriptor del client.
 */
void send_error (int client_fd) {
    // Inizializza il buffer che contiene l'errore
    char err_buffer[MAX_RESPONSE_LENGTH];
    memset(err_buffer, 0, MAX_RESPONSE_LENGTH);
    // Costruisce la stringa formattata
    sprintf(err_buffer, "KO %d \n", errno);
    // Scrive la stringa sul buffer
    int success = send_message(client_fd, err_buffer, MAX_RESPONSE_LENGTH);
    ASSERT_MESSAGE(success != -1, "Writing error message to client", return);
    // Stampa il messaggio anche sullo standard error
    fprintf(stderr, "[objectstore] Client %d: %s\n", client_fd, strerror(errno));
}

/**
 * @brief Stampa un report sullo standard output contenente client connessi, numero di oggetti e dimensione totale dello store
 */
void print_report () {
    // Numero di client connessi
    int clients = 0;
    // Numero di oggetti nello store
    int objects = 0;
    // Dimensione totale dello store
    int size = 0;
    // Recupera le informazioni necessarie
    int success = get_report(&clients, &objects, &size);
    ASSERT_MESSAGE(success != -1, "Retrieving client", return);
    // Stampa le informazioni
    printf("[objectstore] Connected clients: %d Object number: %d Total size: %d bytes\n", clients, objects, size);
}

/**
 * @brief Invia un messaggio di successo al client. Se non ci riesce gli manda un messaggio di errore.
 * 
 * @param client_fd File descriptor del client
 */
void send_ok (int client_fd) {
    // Crea la stringa con scritto ok
    char ok_string[MAX_RESPONSE_LENGTH] = "OK \n";
    int success = send_message(client_fd, ok_string, MAX_RESPONSE_LENGTH);
    ASSERT(success != -1, send_error(client_fd));
}

/**
 * @brief Registra un utente sul server
 * 
 * @param client_fd File descriptor del server
 * @param name Nome con cui registrarsi
 * @return int Se la registrazione è avvenuta con successo invia OK all'utente e restitusice 0. Se c'è un errore restituisce -1 e setta errno.
 */
int handle_registration (int client_fd, char* name) {
    // Registra l'utente nel sistema
    int success = register_user(client_fd, name);
    // Controlla che sia andato tutto bene
    ASSERT_RETURN(success == 0, -1);
    // Restituisce il successo
    send_ok(client_fd);
    return 0;
}

/**
 * @brief Rimuove dallo store un blocco
 * 
 * @param client_fd File descriptor del client
 * @param name Nome del blocco da rimuovere
 * @return int Se l'eliminazione è avvenuta con successo restituisce OK all'utente e restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int handle_deletion (int client_fd, char* name) {
    // Rimuove l'utente dal sistema
    int success = delete_block(client_fd, name);
    // Controlla che l'operazione sia avvenuta con successo
    ASSERT_RETURN(success == 0, -1);
    // Restituisce il successo
    send_ok(client_fd);
    return 0;
}

/**
 * @brief Memorizza un oggetto nello spazio dell'utente
 * 
 * @param client_fd File descriptor del client
 * @param name Nome dell'oggetto da memorizzare
 * @param length Dimensione dell'oggetto
 * @param data Puntatore ai dati da memorizzare
 * @return int Se la memorizzazione è avvenuta con successo manda OK al client e restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int handle_storing (int client_fd, char* name, size_t length, void* data) {
    int success = store_block(client_fd, name, data, length);
    ASSERT_RETURN(success != -1, -1);
    free(data);
    // Invia l'ok
    send_ok(client_fd);
    return 0;
}

/**
 * @brief Recupera un blocco di dati dell'utente identificato dal nome
 * 
 * @param client_fd File descriptor dell'utente
 * @param name Nome del blocco da reperire
 * @return int Se l'oggetto è stato ritrovato con successo invia OK al client e restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int handle_retrieving (int client_fd, char* name) {
    // Alloca l'header del messaggio
    char response[MAX_DATA_LENGTH];
    memset(response, 0, MAX_DATA_LENGTH);
    // Successo dell'invio
    int success = 0;
    // Recupera il blocco
    size_t size;
    void* block = retrieve_block(client_fd, name, &size);
    // Se c'è un errore costruisce la stringa apposita
    if (block == NULL) {
        sprintf(response, "KO %d \n", errno);
        printf("[objectstore] Client %d: %s\n", client_fd, strerror(errno));
    }
    // Altrimenti costruisce l'header della risposta
    else sprintf(response, "DATA %zu \n", size);
    // Invia l'header
    success = send_message(client_fd, response, sizeof(char) * MAX_DATA_LENGTH);
    ASSERT_RETURN(success != -1, -1);
    // Invia il blocco se esiste
    if (block) {
        success = send_message(client_fd, block, size);
        ASSERT_RETURN(success != -1, -1);
    }
    // Libera la memoria occupata dal blocco
    free(block);
    // Restituisce il flag del successo
    return 0;
}

/**
 * @brief Termina la connessione con un client
 * 
 * @param client_fd File descriptor del client
 * @return int 1.
 */
int handle_leaving (int client_fd) {
    // Elimina l'utente dal sistema
    leave_client(client_fd);
    // Restituisce il flag 1 che indica la terminazione della connessione
    return 1;
}

/**
 * @brief Gestisce i segnali di tutto il programma
 * 
 * @param ptr Puntatore alla maschera su cui aspettare i segnali
 * @return void* NULL perché il thread viene avviato come detached
 */
void* signal_handler (void* ptr) {
    // Set di segnali da aspettare
    sigset_t set = *((sigset_t*) ptr);
    // Stampa un messaggio di log
    printf("[objectstore] Signal handling thread started and waiting for signals\n");
    // Entra nel loop di attesa dei segnali
    int signal;
    while (!terminated) {
        // Attende un segnale
        ASSERT_MESSAGE(sigwait(&set, &signal) != -1, "Waiting for signal", pthread_exit(NULL));
        // Riconosce il tipo di segnale
        if (signal == SIGINT || signal == SIGTERM || signal == SIGQUIT)
            terminated = 1;
        else if (signal == SIGUSR1)
            print_report();
    }
    printf("[objectstore] Signal handling thread stopped\n");
    return NULL;
}

/**
 * @brief Interpreta i campi di una richiesta, leggendo anche i dati addizionali ad essa allegati
 * 
 * @param client_fd File descriptor del client
 * @param message Messaggio arrivato dal client
 * @param size Dimensione del messaggio letto
 * @param verb_ptr Puntatore al verbo della richiesta
 * @param name_ptr Puntatore al nome della richiesta
 * @param lenght_ptr Puntatore alla lunghezza dei dati allegati
 * @param data_ptr Puntatore ai dati allegati
 * @return int Se il parsing è avvenuto con successo restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int parse_request (int client_fd, char* message, size_t size, char** verb_ptr, char** name_ptr, size_t* lenght_ptr, void** data_ptr) {
    // Lunghezza dell'header
    size_t header_length = strlen(message) + 1;
    // Puntatore d'appoggio
    char* save_ptr;
    // Campi dell'header
    char* verb = strtok_r(message, " ", &save_ptr);
    char* name = strtok_r(NULL, " ", &save_ptr);
    char* length_str = strtok_r(NULL, " ", &save_ptr);
    size_t length = 0;
    if (length_str) length = strtol(length_str, NULL, 10);
    // Fa puntare i puntatori ai valori da restituire
    *verb_ptr = verb;
    *name_ptr = name;
    *lenght_ptr = length;
    // Se la lunghezza definita è 0 non deve leggere il resto del campo dati
    if (length <= 0) return 0;
    // Buffer per i dati (allocato come char per poter fare addizioni)
    char* data = malloc(length);
    ASSERT_ERRNO_RETURN(data != NULL, ENOMEM, -1);
    // Numero di bytes dati già letti nel messaggio
    size_t data_bytes_read = size - (sizeof(char) * header_length);
    // Numero di bytes ancora da leggere
    size_t data_bytes_remaining = length - data_bytes_read;
    // Copia i bytes già letti nel buffer
    memcpy(data, message + header_length, data_bytes_read);
    // Legge i successivi bytes
    size_t data_bytes_remained = readn(client_fd, data + data_bytes_read, data_bytes_remaining);
    ASSERT(data_bytes_remained == data_bytes_remaining, free(data); return -1);
    // Fa puntare il puntatore ai dati
    *data_ptr = data;
    // Restituisce il successo
    return 0;
}

/**
 * @brief Interpreta i campi della richiesta e in
 * 
 * @param client_fd File descriptor del client
 * @param verb Verbo dell'operazione da svolgere
 * @param name Nome dell'oggetto da gestire
 * @param length Lunghezza dell'oggetto
 * @param data Blocco di dati del messaggio
 * @return int Se l'operazione è stata gestita con successo restituisce 0.
 * Se l'operazione è di terminazione restituisce 1.
 * Se c'è un errore restituisce -1 e setta errno.
 */
int route_request (int client_fd, char* verb, char* name, size_t length, void* data) {
    int success;
    // Prova tutti i possibili verbi ammessi
    if (EQUALS(verb, "REGISTER"))
        success = handle_registration(client_fd, name);
    else if (EQUALS(verb, "DELETE"))
        success = handle_deletion(client_fd, name);
    else if (EQUALS(verb, "STORE"))
        success = handle_storing(client_fd, name, length, data);
    else if (EQUALS(verb, "RETRIEVE"))
        success = handle_retrieving(client_fd, name);
    else if (EQUALS(verb, "LEAVE"))
        success = handle_leaving(client_fd);
    // Se non ha trovato un verbo riconosciuto invia un errore
    else success = -1;
    // Restituisce il flag restituito dai vari handler
    return success;
}


/**
 * @brief Legge un header dal client ed avvia la procedura associata. Se una delle procedure restituisce un errore lo invia al client.
 * 
 * @param ptr Puntatore != -1al file descriptor del client
 * @return void* Sempre NULL dato che la funzione non restituisce nulla
 */
void* connection_handler (void* ptr) {
    // File descriptor del client
    int* client_ptr = (int*) ptr;
    int client_fd = *client_ptr;
    // Verbo dell'operazione da svolgere
    char* verb = NULL;
    // Nome dell'oggetto associato al verbo
    char* name = NULL;
    // Lunghezza del messaggio
    size_t length = -1;
    // Dati allegati al messaggio
    void* data = NULL;
    // Stampa un messaggio di log
    printf("[objectstore] Client %d connected\n", client_fd);
    // Loop di gestione delle comunicazioni
    while (!terminated) {
        // Riceve il messaggio e la relativa dimensione
        size_t size = 0;
        char* message = receive_uninterrupted(client_fd, sizeof(char) * MAX_HEADER_LENGTH, &size);
        // Se non ci riesce la pipe è stata interrotta, quindi esce
        ASSERT((message != NULL) && (size > 0), break);
        // Interpreta il messaggio
        int result = parse_request(client_fd, message, size, &verb, &name, &length, &data);
        ASSERT(result != -1, free(message); break);
        // Stampa un messaggio di log
        printf("[objectstore]: Client %d asked for %s operation\n", client_fd, verb);
        // Indirizza il messaggio alla funzione competente
        result = route_request(client_fd, verb, name, length, data);
        // Libera la memoria occupata dall'header
        free(message);
        // Se la richiesta non è andata a buon stampa un errore
        ASSERT(result != -1, send_error(client_fd));
        // Se parse_request restituisce 1 il messaggio è di terminazione
        if (result == 1) break;
    }
    free(client_ptr);
    // Chiude la connessione
    ASSERT_MESSAGE_RETURN(close_socket(client_fd) == 0, "[objectstore] Closing socket", NULL);
    // Stampa un messaggio di uscita
    printf("[objectstore] Client %d: Connection terminated\n", client_fd);
    // Chiude il thread
    return NULL;
}

int main(int argc, char const *argv[]) {
    // Crea una maschera per mascherare i segnali che intende gestire
    sigset_t set;
    ASSERT_MESSAGE(sigemptyset(&set) != -1, "[objectstore] Emptying signal mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGPIPE) != -1, "[objectstore] Adding SIGPIPE to mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGINT) != -1, "[objectstore] Adding SIGINT to mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGTERM) != -1, "[objectstore] Adding SIGTERM to mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGQUIT) != -1, "[objectstore] Adding SIGQUIT to mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGUSR1) != -1, "[objectstore] Adding SIGUSR1 to mask", exit(1));
    // Maschera questi segnali per tutti i thread
    ASSERT_MESSAGE(pthread_sigmask(SIG_SETMASK, &set, NULL) == 0, "[objectstore] Applying signal mask", exit(1));
    // Avvia il thread gestore dei segnali in modalità detached
    pthread_t sig_handler_id;
    ASSERT_MESSAGE(pthread_create(&sig_handler_id, NULL, signal_handler, (void*) &set) == 0, "[objectstore] Creating signal handling thread", exit(1));
    // Crea la lista dei thread attivi
    pthread_list_t* thread_list = NULL;
    // Crea il server socket su cui attendere connessioni
    int server_fd = create_server_socket(SOCKET_NAME);
    // Controlla che la creazione sia andata a buon fine oppure esce
    ASSERT_MESSAGE(server_fd != -1, "[objectstore] Creating server socket", exit(1));
    // Inizializza le funzioni worker
    int success = init_worker_functions();
    ASSERT_MESSAGE(success != -1, "[objectstore] Initializing data structures for work", exit(1));
    // Crea il file descriptor set per accettare nuove connessioni
    fd_set fset = create_fd_set(server_fd);
    // Crea il timeout per far attendere il selettore
    struct timeval timeout = {1, 0};
    // Stampa un messaggio di log
    printf("[objectstore] Started on socket %s (file descriptor %d) and waiting for connections...\n", SOCKET_NAME, server_fd);
    // Loop in cui attende nuove connessioni
    while (!terminated) {
        // Attende una nuova connessione
        int client_fd = accept_new_client(server_fd, fset, timeout);
        ASSERT_MESSAGE(client_fd != -1, "[objectstore] Accepting client", exit(1));
        // Se è arrivato un nuovo client lo gestisce
        if (client_fd > 0) {
            // Copia il file descriptor in una variabile da passare
            int* client_ptr = malloc(sizeof(int));
            *client_ptr = client_fd;
            // Crea un nuovo thread a cui passa la connessione
            pthread_t thread_id;
            ASSERT_MESSAGE(pthread_create(&thread_id, NULL, connection_handler, (void*) client_ptr) == 0, "[objectstore] Creating thread", break);
            // Mette il thread nella coda
            ASSERT_MESSAGE(insert_pthread_list(&thread_list, thread_id) == 0, "[objectstore] Inserting thread in waiting list", break);
        }
    }
    // Attende la terminazione di tutti i thread
    while (thread_list != NULL) {
        pthread_t thread_id = remove_pthread_list_head(&thread_list);
        ASSERT_MESSAGE(pthread_join(thread_id, NULL) == 0, "[objectstore] Joining thread", exit(1));
    }
    // Libera la memoria occupata dalle funzioni worker
    ASSERT_MESSAGE(stop_worker_functions() != -1, "[objectstore] Stopping worker function", exit(1));
    // Chiude il socket del server, altrimenti stampa un messaggio
    ASSERT_MESSAGE(pthread_join(sig_handler_id, NULL) == 0, "[objectstore] Joining signal handling socket", exit(1));
    ASSERT_MESSAGE(close_server_socket(server_fd, SOCKET_NAME) != -1, "[objectstore] Closing socket", exit(1));
    // Stampa il messaggio di uscita
    printf("[objectstore] Server stopped\n");
    return 0;
}
