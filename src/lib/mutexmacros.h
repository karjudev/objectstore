/**
 * @file mutexmacros.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Macro che servono a .
 * @version 0.1
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#if !defined(_MUTEX)
#define _MUTEX

/**
 * @brief Cerca di acquisire la lock. Se non ci riesce setta errno ed esegue operation.
 */
#define LOCK_ACQUIRE(lock_ptr, operation) \
    ASSERT((errno = pthread_mutex_lock(lock_ptr)) == 0, operation);

/**
 * @brief Cerca di rilasciare la lock. Se non ci riesce setta errno ed esegue operation.
 */
#define LOCK_RELEASE(lock_ptr, operation) \
    ASSERT((errno = pthread_mutex_unlock(lock_ptr)) == 0, operation)

#endif // _MUTEX
