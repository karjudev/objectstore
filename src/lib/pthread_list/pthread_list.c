#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include <assertmacros.h>

#include <pthread_list/pthread_list.h>

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

pthread_t remove_pthread_list_head (pthread_list_t** head) {
    // Se la lista è vuota restituisce 0
    if (*head == NULL) return 0;
    // Rimuove la testa della lista
    pthread_list_t* old_head = *head;
    (*head) = (*head)->next;
    // Valore del thread id
    pthread_t thread_id = old_head->thread_id;
    // Libera il nodo
    free(old_head);
    // Restituisce il valore del nodo
    return thread_id;
}