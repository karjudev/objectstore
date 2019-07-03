/**
 * @file osclient.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Header file della libreria che fornisce i metodi di gestione per una lista concatenata di pthread_t.
 * @version 0.1
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

int insert_pthread_list (pthread_list_t** head, pthread_t thread_id);

pthread_t remove_pthread_list_head (pthread_list_t** head);

#endif // _PTHREAD_LIST
