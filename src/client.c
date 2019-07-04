#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assertmacros.h>

#include <socket/socket.h>
#include <os_client/os_client.h>

#include <shared.h>

/**
 * @brief Tipo di dato grande 1 byte
 */
typedef unsigned char byte;

/**
 * @brief Crea un array di bytes grande size
 * 
 * @param size Numero di bytes
 * @return byte* Array di bytes di dimensione size. Se c'è un errore restituisce NULL e setta errno.
 */
static byte* create_test_array (size_t size) {
    byte* array = (byte*) malloc(size);
    ASSERT_ERRNO_RETURN(array != NULL, ENOMEM, NULL);
    for (int i = 0; i < 100000; i++) array[i] = (byte) i;
    return array;
}

/**
 * @brief Confronta due array di byte
 * 
 * @param array_a Primo array
 * @param array_b Secondo array
 * @param size Lunghezza dell'array da confrontare
 * @return int Se i due array hanno gli stessi elementi restituisce 1, altrimenti 0.
 */
static int data_corresponding (byte* array_a, byte* array_b, size_t size) {
    for (int i = 0; i < size; i++)
        if (array_a[i] != array_b[i])
            return 0;
    return 1;
}

/**
 * @brief Memorizza 20 blocchi di dati da 100B a 100KB, come sequenze di interi consecutivi
 * 
 * @return Se l'operazione è andata a buon fine restituisce 0. Se c'è un errore restituisce un codice di errore.
 */
static int store_data () {
    // Variabile che contiene il nome del blocco
    char name[2] = "A";
    // Buffer da 100KB
    byte* array = create_test_array(100000);
    ASSERT_RETURN(array != NULL, 1);
    // Invia 20 messaggi in ordine crescente di dimensione
    int size = 100;
    int step = (100000 - 100) / 20;
    for (int i = 0; i < 19; i++) {
        ASSERT(os_store(name, array, size) == 1, free(array); return errno);
        size += step;
        name[0]++;
    }
    ASSERT(os_store(name, array, 100000) == 1, free(array); return errno);
    // Libera la memoria occupata dall'array
    free(array);

    return 0;
}

/**
 * @brief Compara un blocco di dati ricevuto dal server con un pezzo di array
 * 
 * @param name Nome del blocco da reperire dal server
 * @param array Array di bytes per il confronto
 * @param size Dimensione dei dati da confrontare
 * @return int Se i due array sono uguali restituisce 0. Se c'è un errore restituisce un codice di errore.
 */
static int compare_data (char* name, byte* array, int size) {
    // Richiede i dati al server
    byte* data = os_retrieve(name);
    ASSERT(data != NULL, free(data); return 1);
    // Verifica che i dati siano uguali
    ASSERT(data_corresponding(data, array, size), free(data); return EIO);
    // Libera la memoria occupata dai dati appena ricevuti
    free(data);
    // Stampa un messaggio di log
    return 0;
}

/**
 * @brief Recupera 20 blocchi di dati da 100B a 100KB e verifica che siano una sequenza di interi consecutivi
 * 
 * @return Se l'operazione è andata a buon fine restituisce 0. Se c'è un errore restituisce 1 e setta errno.
 */
static int retrieve_data () {
    // Byte array di prova
    byte* array = create_test_array(100000);
    // Step con cui aumenta la dimensione dei dati
    int step = (100000 - 100) / 20;
    // Nome della risorsa
    char name[2] = "A";
    // Dimensione della risorsa
    int size = 100;
    for (int i = 0; i < 19; i++) {
        ASSERT(compare_data(name, array, size) == 0, free(array); return errno);
        // Incrementa la dimensione del prossimo blocco
        size += step;
        // Cambia il nome del prossimo blocco
        name[0]++;
    }
    ASSERT_RETURN(compare_data(name, array, 100000) == 0, 1);
    // Libera la memoria occupata dall'array di prova
    free(array);
    
    return 0;
}

/**
 * @brief Cancella 20 blocchi di dati
 * 
 * @return Se l'operazione è andata a buon fine restituisce 0. Se c'è un errore restituisce 1 e setta errno.
 */
static int delete_data () {
    // Nome del blocco
    char name[2] = "A";
    for (int i = 0; i < 20; i++) {
        // Cancella il blocco
        ASSERT_RETURN(os_delete(name) == 1, errno);
        // Incrementa il nome
        name[0]++;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    // Controlla che sia stato passato il corretto numero di argomenti
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <USER_NAME> <TEST_NUMBER>\n", argv[0]);
        exit(1);
    }
    // Nome del client
    char* name = strdup(argv[1]);
    // Numero di test da effettuare
    int test_number = strtol(argv[2], NULL, 10);
    // Controlla che il numero sia corretto
    if ((test_number < 1) || (test_number > 3)) {
        fprintf(stderr, "Test number must be between 1 and 3\n");
        exit(1);
    }
    // Si connette al server con il nome scelto
    ASSERT(os_connect(name) == 1, free(name); return 1);
    // Distingue il tipo di test
    int error = 0;
    if (test_number == 1)
        error = store_data();
    else if (test_number == 2)
        error = retrieve_data();
    else if (test_number == 3)
        error = delete_data();
    // Se l'operazione si è conclusa con successo lo stampa
    if (!error)
        printf("[%s] Test %d: Success\n", name, test_number);
    else {
        fprintf(stderr, "[%s] Test %d: %s\n", name, test_number, strerror(error));
    }
    // Libera la memoria occupata dal nome
    free(name);
    // Si disconnette
    ASSERT(os_disconnect() == 1, return errno);
    // Restituisce il successo dell'operazione performata
    return error;
}
