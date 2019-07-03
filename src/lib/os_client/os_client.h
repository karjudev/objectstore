/**
 * @file osclient.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Header file della libreria di connessione al server object store.
 * @version 0.1
 * @date 2019-06-11
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#if !defined(_CLIENT)
#define _CLIENT

/**
 * @brief Inizializza la connessione globale con il server.
 * 
 * @param name Nome dell'utente da connettere
 * @return int 1 se la connessione è andata a buon fine. Se c'è un errore restituisce 0 e setta errno.
 */
int os_connect (char* name);

/**
 * @brief Memorizza sul server l'area di memoria di lunghezza len puntata da block.
 * 
 * @param name Nome del blocco da memorizzare
 * @param block Dati del blocco da memorizzare
 * @param len Lunghezza del blocco da memorizzare
 * @return int 1 se la memorizzazione è andata a buon fine. Se c'è un errore restituisce 0 e setta errno.
 */
int os_store (char* name, void* block, size_t len);

/**
 * @brief Recupera il blocco di dati identificato da name.
 * 
 * @param name Nome del blocco di dati.
 * @return void* Blocco di dati se il recupero ha avuto successo. Se c'è un errore restituisce NULL e setta errno.
 */
void* os_retrieve (char* name);

/**
 * @brief Cancella il blocco di dati identificato da name
 * 
 * @param name Nome del blocco di dati
 * @return int 1 se l'eliminazione è avvenuta con successo. Se c'è un errore restituisce 0 e setta errno.
 */
int os_delete (char* name);

/**
 * @brief Si disconnette dal server
 * 
 * @return int 1 se la disconnessione è avvenuta con successo. Se c'è un errore restituisce 0 e setta errno.
 */
int os_disconnect();

#endif // _CLIENT
