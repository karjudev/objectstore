/**
 * @file hashtable.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Header della libreria di creazione e gestione di una tabella hash di coppie (intero, stringa) memorizzata con liste di trabocco.
 * La libreria usa 16 lock per partizionare la tabella e garantire l'accesso in mutua esclusione alla tabella.
 * @version 0.1
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */#include <stdio.h>


#if !defined(_HASHTABLE)
#define _HASHTABLE

#include <hashtable/pair_list.h>

/**
 * @brief Tabella hash con dimensione della tabella, numero di entries piene e array di liste di trabocco.
 */
typedef struct hashtable {
    int elements;
    pair_list_t** entries;
} hashtable_t;

/**
 * @brief Crea una tabella hash
 * 
 * @return hashtable_t* Tabella appena creata. Se c'è un errore restituisce NULL e setta errno.
 */
hashtable_t* create_hashtable ();

/**
 * @brief Libera la memoria occupata dalla tabella passata
 * 
 * @param table Tabella da eliminare
 * @return int Se l'eliminazione è avvenuta correttamente restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int destroy_hashtable (hashtable_t* table);

/**
 * @brief Inserisce in mutua esclusione un nuovo elemento nella tabella hash
 * 
 * @param table Tabella in cui inserire l'elemento
 * @param key Chiave che identifica l'elemento
 * @param value Valore associato all'elemento
 * @return int Se l'elemento è stato inserito con successo restituisce 0. Se un elemento con la stessa chiave esiste già restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int insert_hashtable (hashtable_t* table, int key, char* value);

/**
 * @brief Rimuove in mutua esclusione un elemento dalla tabella
 * 
 * @param table Tabella da cui rimuovere l'elemento
 * @param key Chiave che identifica l'elemento
 * @return int Se la rimozione è avvenuta con successo restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int remove_hashtable (hashtable_t* table, int key);

/**
 * @brief Prende il valore associato alla chiave nella tabella.
 * 
 * @param table Tabella da cui recuperare l'elemento
 * @param key Chiave che identifica l'elemento
 * @return char* Valore associato alla chiave nella tabella. Se c'è un errore restituisce NULL e setta errno.
 */
char* retrieve_hashtable (hashtable_t* table, int key);

#endif // _HASHTABLE
