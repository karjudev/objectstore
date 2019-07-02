#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assertmacros.h>

#include <socket/socket.h>
#include <osclient/osclient.h>

#include <shared.h>

/**
 * @brief Tipo di dato grande 1 byte
 */
typedef unsigned char byte;

static byte* create_test_array (size_t size) {
    byte* array = (byte*) malloc(size);
    ASSERT_ERRNO_RETURN(array != NULL, ENOMEM, NULL);
    for (int i = 0; i < 100000; i++) array[i] = (byte) i;
    return array;
}

static int data_corresponding (byte* array_a, byte* array_b, size_t size) {
    for (int i = 0; i < size; i++)
        if (array_a[i] != array_b[i])
            return 0;
    return 1;
}

/**
 * @brief Memorizza 20 blocchi di dati da 100B a 100KB, come sequenze di interi consecutivi
 * 
 * @return Se l'operazione è andata a buon fine restituisce 0. Se c'è un errore restituisce 1 e setta errno.
 */
static int store_data () {
    // Variabile che contiene il nome del blocco
    char name[2] = "A";
    // Buffer da 100KB
    byte* array = create_test_array(100000);
    ASSERT_MESSAGE_RETURN(array != NULL, "Allocating test array", 1);
    // Invia 20 messaggi in ordine crescente di dimensione
    int size = 100;
    int step = (100000 - 100) / 20;
    for (int i = 0; i < 19; i++) {
        ASSERT_MESSAGE(os_store(name, array, size) == 1, "Storing message", free(array); return 1);
        printf("Stored block %s of size %d\n", name, size);
        size += step;
        name[0]++;
    }
    ASSERT_MESSAGE(os_store(name, array, 100000) == 1, "Storing message", free(array); return 1);
    printf("Stored block %s of size 100000\n", name);
    // Libera la memoria occupata dall'array
    free(array);

    return 0;
}

static int compare_data (char* name, byte* array, int size) {
    // Richiede i dati al server
    byte* data = os_retrieve(name);
    ASSERT_MESSAGE(data != NULL, "Getting data", free(data); return 1);
    // Verifica che i dati siano uguali
    ASSERT_MESSAGE(data_corresponding(data, array, size), "Data are not corresponding", free(data); return 1);
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
        ASSERT(compare_data(name, array, size) == 0, free(array); return 1);
        printf("Successifully retrieved block %s of size %d\n", name, size);    
        // Incrementa la dimensione del prossimo blocco
        size += step;
        // Cambia il nome del prossimo blocco
        name[0]++;
    }
    ASSERT_RETURN(compare_data(name, array, 100000) == 0, 1);
    printf("Successifully retrieved block %s of size 100000\n", name);
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
        ASSERT_MESSAGE(os_delete(name) == 1, "Deleting data", return 1);
        // Stampa un messaggio di log
        printf("Deleted file %s\n", name);
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
    ASSERT_MESSAGE(os_connect(name) == 1, "Connecting to socket", return 1);
    printf("Connected with name %s\n", name);
    // Libera la memoria occupata dal nome
    free(name);
    // Distingue il tipo di test
    int success = 0;
    if (test_number == 1)
        success = store_data();
    else if (test_number == 2)
        success = retrieve_data();
    else if (test_number == 3)
        success = delete_data();
    // Si disconnette
    ASSERT_MESSAGE(os_disconnect() == 1, "Leaving connection", return 1);
    printf("Disconnesso\n");
    // Restituisce il successo dell'operazione performata
    return success;
}
