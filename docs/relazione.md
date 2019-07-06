# Relazione progetto SOL A.A 2018/19
## Giacomo Mariani, matricola 545519

Il progetto consta di due eseguibili `objstore` e `client`. Per avviare il test bisogna entrare nella cartella contenente il `Makefile` e digitare:
```
$ make test
```
In questo modo verrà avviato il server, dopodiché sarà avviato lo script `test.sh` e infine, al termine del primo script, sarà avviato `testsum.sh`, che raccoglie e analizza i dati di test, invia il segnale di stampa delle statistiche e infine termina il server.

## Scelte implementative

### Messaggi di errore

Dato che tutte le funzioni che ho scritto sia nel server che nel client, in caso di errore settano opportunamente la variabile `errno`, per favorire la brevità e per fare in modo che i messaggi abbiano dimensione fissa. Il client interpreta questo intero e stampa il messaggio associato ad esso sullo standard error usando la funzione `strerror`.

### Protocollo di comunicazione

Tutte le funzioni che agiscono sui socket usano per comunicare le funzioni fornite `readn` e `writen`. Ogni header contenente un comando viene inviato dal client al server con una dimensione fissa di 267 byte, calcolati sommando i seguenti campi:

- Dimensione del verbo di lunghezza maggiore: `strlen("RETRIEVE") = 8`
- Dimensione del nome di blocco di lunghezza maggiore, ovvero massima dimensione di un file POSIX: 255
- 2 spazi + `\n\0`: 4

In modo analogo è calcolato il numero di bytes restituiti nella risposta:

- Dimensione di `"KO"`: 2
- Codice di errore a 2 cifre: 2
- 2 spazi + `\n\0`: 4

Questi "magic values" sono contenuti insieme a tutti i valori condivisi tra client e server, in `lib/shared.h`.

### Dati di prova

Come blocchi di dati di prova, il client invia al server una serie di array di byte (`unsigned char`) contenenti numeri progressivi `0, 1, 2, ...`. I dati mandati dal client sono 20 pacchetti di dimensione crescente da 10 a 100000 byte, che il client spedisce creando localmente una volta per tutte un array di 100000 byte definito come sopra, e inviando pezzi dell'array di dimensione crescente.

### Tabella hash

Invece di utilizzare la tabella fornita ho scelto di implementare da zero una tabella hash che mappa chiavi di tipo intero e valori di tipo stringa, in modo da associare ad ogni file descriptor di ogni utente attualmente connesso il suo nome simbolico all'interno del sistema. Il codice della tabella è contenuto in `lib/hashtable`, e si articola in due componenti:
- `pair_list` è una libreria che permette di creare e manipolare una lista di coppie (intero, stringa), implementata come una lista linkata.
- `hashtable` è la vera e propria tabella hash: un array di liste di trabocco implementate come `pair_list`, che usa una semplice funzione hash basata su moltiplicazione e somma di numeri primi. La tabella è acceduta in mutua esclusione, dato che contiene un array di 256 mutex che partizionano le 1024 caselle dell'array, per raggiungere un giusto compromesso tra serializzazione e parallelizzazione delle operazioni sulla tabella.

### Lista di thread

Oltre alla lista di coppie è presente anche una libreria `pthread_list`, che permette di inserire e rimuovere `pthread_t` dalla testa di una lista concatenata. La libreria viene usata dal server per tenere traccia dei thread creati dallo stesso, e alla fine dell'esecuzione viene distrutta man mano che i thread corrispondenti agli identificativi vengono terminati con una `pthread_join`.

## Struttura del codice

Ho cercato per quanto possibile di astrarre le operazioni che il server deve svolgere in librerie quanto più autonome possibile.

- `objectstore.c`: Compila l'eseguibile del server. Contiene i metodi che si occupano di creare un nuovo thread per ogni connessione, i quali ricevono continuamente header da un client ed eseguono le operazioni associate ad essi. Alla ricezione di una "LEAVE \n" un thread termina, chiudendo la connessione e liberando le risorse. Il server maschera i segnali `SIGINT`, `SIGTERM`, `SIGQUIT`, `SIGUSR1` e usa un thread apposito che attende l'arrivo di questi segnali con `sigwait`. In caso di `SIGUSR1` viene stampato il report, altrimenti viene settata una variabile globale che fa terminare tutti i thread attivi, dopodiché dealloca la memoria del processo.
- `client.c`: Compila l'eseguibile del client. Contiene i metodi per effettuare i tre test richiesti dalla specifica. All'accesso si collega al file descriptor del server e si registra con il nome passato come primo parametro. Dopodiché esegue uno dei tre test dati nella specifica, associati al numero da 1 a 3 passato come secondo parametro.
- `socket.c`: Libreria che contiene i metodi atti a creare socket `AF_UNIX` sia lato client che server, a distruggerli e ad attendere o instaurare connessioni su di essi. In particolare, il metodo `accept_new_client` fa uso di una `select` con timeout fissato ad un secondo, in modo tale che se non arriva nessun client entro questo intervallo è possibile al chiamante venire notificato dell'arrivo di segnali di varia natura.
- `workers.c`: Libreria che contiene le funzioni del server. Si occupa di interagire con il disco creando lo spazio (la directory) di un utente, e recuperando, eliminando o memorizzando file dentro questo spazio. La libreria mantiene, come variabile globale interna, una tabella hash, e tutte le funzioni si preoccupano di mantenere lo stato della tabella consistente rispetto a quello del disco dall'avvio del programma in poi.
- `os_client.c`: Libreria client che interagisce con il server rispettando il protocollo di comunicazione dato.
- `hashtable.c`: Libreria della tabella hash, per approfondire vedere il paragrafo apposito.
- `pthread_list.c`: Libreria della lista di thread, come sopra.

In aggiunta sono presenti due header files che forniscono delle macro utilizzate per gestire gli errori nel codice:
- `assertmacros.h`: Macro che permettono di modificare il flusso di esecuzione del codice tramite la verifica di asserzioni. In caso di asserzioni false è possibile eseguire operazioni, restituire valori e settare opportunamente errno.
- `mutexmacros.h`: Macro che permettono di acquisire e rilasciare lock e, in caso di errore, eseguire un'operazione.

## Sistemi operativi usati per il testing

- elementaryOS 5.0 Juno
- Xubuntu 14.10 (Macchina virtuale)