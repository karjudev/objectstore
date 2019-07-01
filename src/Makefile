# Cartella in cui sono memorizzate le librerie
LIB = ./lib

# Compilatore e relative opzioni
CC = gcc -std=c99
CFLAGS = -g -Wall -Werror -pedantic -pedantic-errors -pthread -O3 -L $(LIB) -I $(LIB)

# Creatore di librerie e relative opzioni
AR = ar
ARFLAGS = rvs

.PHONY: all clean test test-client test-server

all: objectstore client

# Eseguibile del server
objectstore: objectstore.c $(LIB)/libsocket.a $(LIB)/libhashtable.a $(LIB)/libworkers.a
	$(CC) $(CFLAGS) $< -o $@ -lworkers -lhashtable -lsocket

# Eseguibile del client
client: client.c $(LIB)/libsocket.a $(LIB)/libosclient.a
	$(CC) $(CFLAGS) $< -o $@ -losclient -lsocket

testhash: testhash.c $(LIB)/libhashtable.a
	$(CC) $(CFLAGS) $< -o $@ -lhashtable

# Libreria per la gestione di una tabella hash
$(LIB)/libhashtable.a: $(LIB)/hashtable/hashtable.o $(LIB)/hashtable/pair_list.o
	$(AR) $(ARFLAGS) $@ $^

# Libreria per la gestione dei socket
$(LIB)/libsocket.a: $(LIB)/socket/safeio.o $(LIB)/socket/socket.o
	$(AR) $(ARFLAGS) $@ $^

# Libreria client
$(LIB)/libosclient.a: $(LIB)/osclient/osclient.o
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
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./objectstore

# Test valgrind del client
test-client: client
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client ugo 1
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client ugo 2
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client ugo 3

test: all
	./objectstore