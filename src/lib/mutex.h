/**
 * @file mutex.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Macro che servono a dirottare su errno gli eventuali errori che avvengono nella gestione di mutex e variabili di condizione.
 * @version 0.1
 * @date 2019-06-11
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

/**
 * @brief Struttura base: Viene eseguito statement, assegnando il risultato a errno. Se il risultato non è 0 esegue operation.
 * 
 */
#define _BASE_STRUCTURE(statement, operation) \
    errno = statement; \
    if (errno) { operation; }

/**
 * @brief Se non riesce ad acquisire lock esegue operation.
 * 
 */
#define LOCK_ACQUIRE(lock, operation) \
    _BASE_STRUCTURE(pthread_mutex_lock(lock), operation);

/**
 * @brief Se non riesce ad acquisire lock restituisce value.
 * 
 */
#define LOCK_ACQUIRE_RETURN(lock, value) \
    LOCK_ACQUIRE(lock, return value)

/**
 * @brief Se non riesce a rilasciare lock esegue operation.
 * 
 */
#define LOCK_RELEASE(lock, operation) \
    _BASE_STRUCTURE(pthread_mutex_unlock(lock), operation)

/**
 * @brief Se non riesce a rilasciare lock restituisce value.
 * 
 */
#define LOCK_RELEASE_RETURN(lock, value) \
    LOCK_RELEASE(lock, return value)

/**
 * @brief Se non riesce ad attendere su cond esegue operation.
 * 
 */
#define WAIT(cond, lock, operation) \
    _BASE_STRUCTURE(pthread_cond_wait(cond, lock), operation)

/**
 * @brief Se non riesce ad attendere su cond restituisce value.
 * 
 */
#define WAIT_RETURN(cond, lock, value) \
    WAIT(cond, lock, return value)

/**
 * @brief Se non riesce a segnalare su cond esegue operation.
 * 
 */
#define SIGNAL(cond, operation) \
    _BASE_STRUCTURE(pthread_cond_signal(cond), operation)

/**
 * @brief Se non riesce a segnalare su cond restituisce value.
 * 
 */
#define SIGNAL_RETURN(cond, value) \
    SIGNAL(cond, return value)