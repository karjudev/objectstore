/**
 * @file socket.c
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Implementazione dei metodi di aiuto alla comunicazione tra socket.
 * @version 0.1
 * @date 2019-06-11
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/un.h>

#include <socket/safeio.h>
#include <socket/socket.h>
#include <assertmacros.h>

#define UNIX_PATH_MAX 108

/**
 * @brief Invia un messaggio al server.
 * 
 * @param message Messaggio da inviare
 * @param size Dimensione del messaggio
 * @return int 0 se il messaggio è stato inviato correttamente. Se c'è un errore restituisce -1 e setta errno
 */
int send_message (int file_descriptor, void* message, size_t size) {
    // Controlla che i parametri siano corretti
    ASSERT_ERRNO_RETURN((message != NULL) && (size > 0), EINVAL, -1);
    // Scrive il messaggio al server
    int bytes_written = writen(file_descriptor, message, size);
    // Controlla che il messaggio sia stato inviato correttamente
    ASSERT_RETURN(bytes_written != -1, -1);
    // Restituisce il flag di successo
    return 0;
}

/**
 * @brief Riceve dal client un messaggio di dimensione size
 * 
 * @param file_descriptor File descriptor da cui leggere
 * @param size Dimensione del messaggio
 * @return void* Puntatore al messaggio se la ricezione è avvenuta con successo. Se c'è un errore restituisce NULL e setta errno.
 */
void* receive_message (int file_descriptor, size_t size) {
	// Controlla che i parametri siano corretti
	ASSERT_ERRNO_RETURN((file_descriptor > 0) && (size > 0), EINVAL, NULL);
    // Alloca il buffer per ricevere il messaggio
	void* buffer = malloc(size);
	// Inizializza il buffer
	memset(buffer, 0, size);
	// Controlla che l'allocazione sia avvenuta con successo
	ASSERT_ERRNO_RETURN(buffer != NULL, ENOMEM, NULL);
	// Riceve i dati
	int bytes_read = readn(file_descriptor, buffer, size);
	// Controlla che la lettura sia avvenuta con successo
	ASSERT(bytes_read > 0, free(buffer); return NULL);
	// Restituisce i dati
	return buffer;
}

/**
 * @brief Crea una struttura dati per ospitare l'indirizzo del socket.
 *
 * @param socket_name Nome del file su cui creare il socket.
 * @return struct sockaddr_un Struttura dati di indirizzo se il socket è stato creato con successo, NULL se c'è un errore.
 */
static struct sockaddr_un create_socket_address (char* socket_name) {
	// Alloca la struttura dati
	struct sockaddr_un socket_address;
	// Copia il nome del socket
	strncpy(socket_address.sun_path, socket_name, UNIX_PATH_MAX);
	// Setta il tipo di socket
	socket_address.sun_family = AF_UNIX;
	// Restituisce il socket
	return socket_address;
}

/**
 * @brief Crea un file descriptor collegato ad un server socket AF_UNIX.
 *
 * @param socket_name Nome del file che rappresenta il socket
 * @return int File descriptor del socket, -1 se c'è stato un errore
 */
int create_server_socket (char* socket_name) {
	// File descriptor del socket
	int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_RETURN(server_fd != -1, -1);
	// Indirizzo del socket
	struct sockaddr_un socket_address = create_socket_address(socket_name);
	// Collega l'indirizzo al socket
	int error = bind(server_fd, (struct sockaddr*) &socket_address, sizeof(socket_address));
	ASSERT_RETURN(error != -1, -1);
	// Accetta di ricevere connessioni
	error = listen(server_fd, SOMAXCONN);
	ASSERT_RETURN(error != -1, -1);
	// Restituisce il flag di successo
	return server_fd;
}

/**
 * @brief Crea un file descriptor collegato ad un client socket AF_UNIX.
 *
 * @param socket_name Nome del file che rappresenta il socket
 * @return int File descriptor del socket, -1 se c'è stato un errore
 */
int create_client_socket (char* socket_name) {
	// File descriptor del socket
	int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	ASSERT_RETURN(client_fd != -1, -1);
	// Indirizzo del socket
	struct sockaddr_un socket_address = create_socket_address(socket_name);
	// Collega il client al socket
	int error = connect(client_fd, (struct sockaddr*) &socket_address, sizeof(socket_address));
	ASSERT_RETURN(error != -1, -1);
	// Restituisce il flag del client
	return client_fd;
}

/**
 * @brief Chiude un socket.
 *
 * @param socket_fd File descriptor del socket da chiudere
 * @return int 0 se il socket è stato chiuso con successo, -1 se c'è stato un errore
 */
int close_socket (int socket_fd) {
	// Chiude il socket
	return close(socket_fd);
}

/**
 * @brief Chiude un server socket e cancella il nome del file a cui è associato.
 *
 * @param socket_fd File descriptor del file
 * @param filename Nome del file da cancellare
 * @return 0 se tutto è stato chiuso correttamente, -1 se c'è un errore
 */
int close_server_socket (int socket_fd, char* filename) {
	// Chiude il socket
	int error = close_socket(socket_fd);
	ASSERT_RETURN(error != -1, -1);
	// Cancella il file
	error = unlink(filename);
	// Restituisce il flag che indica il successo
	return error;
}

/**
 * @brief Crea un fd_set che contiene il file descriptor del server
 * 
 * @param server_fd File descriptor del server
 * @return fd_set Insieme di file descriptor che contiene il server
 */
fd_set create_fd_set (int server_fd) {
	fd_set set;
	FD_ZERO(&set);
	FD_SET(server_fd, &set);
	return set;
}

/**
 * @brief Accetta la connessione di un nuovo client tramite un selettore
 * 
 * @param server_fd File descriptor del server
 * @param set File descriptor set da cui leggere connessioni
 * @param timeout Timeout massimo da attendere prima di uscire
 * @return int File descriptor del nuovo client. Se il timeout è scaduto restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int accept_new_client (int server_fd, fd_set set, struct timeval timeout) {
	// Set di appoggio per evitare che l'altro sia modificato
	fd_set ready_set = set;
	// Richiede nuovi file descriptor pronti
	int success = select(server_fd + 1, &ready_set, NULL, NULL, &timeout);
	// Se c'è un errore o non ci sono file descriptor attivi esce
	if (success <= 0) return success;
	// Se è pronto il file descriptor del server
	if (FD_ISSET(server_fd, &ready_set)) {
		int client_fd = accept(server_fd, NULL, 0);
		return client_fd;
	}
	// Se è arrivato in fondo senza trovare nulla (impossibile) esce
	return -1;
}