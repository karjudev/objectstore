/**
 * @file pair_list.c
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Implementazione della libreria di creazione e gestione di una lista concatenata di coppie (chiave, valore).
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assertmacros.h>
#include <shared.h>

#include <hashtable/pair_list.h>

/**
 * @brief Crea un nodo della lista che contiene una coppia
 * 
 * @param key Chiave della coppia
 * @param value Valore associato alla chiave
 * @return struct node* Nodo appena creato. Se c'è un errore restituisce NULL e setta errno.
 */
static struct node* create_node (int key, char* value) {
    // Alloca la struttura dati
    struct node* new = (struct node*) malloc(sizeof(struct node));
    ASSERT_ERRNO_RETURN(new != NULL, ENOMEM, NULL);
    // Setta i dati del nodo
    new->key = key;
    new->value = strdup(value);
    // Setta i puntatori a NULL
    new->prev = new->next = NULL;
    // Restituisce il nodo
    return new;
}

/**
 * @brief Distrugge tutti i nodi della lista puntata da head
 * 
 * @param head Testa della lista da eliminare
 */
static void destroy_nodes (struct node* head) {
    // Se la testa è NULL ha finito
    if (head == NULL) return;
    // Distrugge tutti i nodi successivi
    destroy_nodes(head->next);
    // Distrugge il valore del nodo corrente
    free(head->value);
    // Distrugge il nodo corrente
    free(head);
}

/**
 * @brief Recupera il nodo di chiave key nella lista
 * 
 * @param head Testa della lista di nodi
 * @param key Chiave che identifica il nodo
 * @return struct node* Nodo identificato da key. Se il nodo non esiste restituisce NULL.
 */
static struct node* get_node (struct node* head, int key) {
    // Se il nodo corrente è NULL Non ha trovato il nodo che cercava (è in fondo alla lista o la lista è vuota)
    if (head == NULL) return NULL;
    // Se il nodo corrente ha la stessa chiave della chiave cercata lo restituisce
    if (key == head->key) return head;
    // Se non esiste un successore termina
    if (head->next == NULL) return NULL;
    // Altrimenti restituisce il risultato della ricerca nei prossimi nodi
    return get_node(head->next, key);
}

/**
 * @brief Crea una nuova lista
 * 
 * @return pair_list_t* Struttura dati che rappresenta la lista vuota
 */
pair_list_t* create_list () {
    // Inizializza la struttura dati
    pair_list_t* list = (pair_list_t*) malloc(sizeof(pair_list_t));
    ASSERT_ERRNO_RETURN(list != NULL, ENOMEM, NULL);
    // Inizializza a zero tutti i campi
    list->elements = 0;
    list->head = NULL;
    // Restituisce la lista
    return list;
}

/**
 * @brief Distrugge la lista puntata
 * 
 * @param list Lista da deallocare
 * @return int Se la lista è stata eliminata correttamente restituisce 1. Se c'è un errore restituisce -1 e setta errno.
 */
int destroy_list (pair_list_t* list) {
    // Controlla che la lista esista
    ASSERT_ERRNO_RETURN(list != NULL, EINVAL, -1);
    // Distrugge gli elementi della lista
    destroy_nodes(list->head);
    // Distrugge la struttura dati che contiene la lista
    free(list);
    // Restituisce il successo
    return 1;
}

/**
 * @brief Trova il valore associato alla chiave nella lista
 * 
 * @param list Lista in cui cercare l'elemento
 * @param key Chiave dell'elemento
 * @return char* Valore associato all'elemento. Se c'è un errore restituisce NULL e setta errno.
 */
char* get_value_list (pair_list_t* list, int key) {
    // Controlla che i parametri siano corretti
    ASSERT_ERRNO_RETURN((list != NULL) && (key >= 0), EINVAL, NULL);
    // Se la dimensione della coda è 0 non c'è niente da cercare
    ASSERT_ERRNO_RETURN(list->elements > 0, ENOKEY, NULL);
    // Cerca il nodo nella lista
    struct node* found = NULL;
    if (list->head) found = get_node(list->head, key);
    // Se il nodo non è stato trovato non esiste
    ASSERT_ERRNO_RETURN(found != NULL, ENOKEY, NULL);
    // Restituisce il valore
    return found->value;
}

/**
 * @brief Inserise una nuova coppia nella lista
 * 
 * @param list Lista in cui inserire l'elemento
 * @param key Chiave della coppia
 * @param value Valore associato alla chiave
 * @return int Se l'elemento è stato inserito correttamente restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int insert_list (pair_list_t* list, int key, char* value) {
    // Controlla la correttezza dei parametri
    ASSERT_ERRNO_RETURN((list != NULL) && (key > -1), EINVAL, -1);
    // Cerca, se esiste, il nodo corrispondente alla chiave che si vuole inserire
    if (list->elements > 0) {
        struct node* found = get_node(list->head, key);
        // Se l'elemento era già presente restituisce un errore
        ASSERT_ERRNO_RETURN(found == NULL, EALREADY, -1);
    }
    // Crea un nuovo elemento
    struct node* new = create_node(key, value);
    ASSERT_RETURN(new != NULL, -1);
    // Mette l'elemento in testa alla lista
    new->next = list->head;
    if (list->head)
        list->head->prev = new;
    list->head = new;
    // Incrementa il contatore
    list->elements++;
    // Restituisce il successo
    return 0;
}

/**
 * @brief Rimuove il nodo identificato dalla chiave
 * 
 * @param list Puntatore alla testa della lista
 * @param key Chiave che identifica la coppia
 * @return int Se la rimozione è avvenuta con successo restituisce 0. Se c'è un errore restituisce NULL e setta errno.
 */
int remove_list (pair_list_t** list, int key) {
    // Controlla la correttezza dei parametri
    ASSERT_ERRNO_RETURN(((*list) != NULL) && (key > -1), EINVAL, -1);
    // Se la dimensione è 0 non c'è niente da rimuovere
    ASSERT_ERRNO_RETURN((*list)->elements > 0, ENOKEY, -1);
    // Cerca il nodo da rimuovere
    struct node* found = get_node((*list)->head, key);
    ASSERT_RETURN(found != NULL, 0);
    // Rimuove il nodo dalla coda
    if (found->prev) found->prev->next = found->next;
    else (*list)->head = found->next;
    if (found->next) found->next->prev = found->prev;
    found->prev = found->next = NULL;
    // Libera la memoria del nodo
    destroy_nodes(found);
    // Decrementa il numero totale di nodi
    (*list)->elements--;
    // Se non ci sono più nodi invalida il riferimento alla lista
    if ((*list)->elements == 0) (*list)->head = NULL;
    // Restituisce il successo dell'operazione
    return 0;
}