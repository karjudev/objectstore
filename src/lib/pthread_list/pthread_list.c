/**
 * @file pthread_list.c
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Implementazione della libreria di creazione e gestione di una lista linkata di pthread_t.
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include <assertmacros.h>

#include <pthread_list/pthread_list.h>

/**
 * @brief Inserisce un pthread_t in testa alla lista, modificando la testa della lista.
 * 
 * @param head Puntatore alla testa della lista attuale.
 * @param thread_id Identificatore del thread
 * @return int Se l'inserimento è andato a buon fine modifica la testa della lista aggiungendo il nuovo nodo e restitusce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int insert_pthread_list (pthread_list_t** head, pthread_t thread_id) {
    // Crea un nuovo nodo
    pthread_list_t* new = (pthread_list_t*) malloc(sizeof(pthread_list_t));
    ASSERT_ERRNO_RETURN(new != NULL, ENOMEM, -1);
    // Setta il campo dati del nodo
    new->thread_id = thread_id;
    // Mette il nuovo nodo in testa
    new->next = *head;
    // Modifica la testa
    *head = new;
    // Restituisce il successo
    return 0;
}

/**
 * @brief Rimuove un nodo dalla testa della lista, modificando la testa della lista.
 * 
 * @param head Testa della lista corrente. Viene modificata con il successore oppure NULL
 * @return pthread_t Se la rimozione è andata a buon fine restituisce il valore associato al nodo eliminato. Se non esiste nessun nodo restituisce 0.
 */
pthread_t remove_pthread_list_head (pthread_list_t** head) {
    // Se la lista è vuota restituisce 0
    if (*head == NULL) return 0;
    // Rimuove la testa della lista
    pthread_list_t* old_head = *head;
    if ((*head)->next) (*head) = (*head)->next;
    else (*head) = NULL;
    // Valore del thread id
    pthread_t thread_id = old_head->thread_id;
    // Libera il nodo
    free(old_head);
    // Restituisce il valore del nodo
    return thread_id;
}