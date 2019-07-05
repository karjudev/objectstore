/**
 * @file pthread_list.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Header file della libreria che fornisce i metodi di gestione per una lista concatenata di pthread_t.
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#if !defined(_PTHREAD_LIST)
#define _PTHREAD_LIST

/**
 * @brief Nodo di una lista concatenata
 * 
 */
typedef struct node {
    struct node* next;
    pthread_t thread_id;
} pthread_list_t;

/**
 * @brief Inserisce un pthread_t in testa alla lista, modificando la testa della lista.
 * 
 * @param head Puntatore alla testa della lista attuale.
 * @param thread_id Identificatore del thread
 * @return int Se l'inserimento è andato a buon fine modifica la testa della lista aggiungendo il nuovo nodo e restitusce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int insert_pthread_list (pthread_list_t** head, pthread_t thread_id);

/**
 * @brief Rimuove un nodo dalla testa della lista, modificando la testa della lista.
 * 
 * @param head Testa della lista corrente. Viene modificata con il successore oppure NULL
 * @return pthread_t Se la rimozione è andata a buon fine restituisce il valore associato al nodo eliminato. Se non esiste nessun nodo restituisce 0.
 */
pthread_t remove_pthread_list_head (pthread_list_t** head);

#endif // _PTHREAD_LIST
