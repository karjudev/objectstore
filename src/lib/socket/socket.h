/**
 * @file socket.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Strutture dati e metodi di aiuto alla comunicazione tra socket.
 * @version 0.1
 * @date 2019-06-11
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#if !defined(_SOCKET)
#define _SOCKET

/**
 * @brief Legge n bytes dal file descriptor, inserendoli nel buffer.
 *
 * @param file_descriptor File descriptor da cui leggere
 * @param buffer Buffer in cui scrivere i bytes letti
 * @param n Numero di bytes da leggere
 * @param size_t Numero di bytes effettivamente letti, -1 se c'è un errore
 */
size_t readn (int file_descriptor, void* buffer, size_t n);

/**
 * @brief Scrive n bytes sul file descriptor prendendoli dal buffer.
 *
 * @param file_descriptor File descriptor su cui scrivere
 * @param buffer Buffer da cui prendere i dati
 * @param n Numero di bytes da scrivere
 * @return size_t n se l'operazione è completata con successo, -1 se c'è un errore
 */
size_t writen (int file_descriptor, const void *buffer, size_t n);

/**
 * @brief Crea un file descriptor collegato ad un server socket AF_UNIX.
 *
 * @param socket_name Nome del file che rappresenta il socket
 * @return int File descriptor del socket. Se c'è un errore restituisce -1 e setta errno.
 */
int create_server_socket (char* socket_name);

/**
 * @brief Crea un file descriptor collegato ad un client socket AF_UNIX.
 *
 * @param socket_name Nome del file che rappresenta il socket
 * @return int File descriptor del socket. Se c'è un errore restituisce -1 e setta errno.
 */
int create_client_socket (char* socket_name);

/**
 * @brief Chiude un socket.
 *
 * @param socket_fd File descriptor del socket da chiudere
 * @return int 0 se il socket è stato chiuso con successo. Se c'è un errore restituisce -1 e setta errno.
 */
int close_socket (int socket_fd);

/**
 * @brief Chiude un server socket e cancella il nome del file a cui è associato.
 *
 * @param socket_fd File descriptor del file
 * @param filename Nome del file da cancellare
 * @return 0 se tutto è stato chiuso correttamente. Se c'è un errore restituisce -1 e setta errno.
 */
int close_server_socket (int socket_fd, char* filename);

/**
 * @brief Accetta la connessione di un client su un server socket
 *
 * @param server_fd File descriptor del server
 * @return int File descriptor del client connesso al server. Se c'è un errore restituisce -1 e setta errno.
 */
int accept_client (int server_fd);


#endif // _SOCKET
