// client_matriciel.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define CLIENT_COUNT 4
#define BUFFER_SIZE 4096

int horloge[CLIENT_COUNT][CLIENT_COUNT] = {0};
int idProcessus = 0;

int max(int a, int b) {
    return (a > b) ? a : b;
}

void afficherHorloge() {
    printf("[Matriciel %d] Horloge :\n", idProcessus);
    for (int i = 0; i < CLIENT_COUNT; i++) {
        for (int j = 0; j < CLIENT_COUNT; j++) {
            printf("%d ", horloge[i][j]);
        }
        printf("\n");
    }
}

void evenementLocal() {
    horloge[idProcessus][idProcessus]++;
    printf("[Matriciel %d] Événement local.\n", idProcessus);
    afficherHorloge();
}

void envoyerMessage(int sock) {
    horloge[idProcessus][idProcessus]++;

    char buffer[BUFFER_SIZE] = {0};
    int offset = 0;

    for (int i = 0; i < CLIENT_COUNT; i++) {
        for (int j = 0; j < CLIENT_COUNT; j++) {
            offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "%d,", horloge[i][j]);
        }
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, ";");
    }

    if (send(sock, buffer, strlen(buffer), 0) == -1) {
        perror("Erreur envoi");
        exit(EXIT_FAILURE);
    }

    printf("[Matriciel %d] Matrice envoyée.\n", idProcessus);
    afficherHorloge();
}

void recevoirMessage(int sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    
    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        int mat_recue[CLIENT_COUNT][CLIENT_COUNT] = {0};

        int i = 0, j = 0;
        char *saveptr1, *saveptr2;
        char *ligne = strtok_r(buffer, ";", &saveptr1);
        
        while (ligne != NULL && i < CLIENT_COUNT) {
            j = 0;
            char *valeur = strtok_r(ligne, ",", &saveptr2);
            while (valeur != NULL && j < CLIENT_COUNT) {
                mat_recue[i][j] = atoi(valeur);
                valeur = strtok_r(NULL, ",", &saveptr2);
                j++;
            }
            ligne = strtok_r(NULL, ";", &saveptr1);
            i++;
        }

        // Fusionner les matrices
        for (int a = 0; a < CLIENT_COUNT; a++) {
            for (int b = 0; b < CLIENT_COUNT; b++) {
                horloge[a][b] = max(horloge[a][b], mat_recue[a][b]);
            }
        }

        // Puis incrémenter sa propre diagonale
        horloge[idProcessus][idProcessus]++;
        printf("[Matriciel %d] Après réception.\n", idProcessus);
        afficherHorloge();
    } else if (n == 0) {
        printf("Le serveur a fermé la connexion.\n");
        close(sock);
        exit(EXIT_SUCCESS);
    } else {
        perror("Erreur réception");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;

    printf("Entrez l'ID du processus (0 à 3) : ");
    scanf("%d", &idProcessus);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur création socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Adresse IP invalide");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur connexion serveur");
        exit(EXIT_FAILURE);
    }

    printf("[Client Matriciel %d] Connecté au serveur.\n", idProcessus);

    for (int i = 0; i < 5; i++) {
        evenementLocal();
        sleep(1);
    }

    for (int i = 0; i < 4; i++) {
        envoyerMessage(sock);
        sleep(1);
        recevoirMessage(sock);
        sleep(2);
    }

    close(sock);
    return 0;
}
