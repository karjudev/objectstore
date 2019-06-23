#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assertmacros.h>

#include <socket/socket.h>
#include <osclient/osclient.h>

#include <shared.h>

int main(int argc, char *argv[]) {
    // Si connette al server
    ASSERT_MESSAGE(os_connect("ugo") == 1, "Connecting to socket", exit(1));
    printf("Connesso\n");
    // Crea il messaggio
    int buffer[10];
    for (int i = 0; i < 10; i++) buffer[i] = i;
    // Invia il messaggio
    ASSERT_MESSAGE(os_store("pippo", buffer, sizeof(buffer)) == 1, "Storing data", exit(1));
    printf("STORE Completata\n");
    // Legge la risposta
    int* data = (int*) os_retrieve("pippo");
    ASSERT_MESSAGE(data != NULL, "Retrieving message", exit(1));
    for (int i = 0; i < 10; i++)
        ASSERT_MESSAGE(buffer[i] == data[i], "Confronting data", exit(1));
    free(data);
    printf("Dati corretti\n");
    // Elimina i dati
    ASSERT_MESSAGE(os_delete("pippo") == 1, "Deleting data", exit(1));
    // Si disconnette
    ASSERT_MESSAGE(os_disconnect() == 1, "Leaving connection", exit(1));
    printf("Disconnesso\n");
}
