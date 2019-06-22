/**
 * @file mutex.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Macro che contiene i valori condivisi tra client e server.
 * @version 0.1
 * @date 2019-06-11
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */
#if !defined(_SHARED)
#define _SHARED

// Nome del socket condiviso tra server e client
#define SOCKET_NAME "./tmp.sock"

// Header di lunghezza massima dato da verbo di lunghezza massima + massima dimensione di un nome di file POSIX + spazio + \n + \0
#define MAX_HEADER_LENGTH (strlen("RETRIEVE") + 255 + 3)

// Macro che testa se i primi n bytes dell stringa b sono uguali ai primi n bytes della stringa a
#define EQUALS(a, b, n) (strncmp(a, b, n) == 0)

#endif // _SHARED