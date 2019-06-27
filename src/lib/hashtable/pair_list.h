/**
 * @file pair_list.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Header della libreria di creazione e gestione di una lista concatenata di coppie (chiave, valore).
 * @version 0.1
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#if !defined(_LIST)
#define _LIST

/**
 * @brief Nodo di una lista concatenata con coppia (chiave, valore) e puntatore al prossimo elemento. Gli elementi hanno chiavi tutte distinte.
 * 
 */
struct node {
    char* key;
    int value;
    struct node* next;
    struct node* prev;
};

typedef struct list {
    int elements;
    struct node* head;
} pair_list_t;

/**
 * @brief Crea una nuova lista
 * 
 * @return pair_list_t* Struttura dati che rappresenta la lista vuota
 */
pair_list_t* create_list ();

/**
 * @brief Distrugge la lista puntata
 * 
 * @param list Lista da deallocare
 * @return int Se la lista è stata eliminata correttamente restituisce 1. Se c'è un errore restituisce -1 e setta errno.
 */
int destroy_list (pair_list_t* list);

/**
 * @brief Trova il valore associato alla chiave nella lista
 * 
 * @param list Lista in cui cercare l'elemento
 * @param key Chiave dell'elemento
 * @return int Valore associato all'elemento. Se non esiste restituisce -1.
 */
int get_value_list (pair_list_t* list, char* key);

/**
 * @brief Inserise una nuova coppia nella lista
 * 
 * @param list Lista in cui inserire l'elemento
 * @param key Chiave della coppia
 * @param value Valore associato alla chiave
 * @return int Se l'elemento è stato inserito correttamente restituisce 1. Se l'elemento era già presente restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int insert_list (pair_list_t* list, char* key, int value);

/**
 * @brief Rimuove il nodo identificato dalla chiave
 * 
 * @param list Puntatore alla testa della lista
 * @param key Chiave che identifica la coppia
 * @return int Valore che era associato a key nella lista. Se l'elemento non era presente restituisce -1. Se c'è un errore restituisce -1 e setta errno.
 */
int remove_list (pair_list_t* list, char* key);

#endif // _LIST
