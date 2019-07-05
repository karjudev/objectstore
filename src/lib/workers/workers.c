#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <ftw.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <assertmacros.h>

#include <shared.h>

#include <socket/safeio.h>

#include <hashtable/hashtable.h>
#include <workers/workers.h>

// Tabella hash in cui memorizzare le coppie (username, file descriptor)
static hashtable_t* table;
// Numero totale di oggetti nello store
static int objects_count;
// Dimensione totale dello store
static size_t total_size;

static size_t get_file_size (char* file_name) {
    struct stat sb = {0};
    int success = stat(file_name, &sb);
    ASSERT_RETURN(success != -1, -1);
    return sb.st_size;
}

/**
 * @brief Se la cartella esiste la cancella, poi la ricrea
 * 
 * @param name 
 * @return int 
 */
static int create_directory_if_not_exists (char* name) {
    struct stat sb = {0};
    if (!(stat(name, &sb) == 0 && S_ISDIR(sb.st_mode))) {
        return mkdir(name, 0777);
    }
    return 0;
}

/**
 * @brief Crea un percorso composto da DATA_DIRECTORY/directory[/filename].
 * 
 * @param directory Nome della directory
 * @param filename Opzionale nome del file
 * @return char* Percorso UNIX creato come specificato. Se c'è un errore restituisce NULL e setta errno.
 */
static char* create_path (char* directory, char* filename) {
    // Alloca il percorso della dimensione adatta
    size_t length = strlen(DATA_DIRECTORY) + strlen(directory) + 2;
    if (filename) length += strlen(filename) + 1;
    char* path = (char*) malloc(sizeof(char) * length);
    ASSERT_ERRNO_RETURN(path != NULL, ENOMEM, NULL);
    // Costruisce il percorso
    if (filename)
        sprintf(path, "%s/%s/%s", DATA_DIRECTORY, directory, filename);
    else
        sprintf(path, "%s/%s", DATA_DIRECTORY, directory);
    // Restituisce il percorso
    return path;
}

/**
 * @brief Inizializza le strutture dati necessarie alle funziioni.
 * 
 * @return int Se l'inizializzazione è andata a buon fine restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int init_worker_functions () {
    // Crea la cartella dati se non esiste
    int success = create_directory_if_not_exists("data");
    ASSERT_RETURN(success == 0, -1);
    // Inizializza la tabella hash
    table = create_hashtable();
    ASSERT_RETURN(table != NULL, -1);
    // Restituisce il successohashtable
    return 0;
}

/**
 * @brief Libera la memoria occupata dalle strutture dati necessarie alle funzioni.
 * 
 * @return int Se l'eliminazione è andata a buon fine restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int stop_worker_functions () {
    // Elimina la tabella hash
    return destroy_hashtable(table);
}

/**
 * @brief Registra un nuovo utente creando la sua cartella su disco
 * 
 * @param client_fd File descriptor del client
 * @param name Nome utente del client
 * @return int Se il client è stato registrato correttamente restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int register_user (int client_fd, char* name) {
    // Crea la cartella dell'utente se questa non esiste già
    char* path = create_path(name, NULL);
    ASSERT_RETURN(path != NULL, -1);
    int success = create_directory_if_not_exists(path);
    free(path);
    // Controlla che la creazione sia avvenuta con successo
    ASSERT_RETURN(success != -1, -1);
    // Inserisce l'utente nella tabella
    success = insert_hashtable(table, client_fd, name);
    // Controlla che non ci siano errori
    ASSERT_RETURN(success != -1, -1);
    // Termina con successo
    return 0;
}

/**
 * @brief Scrive un nuovo blocco nel file con lo stesso nome.
 * 
 * @param client_fd File descriptor del client
 * @param name Nome del blocco da scrivere
 * @param data Blocco di dati da scrivere
 * @param size Dimensione dei dati
 * @return int Se il blocco è stato scritto correttamente restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int store_block (int client_fd, char* name, void* data, size_t size) {
    // Prende lo username dell'utente
    char* username = retrieve_hashtable(table, client_fd);
    // Se lo username non esiste esce
    ASSERT_RETURN(username != NULL, -1);
    // Crea il percorso del file
    char* path = create_path(username, name);
    ASSERT_RETURN(path != NULL, -1);
    // Apre il file da leggere
    int file_fd = open(path, O_CREAT | O_WRONLY, 0777);
    free(path);
    ASSERT_RETURN(file_fd != -1, -1);
    // Scrive tutti i bytes sul file
    int bytes_written = writen(file_fd, data, size);
    // Chiude il file da leggere
    int success = close(file_fd);
    ASSERT_RETURN(success != -1, -1);
    // Controlla che il file sia stato scritto correttamente
    ASSERT_RETURN(bytes_written != -1, -1);
    // Resituisce il succeso
    return 0;
}

/**
 * @brief Recupera un blocco di dati
 * 
 * @param client_fd File descriptor del client
 * @param name Nome del blocco da recuperare
 * @param size_ptr Puntatore alla dimensione del blocco, il cui valore puntato viene settato al termine della funzione
 * @return void* Blocco di dati identificato dal nome. Se c'è un errore restituisce NULL e setta errno.
 */
