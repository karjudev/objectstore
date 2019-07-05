/**
 * @file pair_list.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Header della libreria di creazione e gestione di una lista concatenata di coppie (chiave, valore) di tipo (intero, stringa).
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
    int key;
    char* value;
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
 * @return int Se la lista è stata eliminata correttamente restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int destroy_list (pair_list_t* list);

/**
 * @brief Trova il valore associato alla chiave nella lista
 * 
 * @param list Lista in cui cercare l'elemento
 * @param key Chiave dell'elemento
 * @return char* Valore associato all'elemento. Se c'è un errore restituisce NULL e setta errno.
 */
char* get_value_list (pair_list_t* list, int key);

/**
 * @brief Inserise una nuova coppia nella lista
 * 
 * @param list Lista in cui inserire l'elemento
 * @param key Chiave della coppia
 * @param value Valore associato alla chiave
 * @return int Se l'elemento è stato inserito correttamente restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int insert_list (pair_list_t* list, int key, char* value);

/**
 * @brief Rimuove il nodo identificato dalla chiave
 * 
 * @param list Puntatore alla testa della lista
 * @param key Chiave che identifica la coppia
 * @return int Se la rimozione è avvenuta con successo restituisce 0. Se c'è un errore restituisce NULL e setta errno.
 */
int remove_list (pair_list_t** list, int key);

/**
 * @brief Rimuove il nodo di testa della lista
 * 
 * @param list Lista da cui rimuovere la testa
 * @return struct node* Nodo di testa della lista. Se c'è un errore restituisce NULL e setta errno.
 */
struct node* remove_head (pair_list_t* list);

#endif // _LIST
