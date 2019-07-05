/**
 * @file assertmacros.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Macro che servono a verificare asserzioni e in caso contrario eseguire operazioni e stampare messaggi.
 * @version 0.1
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#if !defined(_ASSERT)
#define _ASSERT

/**
 * @brief Se statement restituisce FALSE, esegue operation.
 */
#define ASSERT(statement, operation) \
    if (!(statement)) { \
        operation; \
    }

/**
 * @brief Se statement restituisce FALSE, setta errno = errvalue ed esegue operation.
 */
#define ASSERT_ERRNO(statement, errvalue, operation) \
    ASSERT(statement, errno = errvalue; operation)

/**
 * @brief Se statement restituisce FALSE, stampa message ed esegue operation
 */
#define ASSERT_MESSAGE(statement, message, operation) \
    ASSERT(statement, perror(message); operation)

/**
 * @brief Se statement restituisce FALSE, restituisce value.
 */
#define ASSERT_RETURN(statement, value) \
    ASSERT(statement, return value)

/**
 * @brief Se statement restituisce FALSE, setta errno = errvalue e restituisce value.
 */
#define ASSERT_ERRNO_RETURN(statement, errvalue, value) \
    ASSERT(statement, errno = errvalue; return value)

/**
 * @brief Se statement restituisce FALSE, stampa message e restituisce value
 * 
 */
#define ASSERT_MESSAGE_RETURN(statement, message, value) \
    ASSERT_MESSAGE(statement, message, return value)

/**
 * @brief Se statement restituisce FALSE stampa message, setta errno = errvalue ed esegue operation
 * 
 */
#define ASSERT_MESSAGE_ERRNO(statement, message, errvalue, operation) \
    ASSERT(statement, perror(message); errno = errvalue; operation)

/**
 * @brief Se statement restituisce FALSE, stampa message, setta errno = errval e restituisce value.
 */
#define ASSERT_MESSAGE_ERRNO_RETURN(statement, message, errval, value) \
    ASSERT(statement, perror(message); errno = errval; return value)

#endif // _ASSERT
