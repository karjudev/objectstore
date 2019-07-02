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
 */
static void store_data () {
    // Variabile che contiene il nome del blocco
    char name[2] = "A";
    // Buffer da 100KB
    byte* array = create_test_array(100000);
    ASSERT_MESSAGE(array != NULL, "Allocating test array", return);
    // Invia 20 buffer in ordine
    int step = (100000 - 100) / 19;
    for (size_t size = 100; size <= 100000; size += step) {
        ASSERT_MESSAGE(os_store(name, array, size) == 1, "Storing message", free(array); return);
        printf("Stored block %s of size %ld\n", name, size);
        name[0]++;
    }
    // Libera la memoria occupata dall'array
    free(array);
}

/**
 * @brief Recupera 20 blocchi di dati da 100B a 100KB e verifica che siano una sequenza di interi consecutivi
 */
static void retrieve_data () {
    // Byte array di prova
    byte* array = create_test_array(100000);
    // Step con cui aumenta la dimensione dei dati
    int step = (100000 - 100) / 19; 
    // Nome della risorsa
    char name[2] = "A";
    for (size_t size = 100; size <= 100000; size += step) {
        // Richiede i dati al server
        byte* data = os_retrieve(name);
        ASSERT_MESSAGE(data != NULL, "Getting data", free(data); free(array); return);
        // Verifica che i dati siano uguali
        ASSERT_MESSAGE(data_corresponding(data, array, size), "Data are not corresponding", free(data); free(array); return);
        // Libera la memoria occupata dai dati appena ricevuti
        free(data);
        // Stampa un messaggio di log
        printf("Successifully retrieved block %s of size %ld\n", name, size);
        // Cambia il nome
        name[0]++;
    }
    // Libera la memoria occupata dall'array di prova
    free(array);
}

/**
 * @brief Cancella 20 blocchi di dati!= -1
 */
static void delete_data () {
    // Nome del blocco
    char name[2] = "A";
    for (int i = 0; i < 20; i++) {
        // Cancella il blocco
        ASSERT_MESSAGE(os_delete(name) == 1, "Deleting data", return);
        // Stampa un messaggio di log
        printf("Deleted file %s\n", name);
        // Incrementa il nome
        name[0]++;
    }
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
    if (test_number == 1)
        store_data();
    else if (test_number == 2)
        retrieve_data();
    else if (test_number == 3)
        delete_data();
    // Si disconnette
    ASSERT_MESSAGE(os_disconnect() == 1, "Leaving connection", return 1);
    printf("Disconnesso\n");
    return 0;
}
