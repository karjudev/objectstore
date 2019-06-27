/**
 * @file osclient.c
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Implementazione della libreria di connessione al server object store.
 * @version 0.1
 * @date 2019-06-11
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/select.h>

#include <socket/socket.h>

#include <osclient/osclient.h>

#include <assertmacros.h>

#include <shared.h>

// File descriptor del client, variabile globlae della libreria
static int server_fd = -1;

static int check_response (char* response) {
    if (EQUALS(response, "OK \n")) return 1;
    int errcode = 0;
    sscanf(response, "KO %d \n", &errcode);
    errno = errcode;
    return 0;
}

static int send_header (char* format, char* name, size_t length) {
    // Inizializza il buffer che contiene l'header
    char* header = (char*) calloc(MAX_HEADER_LENGTH, sizeof(char));
    ASSERT_ERRNO_RETURN(header != NULL, ENOMEM, -1);
    // Distingue il tipo di parametri passati e costruisce la stringa
    if (length != 0) sprintf(header, format, name, length);
    else sprintf(header, format, name);
    // Invia l'header
    int success = send_message(server_fd, header, MAX_HEADER_LENGTH);
    // Libera la memoria occupata dall'header
    free(header);
    // Restituisce il successo dell'operazione
    return success;
}

/**
 * @brief Inizializza la connessione globale con il server.
 * 
 * @param name Nome dell'utente da connettere
 * @return int 1 se la connessione è andata a buon fine. Se c'è un errore restituisce 0 e setta errno.
 */
int os_connect (char* name) {
    // Si collega al server socket
    server_fd = create_client_socket(SOCKET_NAME);
    ASSERT_RETURN(server_fd != -1, -1);
    // Invia l'header
    int success = send_header("REGISTER %s \n", name, 0);
    ASSERT_RETURN(success != -1, 0);
    // Riceve la risposta
    char* response = receive_message(server_fd, 7);
    ASSERT(response != NULL, free(response); return 0);
    // Restituisce il valore della risposta
    success = check_response(response);
    free(response);
    return success;
}

/**
 * @brief Memorizza sul server l'area di memoria di lunghezza len puntata da block.
 * 
 * @param name Nome del blocco da memorizzare
 * @param block Dati del blocco da memorizzare
 * @param len Lunghezza del blocco da memorizzare
 * @return int 1 se la memorizzazione è andata a buon fine. Se c'è un errore restituisce 0 e setta errno.
 */
int os_store (char* name, void* block, size_t len) {
    // Controlla la correttezza dei parametri
    ASSERT_ERRNO_RETURN((name != NULL) && (block != NULL) && (len > 0), EINVAL, 0);
    ASSERT_ERRNO_RETURN(server_fd > 0, ENOTCONN, -1);
    // Invia l'header
    int success = send_header("STORE %s %ld \n", name, len);
    // Verifica che l'invio sia andato a buon fine
    ASSERT_RETURN(success != -1, 0);
    // Invia i dati
    success = send_message(server_fd, block, len);
    ASSERT_RETURN(success != -1, 0);
    // Riceve la risposta
    char* response = receive_message(server_fd, 7);
    ASSERT(response != NULL, free(response); return 0);
    // Restituisce il valore della risposta
    success = check_response(response);
    free(response);
    return success;
}

/**
 * @brief Recupera il blocco di dati identificato da name.
 * 
 * @param name Nome del blocco di dati.
 * @return void* Blocco di dati se il recupero ha avuto successo. Se c'è un errore restituisce NULL e setta errno.
 */
void* os_retrieve (char* name) {
    // Controlla che il nome sia stato passato correttamente
    ASSERT_ERRNO_RETURN(name != NULL, EINVAL, NULL);
    // Invia l'header
    int success = send_header("RETRIEVE %s \n", name, 0);
    ASSERT_RETURN(success != -1, 0);
    // Riceve il messaggio con l'header della risposta
    char* res_header = receive_message(server_fd, MAX_HEADER_LENGTH);
    ASSERT_RETURN(res_header != NULL, 0);
    // Legge la dimensione dei dati in arrivo
    size_t size;
    sscanf(res_header, "DATA %zu \n", &size);
    free(res_header);
    // Riceve effettivamente i dati
    void* data = receive_message(server_fd, size);
    ASSERT_RETURN(data != NULL, 0);
    // Restituisce i dati
    return data;
}

/**
 * @brief Cancella il blocco di dati identificato da name
 * 
 * @param name Nome del blocco di dati
 * @return int 1 se l'eliminazione è avvenuta con successo. Se c'è un errore restituisce 0 e setta errno.
 */
int os_delete (char* name) {
    // Invia l'header
    int success = send_header("DELETE %s \n", name, 0);
    ASSERT_RETURN(success != -1, 0);
    // Attende la risposta
    char* response = receive_message(server_fd, 7);
    // Verifica che sia un ok
    success = check_response(response);
    free(response);
    return success;
}

/**
 * @brief Si disconnette dal server
 * 
 * @return int 1 se la disconnessione è avvenuta con successo. Se c'è un errore restituisce 0 e setta errno.
 */
int os_disconnect() {
    // Invia al server il comando di leave
    int success = send_message(server_fd, "LEAVE \n", MAX_HEADER_LENGTH);
    ASSERT_RETURN(success != 1, 0);
    // Chiude la connessione al socket
    success = close_socket(server_fd);
    return (success == 0);
}