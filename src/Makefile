# Cartella in cui sono memorizzate le librerie
LIB = ./lib

# Compilatore e relative opzioni
CC = gcc -std=c99
CFLAGS = -g -Wall -Werror -pedantic -pedantic-errors -pthread -O3 -L $(LIB) -I $(LIB)

# Creatore di librerie e relative opzioni
AR = ar
ARFLAGS = rvs

.PHONY: all test-server test

all: objectstore client

# Eseguibile del server
objectstore: objectstore.c $(LIB)/libsocket.a
	$(CC) $(CFLAGS) $< -o $@ -lsocket

# Eseguibile del client
client: client.c $(LIB)/libsocket.a $(LIB)/libosclient.a
	$(CC) $(CFLAGS) $< -o $@ -losclient -lsocket

# Libreria per la gestione dei socket
$(LIB)/libsocket.a: $(LIB)/socket/socket.o
	$(AR) $(ARFLAGS) $@ $<

# Libreria client
$(LIB)/libosclient.a: $(LIB)/osclient/osclient.o
	$(AR) $(ARFLAGS) $@ $<

# Pattern generico di compilazione di un file oggetto
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

# Test valgrind del server
test-server: objectstore
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./objectstore

# Test valgrind del client
test-client: client
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client