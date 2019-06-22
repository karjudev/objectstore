#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

#include <assertmacros.h>
#include <socket/socket.h>

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
    char* err_buffer = malloc(sizeof(char) * 7);
    ASSERT_MESSAGE_RETURN(err_buffer != NULL, "Allocating buffer for error",);
    // Costruisce la stringa formattata
    sprintf(err_buffer, "KO %d \n", errno);
    // Scrive la stringa sul buffer
    int success = send_message(client_fd, err_buffer, 7);
    ASSERT_MESSAGE_RETURN(success != -1, "Writing error message to client",);
    // Libera il buffer
    free(err_buffer);
}

/**
 * @brief Invia un messaggio di successo al client. Se non ci riesce gli manda un messaggio di errore.
 * 
 * @param client_fd File descriptor del client
 */
void send_ok (int client_fd) {
    int success = send_message(client_fd, "OK \n", 7);
    ASSERT(success != -1, send_error(client_fd));
}

int handle_registration (int client_fd, char* name) {
    send_ok(client_fd);
    return 0;
}

int handle_deletion (int client_fd, char* name) {
    send_ok(client_fd);
    return 0;
}

int handle_storing (int client_fd, char* name, size_t length) {
    // Legge i dati
    void* data = receive_message(client_fd, length);
    ASSERT_RETURN(data != NULL, -1);
    free(data);
    // Invia l'ok
    send_ok(client_fd);
    return 0;
}

int handle_retrieving (int client_fd, char* name) {
    // Recupera il blocco
    int data[10];
    for (int i = 0; i < 10; i++) data[i] = i;
    // Crea l'header
    char* header = (char*) calloc(MAX_HEADER_LENGTH, sizeof(char));
    sprintf(header, "DATA %zu \n", sizeof(data));
    // Invia l'header
    int success = send_message(client_fd, header, sizeof(char) * MAX_HEADER_LENGTH);
    free(header);
    ASSERT_RETURN(success != -1, -1);
    // Invia il blocco
    success = send_message(client_fd, data, sizeof(data));
    return success;
}

/**
 * @brief Gestisce una richiesta riconoscendo l'header come una concatenazione <verb> <name> [<length>]
 * 
 * @param client_fd File descriptor del client
 * @param header Header inviato dal client
 * @return int 0 se la richiesta è stata gestita con successo, 1 se la richiesta è di terminazione. Se c'è un errore restituisce -1 e setta errno.
 */
int parse_request (int client_fd, char* header) {
    // Controlla che la richiesta sia di terminazione
    if (strncmp(header, "LEAVE \n", 8) == 0) return 1;
    // Verbo nell'header
    char* verb = (char*) calloc(9, sizeof(char));
    // Nome nell'header
    char* name = (char*) calloc(255, sizeof(char));
    // Dimensione nell'header
    size_t length = 0;
    printf("header = %s", header);
    // Analizza la stringa di header estraendo le informazioni
    sscanf(header, "%s %s %zu \n", verb, name, &length);
    // Prima tenta di riconoscere i verbi che non necessitano di ulteriori letture o scritture
    if (EQUALS(verb, "REGISTER", 9))
        return handle_registration(client_fd, name);
    else if (EQUALS(verb, "DELETE", 7))
        return handle_deletion(client_fd, name);
    // Dopodiché passa il controllo ai metodi che richiedono di leggere o scrivere ancora dal client
    else if (EQUALS(verb, "STORE", 6))
        return handle_storing(client_fd, name, length);
    else if (EQUALS(verb, "RETRIEVE", 9))
        return handle_retrieving(client_fd, name);
    // Se non è un verbo riconosciuto invia un errore
    else send_error(client_fd);
    // Libera la memoria delle stringhe
    free(verb); free(name);
    return 0;
}

/**
 * @brief Legge un header dal client ed avvia la procedura associata.
 * 
 * @param ptr Puntatore al file descriptor del client
 * @return void* Sempre NULL dato che la funzione non restituisce nulla
 */
void* connection_handler (void* ptr) {
    // Flag che indica che la connessione è terminata
    int connection_terminated = 0;
    // File descriptor del client
    int client_fd = *((int*) ptr);
    while (!connection_terminated) {
        // Header del messaggio
        char* header = receive_message(client_fd, sizeof(char) * MAX_HEADER_LENGTH);
        // Se non ci riesce libera la memoria, invia un errore e ritorna all'iterazione successiva
        ASSERT_MESSAGE(header != NULL, "Reading header", free(header); send_error(client_fd); continue);
        // Avvia la gestione della richiesta
        int result = parse_request(client_fd, header);
        // Se la richiesta non è andata a buon stampa un errore
        ASSERT(result != -1, send_error(client_fd));
        // Se parse_request restituisce 1 il messaggio è di terminazione
        if (result == 1) connection_terminated = 1;
        // Libera la memoria occupata dall'header
        free(header);
    }
    close_socket(client_fd);
    return NULL;
}

int main(int argc, char const *argv[]) {
    // Crea un server socket su cui attendere connessioni
    int server_fd = create_server_socket(SOCKET_NAME);
    // Controlla che la creazione sia andata a buon fine oppure esce
    ASSERT_MESSAGE(server_fd != -1, "Creating server socket", exit(1));
    // Loop in cui attende nuove connessioni
    while (!terminated) {
        // Attende una nuova connessione
        int client_fd = accept_client(server_fd);
        ASSERT_MESSAGE(client_fd != -1, "Accepting client", exit(1));
        // Crea un nuovo thread a cui passa la connessione
        pthread_t thread;
        pthread_create(&thread, NULL, connection_handler, (void*) &client_fd);
        // Mette il thread in modalità detached
        pthread_detach(thread);
    }
    // Chiude il socket del server, altrimenti stampa un messaggio
    ASSERT_MESSAGE(close_server_socket(server_fd, SOCKET_NAME) != -1, "Closing socket", exit(1));
    return 0;
}