void* retrieve_block (int client_fd, char* name, size_t* size_ptr) {
    // Controlla la correttezza dei parametri
    ASSERT_ERRNO_RETURN((client_fd > 0) && (name != NULL), EINVAL, NULL);
    // Prende lo username dell'utente
    char* username = retrieve_hashtable(table, client_fd);
    // Se lo username non esiste esce
    ASSERT_RETURN(username != NULL, NULL);
    // Costruisce il percorso del file
    char* path = create_path(username, name);
    ASSERT_RETURN(path != NULL, NULL);
    // Crea un buffer grande quanto il file
    size_t size = get_file_size(path);
    ASSERT(size != -1, free(path); return NULL);
    void* buffer = malloc(size);
    ASSERT_ERRNO_RETURN(buffer != NULL, ENOMEM, NULL);
    // Apre il file puntato
    int file_fd = open(path, O_RDONLY);
    free(path);
    ASSERT_RETURN(file_fd != -1, NULL);
    // Legge il contenuto del file
    int bytes_read = readn(file_fd, buffer, size);
    // Chiude il file
    int success = close(file_fd);
    // Verifica che lettura e chiusura siano andate a buon fine
    ASSERT((bytes_read != -1) && (success != -1), free(buffer); return NULL);
    // Setta il valore del puntatore alla dimensione
    *size_ptr = size;
    // Restituisce il buffer
    return buffer;
}

/**
 * @brief Rimuove dal disco un blocco di dati dell'utente
 * 
 * @param client_fd File descriptor dell'utente
 * @param name Nome del blocco da rimuovere
 * @return int Se il blocco è stato rimosso con successo restituisce 0. Se c'è un errore restituisce -1 e setta errno.
 */
int delete_block (int client_fd, char* name) {
    // Recupera lo username dell'utente
    char* username = retrieve_hashtable(table, client_fd);
    ASSERT_RETURN(username != NULL, -1);
    // Costruisce il path del file
    char* path = create_path(username, name);
    // Rimuove il file identificato dal path
    int success = unlink(path);
    // Libera la memoria occupata dal percorso
    free(path);
    // L'operazione ha avuto successo se l'eliminazione ha avuto successo
    return success;
}

/**
 * @brief Cancella il client dal sistema
 * 
 * @param client_fd File descriptor di client
 */
void leave_client (int client_fd) {
    // Rimuove se esiste il descrittore dalla tabella hash
    remove_hashtable(table, client_fd);
}

static int count_file_number_size (const char* filename, const struct stat* sb, int typeflag) {
    // Se l'elemento corrente è un file incrementa il numero di file
    if (typeflag == FTW_F)
        objects_count++;
    // Incrementa la dimensione totale
    total_size += sb->st_size;
    // Restituisce il successo
    return 0;
}

/**
 * @brief Scrive le informazioni di report sui puntatori passati
 * 
 * @param clients_ptr Puntatore al numero di client connessi
 * @param objects_ptr Puntatore al numero di oggetti salvati
 * @param size_ptr Puntatore alla dimensione totale dello store
 * @return int Se le informazioni sono state estratte con successo restituisce 0. Se c'è un errore restituisce -1 e setta errno. 
 */
int get_report (int* clients_ptr, int* objects_ptr, int* size_ptr) {
    // Conta il numero di files e dimensione totale
    total_size = 0;
    objects_count = 0;
    int success = ftw("./data", count_file_number_size, 0);
    ASSERT_RETURN(success != -1, -1);
    *size_ptr = total_size;
    *objects_ptr = objects_count;
    // Conta il numero di client connessi
    *clients_ptr = table->elements;
    // Restituisce il successo
    return 0;
}