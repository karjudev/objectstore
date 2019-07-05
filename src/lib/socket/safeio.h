/**
 * @file socket.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Implementazione libreria che fornisce i metodi per la scrittura e la lettura di un dato numero di bytes.
 * @version 0.1
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */

#if !defined(_SAFE_IO)
#define _SAFE_IO

/**
 * @brief Legge n bytes dal file descriptor, inserendoli nel buffer.
 *
 * @param file_descriptor File descriptor da cui leggere
 * @param buffer Buffer in cui scrivere i bytes letti
 * @param n Numero di bytes da leggere
 * @param size_t Numero di bytes effettivamente letti, -1 se c'è un errore
 */
size_t readn (int file_descriptor, void* buffer, size_t n);

/**
 * @brief Scrive n bytes sul file descriptor prendendoli dal buffer.
 *
 * @param file_descriptor File descriptor su cui scrivere
 * @param buffer Buffer da cui prendere i dati
 * @param n Numero di bytes da scrivere
 * @return size_t n se l'operazione è completata con successo, -1 se c'è un errore
 */
size_t writen (int file_descriptor, const void *buffer, size_t n);

#endif // _SAFE_IO
