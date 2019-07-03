# Cartella in cui sono memorizzate le librerie
LIB = ./lib

# Compilatore e relative opzioni
CC = gcc -std=c99
CFLAGS = -Wall -Werror -pedantic -pedantic-errors -pthread -O3 -L $(LIB) -I $(LIB) \
#-fsanitize=undefined -fsanitize=leak -fsanitize=address -fno-omit-frame-pointer

# Creatore di librerie e relative opzioni
AR = ar
ARFLAGS = rvs

.PHONY: all clean test test-client test-server

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

# Test valgrind del server
test-server: objectstore
	if [ -S ./tmp.sock ]; then rm ./tmp.sock; fi
	#if [ -d ./data ]; then rm -rf ./data; fi
	./objectstore

# Test valgrind del client
test-client: client
	./client ugo 1
	./client ugo 2
	./client ugo 3

test: all
	./test.sh

clean:
	rm -rf ./lib/*/*.o ./lib/*.a ./objectstore ./client ./testout.log