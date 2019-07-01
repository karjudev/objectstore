/**
 * @file workers.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Header delle funzioni che svolgono il lavoro del server.
 * @version 0.1
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#if !defined(_WORKERS)
#define _WORKERS

/**
 * @brief Inizializza le strutture dati necessarie alle funziioni.
 * 
 * @return int Se l'inizializzazione è andata a buon fine restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int init_worker_functions ();

/**
 * @brief Libera la memoria occupata dalle strutture dati necessarie alle funzioni.
 * 
 * @return int Se l'eliminazione è andata a buon fine restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int stop_worker_functions ();

/**
 * @brief Registra un nuovo utente creando la sua cartella su disco
 * 
 * @param client_fd File descriptor del client
 * @param name Nome utente del client
 * @return int Se il client è stato registrato correttamente restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int register_user (int client_fd, char* name);

/**
 * @brief Scrive un nuovo blocco nel file con lo stesso nome.
 * 
 * @param client_fd File descriptor del client
 * @param name Nome del blocco da scrivere
 * @param data Blocco di dati da scrivere
 * @param size Dimensione dei dati
 * @return int Se il blocco è stato scritto correttamente restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int store_block (int client_fd, char* name, void* data, size_t size);

/**
 * @brief Recupera un blocco di dati del client dal disco
 * 
 * @param client_fd File descriptor del client
 * @param name Nome del blocco da recuperare
 * @param size_ptr Puntatore alla dimensione del blocco, il cui valore puntato viene settato dalla funzione
 * @return void* Blocco di dati identificato dal nome. Se c'è un errore restituisce NULL e setta errno.
 */
void* retrieve_block (int client_fd, char* name, size_t* size_ptr);

/**
 * @brief Rimuove dal disco un blocco di dati dell'utente
 * 
 * @param client_fd File descriptor dell'utente
 * @param name Nome del blocco da rimuovere
 * @return int Se il blocco è stato rimosso con successo restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int delete_block (int client_fd, char* name);

/**
 * @brief Cancella il client dal sistema
 * 
 * @param client_fd File descriptor di client
 */
void leave_client (int client_fd);

#endif // _WORKERS
