#define _POSIX_SOURCE

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

#include <socket/socket.h>
#include <workers/workers.h>
#include <pthread_list/pthread_list.h>

#include <shared.h>

// Variabile globale che indica la terminazione
int terminated = 0;

/**
 * @brief Invia al client il messaggio 'KO <errno>'
 * 
 * @param client_fd File descriptor del client.
 */
void send_error (int client_fd) {
    // Inizializza il buffer che contiene l'errore
    char* err_buffer = (char*) calloc(MAX_HEADER_LENGTH, sizeof(char));
    ASSERT_MESSAGE(err_buffer != NULL, "Allocating buffer for error", return);
    // Costruisce la stringa formattata
    sprintf(err_buffer, "KO %d \n", errno);
    // Scrive la stringa sul buffer
    int success = send_message(client_fd, err_buffer, MAX_HEADER_LENGTH);
    ASSERT_MESSAGE(success != -1, "Writing error message to client", return);
    // Libera il buffer
    free(err_buffer);
}

/**
 * @brief Stampa un report sullo standard output
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
    printf("[REPORT] Connected clients: %d Object number: %d Total size: %d bytes\n", clients, objects, size);
}

/**
 * @brief Invia un messaggio di successo al client. Se non ci riesce gli manda un messaggio di errore.
 * 
 * @param client_fd File descriptor del client
 */
void send_ok (int client_fd) {
    // Crea la stringa con scritto ok
    char ok_string[MAX_HEADER_LENGTH] = "OK \n";
    int success = send_message(client_fd, ok_string, MAX_HEADER_LENGTH);
    ASSERT(success != -1, send_error(client_fd));
}

int handle_registration (int client_fd, char* name) {
    // Registra l'utente nel sistema
    int success = register_user(client_fd, name);
    // Controlla che sia andato tutto bene
    ASSERT_RETURN(success == 0, -1);
    // Restituisce il successo
    send_ok(client_fd);
    return 0;
}

int handle_deletion (int client_fd, char* name) {
    // Rimuove l'utente dal sistema
    int success = delete_block(client_fd, name);
    // Controlla che l'operazione sia avvenuta con successo
    ASSERT_RETURN(success == 0, -1);
    // Restituisce il successo
    send_ok(client_fd);
    return 0;
}

int handle_storing (int client_fd, char* name, size_t length) {
    // Legge i dati
    void* data = receive_message(client_fd, length);
    ASSERT_RETURN(data != NULL, -1);
    // Scrive i dati sul disco
    int success = store_block(client_fd, name, data, length);
    ASSERT_RETURN(success != -1, -1);
    free(data);
    // Invia l'ok
    send_ok(client_fd);
    return 0;
}

int handle_retrieving (int client_fd, char* name) {
    // Alloca l'header del messaggio
    char* header = (char*) calloc(MAX_HEADER_LENGTH, sizeof(char));
    ASSERT_RETURN(header != NULL, -1);
    // Recupera il blocco
    size_t size;
    void* block = retrieve_block(client_fd, name, &size);
    ASSERT(block != NULL, free(header); return -1);
    // Costruisce e invia l'header
    sprintf(header, "DATA %zu \n", size);
    int success = send_message(client_fd, header, sizeof(char) * MAX_HEADER_LENGTH);
    free(header);
    ASSERT_RETURN(success != -1, -1);
    // Invia il blocco
    success = send_message(client_fd, block, size);
    ASSERT_RETURN(success != -1, -1);
    // Libera la memoria occupata dal blocco
    free(block);
    // Restituisce il flag del successo
    return 0;
}

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
    // Crea una maschera per SIGINT e SIGUSR1
    sigset_t set = *((sigset_t*) ptr);
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
    return NULL;
}

/**
 * @brief Gestisce una richiesta riconoscendo l'header come una concatenazione <verb> <name> [<length>]
 * 
 * @param client_fd File descriptor del client
 * @param header Header inviato dal client
 * @return int 0 se la richiesta è stata gestita con successo, 1 se la richiesta è di terminazione. Se c'è un errore restituisce -1 e setta errno.
 */
int parse_request (int client_fd, char* header) {
    // Verbo nell'header
    char* verb = (char*) calloc(9, sizeof(char));
    // Nome nell'header
    char* name = (char*) calloc(255, sizeof(char));
    // Dimensione nell'header
    size_t length = 0;
    // Analizza la stringa di header estraendo le informazioni
    sscanf(header, "%s %s %zu \n", verb, name, &length);
    // Risultato della computazione
    int success;
    // Prima tenta di riconoscere i verbi che non necessitano di ulteriori letture o scritture
    if (EQUALS(verb, "REGISTER"))
        success = handle_registration(client_fd, name);
    else if (EQUALS(verb, "DELETE"))
        success = handle_deletion(client_fd, name);
    // Dopodiché passa il controllo ai metodi che richiedono di leggere o scrivere ancora dal client
    else if (EQUALS(verb, "STORE"))
        success = handle_storing(client_fd, name, length);
    else if (EQUALS(verb, "RETRIEVE"))
        success = handle_retrieving(client_fd, name);
    else if (EQUALS(verb, "LEAVE"))
        success = handle_leaving(client_fd);
    // Se non ha trovato un verbo riconosciuto invia un errore
    else success = -1;
    // Libera la memoria occupata dalle stringhe
    free(verb); free(name);
    // Restituisce il flag restituito dai controller
    return success;
}

