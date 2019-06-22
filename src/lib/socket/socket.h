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
 * @brief Invia un messaggio al server.
 * 
 * @param file_descriptor File descriptor su cui scrivere
 * @param message Messaggio da inviare
 * @param size Dimensione del messaggio
 * @return int 0 se il messaggio è stato inviato correttamente. Se c'è un errore restituisce -1 e setta errno.
 */
int send_message (int file_descriptor, void* message, size_t size);

/**
 * @brief Riceve dal client un messaggio di dimensione size
 * 
 * @param file_descriptor File descriptor da cui leggere
 * @param size Dimensione del messaggio
 * @return void* Puntatore al messaggio se la ricezione è avvenuta con successo. Se c'è un errore restituisce NULL e setta errno.
 */
void* receive_message (int file_descriptor, size_t size);

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
