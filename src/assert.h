/**
 * @file assert.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Macro che servono a verificare asserzioni e in caso contrario eseguire operazioni.
 * @version 0.1
 * @date 2019-06-11
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
    if ((!statement)) { operation; }

/**
 * @brief Se statement restituisce FALSE, setta errno = errvalue ed esegue operation.
 */
#define ASSERT_ERRNO(statement, errvalue, operation) \
    ASSERT(statement, errno = errvalue; operation)

/**
 * @brief Se statement restituisce FALSE, restituisce value.
 */
#define ASSERT_RETURN(satement, value) \
    ASSERT(statement, return value)

/**
 * @brief Se statement restituisce FALSE, setta errno = errvalue e restituisce value.
 */
#define ASSERT_ERRNO_RETURN(statement, errvalue, value) \
    ASSERT(statement, errno = errvalue; return value)

#endif // _ASSERT