/**
 * @brief Legge un header dal client ed avvia la procedura associata.
 * 
 * @param ptr Puntatore != -1al file descriptor del client
 * @return void* Sempre NULL dato che la funzione non restituisce nulla
 */
void* connection_handler (void* ptr) {
    // File descriptor del client
    int* client_ptr = (int*) ptr;
    int client_fd = *(client_ptr);
    // Loop di gestione delle comunicazioni
    while (!terminated) {
        // Header del messaggio
        char* header = receive_message(client_fd, sizeof(char) * MAX_HEADER_LENGTH);
        // Se non ci riesce la pipe è stata interrotta, quindi esce
        if (!header) {
            free(header);
            break;
        }
        // Avvia la gestione della richiesta
        int result = parse_request(client_fd, header);
        // Libera la memoria occupata dall'header
        free(header);
        // Se la richiesta non è andata a buon stampa un errore
        ASSERT(result != -1, send_error(client_fd));
        // Se parse_request restituisce 1 il messaggio è di terminazione
        if (result == 1) break;
    }
    // Libera la memoria occupata dal file descriptor
    free(client_ptr);
    // Chiude la connessione
    ASSERT_MESSAGE_RETURN(close_socket(client_fd) == 0, "Closing socket", NULL);
    return NULL;
}

int main(int argc, char const *argv[]) {
    // Crea una maschera per mascherare i segnali che intende gestire
    sigset_t set;
    ASSERT_MESSAGE(sigemptyset(&set) != -1, "Emptying signal mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGPIPE) != -1, "Adding SIGPIPE to mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGINT) != -1, "Adding SIGINT to mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGTERM) != -1, "Adding SIGTERM to mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGQUIT) != -1, "Adding SIGQUIT to mask", exit(1));
    ASSERT_MESSAGE(sigaddset(&set, SIGUSR1) != -1, "Adding SIGUSR1 to mask", exit(1));
    // Maschera questi segnali per tutti i thread
    ASSERT_MESSAGE(pthread_sigmask(SIG_SETMASK, &set, NULL) == 0, "Applying signal mask", exit(1));
    // Avvia il thread gestore dei segnali in modalità detached
    pthread_t sig_handler_id;
    ASSERT_MESSAGE(pthread_create(&sig_handler_id, NULL, signal_handler, (void*) &set) == 0, "Creating signal handling thread", exit(1));
    // Crea la lista dei thread attivi
    pthread_list_t* thread_list = NULL;
    // Crea il server socket su cui attendere connessioni
    int server_fd = create_server_socket(SOCKET_NAME);
    // Controlla che la creazione sia andata a buon fine oppure esce
    ASSERT_MESSAGE(server_fd != -1, "Creating server socket", exit(1));
    // Inizializza le funzioni worker
    int success = init_worker_functions();
    ASSERT_MESSAGE(success != -1, "Initializing workers", exit(1));
    // Crea il file descriptor set per accettare nuove connessioni
    fd_set fset = create_fd_set(server_fd);
    // Crea il timeout per far attendere il selettore
    struct timeval timeout = {1, 0};
    // Loop in cui attende nuove connessioni
    while (!terminated) {
        // Attende una nuova connessione
        int client_fd = accept_new_client(server_fd, fset, timeout);
        ASSERT_MESSAGE(client_fd != -1, "Accepting client", exit(1));
        // Se è arrivato un nuovo client lo gestisce
        if (client_fd > 0) {
            // Copia il file descriptor in una variabile da passare
            int* client_ptr = malloc(sizeof(int));
            *client_ptr = client_fd;
            // Crea un nuovo thread a cui passa la connessione
            pthread_t thread_id;
            ASSERT_MESSAGE(pthread_create(&thread_id, NULL, connection_handler, (void*) client_ptr) == 0, "Creating thread", break);
            // Mette il thread nella coda
            ASSERT_MESSAGE(insert_pthread_list(&thread_list, thread_id) == 0, "Inserting thread in waiting list", break);
        }
    }
    // Attende la terminazione di tutti i thread
    while (thread_list != NULL) {
        pthread_t thread_id = remove_pthread_list_head(&thread_list);
        ASSERT_MESSAGE(pthread_join(thread_id, NULL) == 0, "Joining thread", exit(1));
    }
    // Libera la memoria occupata dalle funzioni worker
    ASSERT_MESSAGE(stop_worker_functions() != -1, "Stopping workers", exit(1));
    // Chiude il socket del server, altrimenti stampa un messaggio
    ASSERT_MESSAGE(pthread_join(sig_handler_id, NULL) == 0, "Joining signal handling socket", exit(1));
    ASSERT_MESSAGE(close_server_socket(server_fd, SOCKET_NAME) != -1, "Closing socket", exit(1));
    return 0;
}
