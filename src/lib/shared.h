/**
 * @file mutex.h
 * @author Giacomo Mariani, Matricola 545519, Corso B
 * @brief Header che contiene i valori condivisi tra client e server.
 * @version 0.1
 * @date 2019-06-11
 * 
 * Si dichiara che tutto il codice è stato realizzato dallo studente.
 * 
 */
#if !defined(_SHARED)
#define _SHARED

// Nome del socket condiviso tra server e client
#define SOCKET_NAME "./objstore.sock"

// Nome della cartella dati
#define DATA_DIRECTORY "./data"

// Lunghezza massima di un header che contiene un comando dato da verbo di lunghezza massima ("RETRIEVE") + massima dimensione di un nome di file POSIX (255) + due spazi + \n + \0
#define MAX_HEADER_LENGTH 267

// Lunghezza massima di una risposta di tipo diverso dai dati, costituito da "KO <er> \n"
#define MAX_RESPONSE_LENGTH 8

// Lunghezza massima della stringa "DATA <length> \n ", con la lunghezza massima di length data da 2^64 (20 cifre)
#define MAX_DATA_LENGTH 29

// Macro che testa se i primi n bytes dell stringa b sono uguali ai primi n bytes della stringa a
#define EQUALS(a, b) (strcmp(a, b) == 0)

#endif // _SHARED