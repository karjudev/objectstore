/**
 * @file hashtable.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Implementazione della libreria di creazione e gestione di una tabella hash di coppie (stringa, intero) memorizzata con liste di trabocco.
 * @version 0.1
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include <assertmacros.h>
#include <mutexmacros.h>

#include <hashtable/pair_list.h>

#include <hashtable/hashtable.h>

#define HASHTABLE_SIZE 100
#define MUTEX_NUMBER 16

// Array di lock che partizionano l'accesso alla tabella
static pthread_mutex_t mutexes[MUTEX_NUMBER];

/**
 * @brief Funzione hash che usa l'algoritmo djb2 (http://www.cse.yorku.ca/~oz/hash.html)
 * 
 * @param key Chiave di cui calcolare l'hash
 * @return int Hash della chiave
 */
static int hash_function (char* key) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++))
        hash = ((hash << 5) + hash) + c;
    return (hash % HASHTABLE_SIZE);
}

/**
 * @brief Crea una tabella hash
 * 
 * @return hashtable_t* Tabella appena creata. Se c'è un errore restituisce NULL e setta errno.
 */
hashtable_t* create_hashtable () {
    // Inizializza la struttura dati della tabella
    hashtable_t* table = (hashtable_t*) malloc(sizeof(hashtable_t));
    ASSERT_RETURN(table != NULL, NULL);
    // Inizializza l'array delle liste di trabocco
    table->entries = (pair_list_t**) calloc(HASHTABLE_SIZE, sizeof(pair_list_t*));
    ASSERT_ERRNO(table->entries != NULL, ENOMEM, free(table); return NULL);
    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        table->entries[i] = create_list();
        ASSERT(table->entries[i] != NULL, free(table); return NULL);
    }
    // Inizializza le variabili di condizione
    int success;
    for (int i = 0; i < MUTEX_NUMBER; i++) {
        success = pthread_mutex_init(&mutexes[i], NULL);
        ASSERT_ERRNO(success == 0, success, destroy_hashtable(table); return NULL);
    }
    // Inizializza i campi della tabella
    table->elements = 0;
    // Restituisce la tabella
    return table;
}

/**
 * @brief Libera la memoria occupata dalla tabella passata
 * 
 * @param table Tabella da eliminare
 * @return int Se l'eliminazione è avvenuta correttamente restituisce 1. Se c'è un errore restituisce -1 e setta errno.
 */
int destroy_hashtable (hashtable_t* table) {
    // Controlla che la tabella esista
    ASSERT_ERRNO_RETURN(table != NULL, EINVAL, -1);
    // Elimina tutte le liste di trabocco
    int success;
    for (int i = 0; i < HASHTABLE_SIZE; i++)
        if (table->entries[i]) {
            success = destroy_list(table->entries[i]);
            ASSERT_RETURN(success != -1, -1);
        }
    free(table->entries);
    // Elimina le vaiabili di condizione
    for (int i = 0; i < 16; i++) {
        success = pthread_mutex_destroy(&mutexes[i]);
        ASSERT_ERRNO_RETURN(success == 0, success, -1);
    }
    // Libera tutta la struttura dati
    free(table);
    // Restituisce il successo
    return 1;
}

/**
 * @brief Inserisce in mutua esclusione un nuovo elemento nella tabella hash
 * 
 * @param table Tabella in cui inserire l'elemento
 * @param key Chiave che identifica l'elemento
 * @param value Valore associato all'elemento
 * @return int Se l'elemento è stato inserito con successo restituisce 1. Se un elemento con la stessa chiave esiste già restituisce 0. Se c'è un errore restituisce -1.
 */
int insert_hashtable (hashtable_t* table, char* key, int value) {
    // Controlla la correttezza dei parametri
    ASSERT_ERRNO_RETURN(table != NULL, EINVAL, -1);
    ASSERT_ERRNO_RETURN(key != NULL, EINVAL, -1);
    ASSERT_ERRNO_RETURN(value > -1, EINVAL, -1);
    // Calcola l'hash dell'elemento
    int hash = hash_function(key);
    // Prende la lock associata all'elemento
    pthread_mutex_t* mutex = &(mutexes[hash / MUTEX_NUMBER]);
    LOCK_ACQUIRE(mutex, return -1);
    // Inserisce, se esiste, l'elemento nella tabella
    int success = insert_list(table->entries[hash], key, value);
    // Rilascia la lock
    LOCK_RELEASE(mutex, return -1);
    // Incrementa il numero di elementi nella tabella
    if (success == 1) table->elements++;
    // Restituisce il successo dell'operazione
    return success;
}

/**
 * @brief Rimuove in mutua esclusione un elemento dalla tabella
 * 
 * @param table Tabella da cui rimuovere l'elemento
 * @param key Chiave che identifica l'elemento
 * @return int Valore che era associato alla chiave. Se l'elemento identificato da key non esiste restituisce -1. Se c'è un errore restituisce -1 e setta errno.
 */
int remove_hashtable (hashtable_t* table, char* key) {
    // Controlla la correttezza dei parametri
    ASSERT_ERRNO_RETURN((table != NULL) && (key != NULL), EINVAL, -1);
    // Calcola l'hash della chiave come indice della tabella
    int hash = hash_function(key);
    // Prende la lock associata all'elemento
    pthread_mutex_t* mutex = &(mutexes[hash / MUTEX_NUMBER]);
    LOCK_ACQUIRE(mutex, return -1);
    // Rimuove l'elemento nella lista
    int value = remove_list(table->entries[hash], key);
    // Rilascia la lock associata all'elemento
    LOCK_RELEASE(mutex, return -1);
    // Restituisce il valore associato alla chiave nella lista
    return value;
}