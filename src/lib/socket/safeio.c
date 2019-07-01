#include <unistd.h>
#include <errno.h>

#include <assertmacros.h>

#include <socket/safeio.h>

/**
 * @brief Legge n bytes dal file descriptor, inserendoli nel buffer.
 *
 * @param file_descriptor File descriptor da cui leggere
 * @param buffer Buffer in cui scrivere i bytes letti
 * @param n Numero di bytes da leggere
 * @param size_t Numero di bytes effettivamente letti, -1 se c'è un errore
 */
size_t readn (int file_descriptor, void* buffer, size_t n) {
	// Controlla la correttezza dei parametri
	ASSERT_ERRNO_RETURN((file_descriptor > 0) && (buffer != NULL) && (n > 0), EINVAL, -1);
	// Numero di bytes rimasti
	size_t nleft = n;
	// Numero di bytes letti ad ogni iterazione
	size_t nread;
	// Puntatore che si sposta all'interno del buffer
	char* ptr = buffer;
	// Continua a scorrere finché non ha letto tutti i bytes
	while (nleft > 0) {
		// Legge un insieme di bytes
		nread = read(file_descriptor, ptr, nleft);
		// Se la read ha letto un numero negativo di bytes ci potrebbe essere stato un errore
		if (nread < 0) {
			// Se la read è stata interrotta deve essere ritentata
			if (errno == EINTR) nread = 0;
			// Altrimenti l'errore non è gestibile
			else return (-1);
		}
		// Se la read ha letto 0 bytes il client ha terminato la connessione
		else if (nread == 0) break;
		// Decrementa il numero di bytes ancora da leggere
		nleft -= nread;
		// Sposta il puntatore per leggere il prossimo insieme di bytes
		ptr += nread;
	}
	// Restituisce il numero di bytes effettivamente letti
	return (n - nleft);
}

/**
 * @brief Scrive n bytes sul file descriptor prendendoli dal buffer.
 *
 * @param file_descriptor File descriptor su cui scrivere
 * @param buffer Buffer da cui prendere i dati
 * @param n Numero di bytes da scrivere
 * @return size_t n se l'operazione è completata con successo, -1 se c'è un errore
 */
size_t writen (int file_descriptor, const void *buffer, size_t n) {
	// Controlla la correttezza degli argomenti
	ASSERT_ERRNO_RETURN((file_descriptor > 0) && (n > 0), EINVAL, -1);
	// Numero di bytes rimasti da leggere
	size_t nleft = n;
	// Numero di bytes scritti ad ogni iterazione
	size_t nwritten;
	// Buffer che si sposta all'interno del buffer
	const char* ptr = buffer;
	// Continua finché non ha scritto tutti i bytes
	while (nleft > 0) {
		// Scrive uno stock di bytes
		nwritten = write(file_descriptor, ptr, nleft);
		// Se è stato scritto un numero non nullo di bytes ci potrebbe essere stato un errore
		if (nwritten <= 0) {
			// Se la scrittura è stata interrotta si può ripetere
			if (nwritten < 0 && errno == EINTR) nwritten = 0;
			// Altrimenti l'errore non è recuperabile
			else return (-1);
		}
		// Decrementa il numero di bytes rimasti
		nleft -= nwritten;
		// Sposta il puntatore dentro il buffer
		ptr += nwritten;
	}
	// Restituisce il numero di bytes scritti perché è andato tutto bene
	return n;
}