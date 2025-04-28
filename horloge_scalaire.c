// client_scalaire.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int horloge = 0;

int max(int a, int b) {
    return (a > b) ? a : b;
}

void evenementLocal() {
    horloge++;
    printf("[Scalaire] Événement local. Horloge = %d\n", horloge);
}

void envoyerMessage(int sock) {
    horloge++;
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%d", horloge);
    send(sock, buffer, strlen(buffer), 0);
    printf("[Scalaire] Message envoyé (horloge = %d)\n", horloge);
}

void recevoirMessage(int sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    int n = read(sock, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        int horlogeRecue = atoi(buffer);

        // Mise à jour correcte de l'horloge scalaire
        if (horloge < horlogeRecue) {
            horloge= horlogeRecue;
        }
        horloge += 1;

        printf("[Scalaire] Après réception, horloge = %d\n", horloge);
    }
}
int main() {
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("[Client Scalaire] Connecté au serveur.\n");

    for (int i = 0; i < 5; i++) {
        evenementLocal();
        sleep(1);
    }

    for (int i = 0; i < 4; i++) {
        envoyerMessage(sock);
        sleep(1);
        recevoirMessage(sock);
        sleep(1);
    }

    close(sock);
    return 0;
}
