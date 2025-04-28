// client_vectoriel.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define NB_PROCESSUS 4
#define BUFFER_SIZE 1024

int horloge[NB_PROCESSUS] = {0};
int idProcessus = 0;

void afficherHorloge() {
    printf("[Vectoriel %d] Horloge: [ ", idProcessus);
    for (int i = 0; i < NB_PROCESSUS; i++) {
        printf("%d ", horloge[i]);
    }
    printf("]\n");
}

void evenementLocal() {
    horloge[idProcessus]++;
    printf("[Vectoriel %d] Événement local.\n", idProcessus);
    afficherHorloge();
}

void envoyerMessage(int sock) {
    horloge[idProcessus]++;
    char buffer[BUFFER_SIZE] = {0};

    for (int i = 0; i < NB_PROCESSUS; i++) {
        char temp[10];
        sprintf(temp, "%d,", horloge[i]);
        strcat(buffer, temp);
    }

    send(sock, buffer, strlen(buffer), 0);
    printf("[Vectoriel %d] Message envoyé.\n", idProcessus);
    afficherHorloge();
}

void recevoirMessage(int sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int n = read(sock, buffer, sizeof(buffer) - 1);

    if (n > 0) {
        buffer[n] = '\0';
        int horloge_recue[NB_PROCESSUS];
        sscanf(buffer, "%d,%d,%d,%d,", &horloge_recue[0], &horloge_recue[1], &horloge_recue[2], &horloge_recue[3]);

        printf("[Vectoriel %d] Message reçu.\n", idProcessus);

        for (int i = 0; i < NB_PROCESSUS; i++) {
            horloge[i] = (horloge[i] > horloge_recue[i]) ? horloge[i] : horloge_recue[i];
        }

        horloge[idProcessus]++;
        afficherHorloge();
    }
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;

    printf("Entrez l'ID du processus (0-3) : ");
    scanf("%d", &idProcessus);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("[Client Vectoriel %d] Connecté au serveur.\n", idProcessus);

    for (int i = 0; i < 5; i++) {
        evenementLocal();
        sleep(1);
    }

    for (int i = 0; i < 4; i++) {
        envoyerMessage(sock);
        recevoirMessage(sock);
        sleep(2);
    }

    close(sock);
    return 0;
}
