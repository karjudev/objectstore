# Cartella in cui sono memorizzate le librerie
LIB = ./lib

# Compilatore e relative opzioni
CC = gcc -std=c99
CFLAGS = -Wall -Werror -pedantic -pedantic-errors -pthread -O3 -L $(LIB) -I $(LIB)

# Creatore di librerie e relative opzioni
AR = ar
ARFLAGS = rvs

.PHONY: all clean test

all: objectstore client

# Eseguibile del server
objectstore: objectstore.c $(LIB)/libsocket.a $(LIB)/libhashtable.a $(LIB)/libworkers.a $(LIB)/libpthreadlist.a
	$(CC) $(CFLAGS) $< -o $@ -lpthreadlist -lworkers -lhashtable -lsocket

# Eseguibile del client
client: client.c $(LIB)/libsocket.a $(LIB)/libosclient.a
	$(CC) $(CFLAGS) $< -o $@ -losclient -lsocket

testhash: testhash.c $(LIB)/libhashtable.a
	$(CC) $(CFLAGS) $< -o $@ -lhashtable

# Libreria per la gestione di una tabella hash
$(LIB)/libhashtable.a: $(LIB)/hashtable/hashtable.o $(LIB)/hashtable/pair_list.o
	$(AR) $(ARFLAGS) $@ $^

# Libreria per la gestione di una lista di pthread_t
$(LIB)/libpthreadlist.a: $(LIB)/pthread_list/pthread_list.o
	$(AR) $(ARFLAGS) $@ $^

# Libreria per la gestione dei socket
$(LIB)/libsocket.a: $(LIB)/socket/safeio.o $(LIB)/socket/socket.o
	$(AR) $(ARFLAGS) $@ $^

# Libreria client
$(LIB)/libosclient.a: $(LIB)/os_client/os_client.o
	$(AR) $(ARFLAGS) $@ $^

# Libreria che esegue le funzioni che il server offre al client
$(LIB)/libworkers.a: $(LIB)/workers/workers.o
	$(AR) $(ARFLAGS) $@ $<

# Pattern generico di compilazione di un file oggetto
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

# Target che avvia il server e gli script di test
test: all
	./objectstore &
	./test.sh
	./testsum.sh

clean:
	rm -rf ./data ./tmp.sock ./lib/*/*.o ./lib/*.a ./objectstore ./client ./testout.log
