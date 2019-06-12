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
#include <sys/types.h>
#include <sys/un.h>

#include <socket/socket.h>
#include <assert.h>

#define UNIX_PATH_MAX 108

/**
 * @brief Legge n bytes dal file descriptor, inserendoli nel buffer.
 *
 * @param file_descriptor File descriptor da cui leggere
 * @param buffer Buffer in cui scrivere i bytes letti
 * @param n Numero di bytes da leggere
 * @param size_t Numero di bytes effettivamente letti, -1 se c'è un errore
 */
size_t readn (int file_descriptor, void* buffer, size_t n) {
	// Controlla la correttezza dei parametri
	ASSERT_ERRNO_RETURN((file_descriptor > 0) && (buffer != NULL) && (n > 0), EINVAL, -1);
	// Numero di bytes rimasti
	size_t nleft = n;
	// Numero di bytes letti ad ogni iterazione
	size_t nread;
	// Puntatore che si sposta all'interno del buffer
	char* ptr = buffer;
	// Continua a scorrere finché non ha letto tutti i bytes
	while (nleft > 0) {
		// Legge un insieme di bytes
		nread = read(file_descriptor, ptr, nleft);
		// Se la read ha letto un numero negativo di bytes ci potrebbe essere stato un errore
		if (nread < 0) {
			// Se la read è stata interrotta deve essere ritentata
			if (errno == EINTR) nread = 0;
			// Altrimenti l'errore non è gestibile
			else return (-1);
		}
		// Se la read ha letto 0 bytes il client ha terminato la connessione
		else if (nread == 0) break;
		// Decrementa il numero di bytes ancora da leggere
		nleft -= nread;
		// Sposta il puntatore per leggere il prossimo insieme di bytes
		ptr += nread;
	}
	// Restituisce il numero di bytes effettivamente letti
	return (n - nleft);
}

/**
 * @brief Scrive n bytes sul file descriptor prendendoli dal buffer.
 *
 * @param file_descriptor File descriptor su cui scrivere
 * @param buffer Buffer da cui prendere i dati
 * @param n Numero di bytes da scrivere
 * @return size_t n se l'operazione è completata con successo, -1 se c'è un errore
 */
size_t writen (int file_descriptor, const void *buffer, size_t n) {
	// Controlla la correttezza degli argomenti
	ASSERT_ERRNO_RETURN((file_descriptor > 0) && (n > 0), EINVAL, -1);
	// Numero di bytes rimasti da leggere
	size_t nleft = n;
	// Numero di bytes scritti ad ogni iterazione
	size_t nwritten;
	// Buffer che si sposta all'interno del buffer
	const char* ptr = buffer;
	// Continua finché non ha scritto tutti i bytes
	while (nleft > 0) {
		// Scrive uno stock di bytes
		nwritten = write(file_descriptor, ptr, nleft);
		// Se è stato scritto un numero non nullo di bytes ci potrebbe essere stato un errore
		if (nwritten <= 0) {
			// Se la scrittura è stata interrotta si può ripetere
			if (nwritten < 0 && errno == EINTR) nwritten = 0;
			// Altrimenti l'errore non è recuperabile
			else return (-1);
		}
		// Decrementa il numero di bytes rimasti
		nleft -= nwritten;
		// Sposta il puntatore dentro il buffer
		ptr += nwritten;
	}
	// Restituisce il numero di bytes scritti perché è andato tutto bene
	return n;
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
 * @brief Accetta la connessione di un client su un server socket
 *
 * @param server_fd File descriptor del server
 * @return int File descriptor del client connesso al server, -1 se c'è stato un errore
 */
int accept_client (int server_fd) {
	// Accetta l'arrivo di un nuovo client
	int client_fd = accept(server_fd, NULL, 0);
	ASSERT_RETURN(client_fd != -1, -1);
	// Restituisce il numero di file descriptor del client
	return client_fd;
}
