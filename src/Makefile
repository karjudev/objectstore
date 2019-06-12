# Cartella in cui sono memorizzate le librerie
LIB = ./lib

# Compilatore e relative opzioni
CC = gcc -std=c99
CFLAGS = -g -Wall -Werror -pedantic -pthread -O3 -L $(LIB) -I $(LIB)

# Creatore di librerie e relative opzioni
AR = ar
ARFLAGS = rvs

$(LIB)/libsocket.a: $(LIB)/socket/socket.o
	$(AR) $(ARFLAGS) $@ $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@