#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define IP_SERVEUR "127.0.0.1"
#define PORT 8080
#define NB_PROCESSUS 4
#define TAILLE_BUFFER 2048

int horloge[NB_PROCESSUS][NB_PROCESSUS] = {0};
int id_processus = 0;

/* Initialise l'horloge matricielle */
void initialiser_horloge() {
    memset(horloge, 0, sizeof(horloge));
    horloge[id_processus][id_processus] = 0;
    printf("[Processus %d] Horloge initialisée\n", id_processus);
}

/* Affiche l'état courant de l'horloge */
void afficher_horloge() {
    printf("\n[Processus %d] État de l'horloge:\n", id_processus);
    for (int i = 0; i < NB_PROCESSUS; i++) {
        printf("P%d: [", i);
        for (int j = 0; j < NB_PROCESSUS; j++) {
            printf("%2d ", horloge[i][j]);
        }
        printf("]\n");
    }
}

/* Exécute un événement local */
void evenement_local() {
    horloge[id_processus][id_processus]++;
    printf("\n[Processus %d] Exécution événement local\n", id_processus);
    afficher_horloge();
}

void mettre_a_jour(int recue[NB_PROCESSUS][NB_PROCESSUS]) {
    printf("\n[Processus %d] Mise à jour à partir des données reçues:\n", id_processus);
    
    for (int i = 0; i < NB_PROCESSUS; i++) {
        for (int j = 0; j < NB_PROCESSUS; j++) {
            if (recue[i][j] > horloge[i][j]) {
                printf("Mise à jour P%d[%d]: %d → %d\n", i, j, horloge[i][j], recue[i][j]);
                horloge[i][j] = recue[i][j];
            }
        }
    }

    // *** Incrémenter l'horloge du processus récepteur pour l'événement de réception ***
    horloge[id_processus][id_processus]++;

    afficher_horloge();
}


/* Envoie l'horloge courante au serveur */
void envoyer_horloge(int sock) {
    horloge[id_processus][id_processus]++; // Incrément avant envoi
    
    char buffer[TAILLE_BUFFER] = {0};
    char temp[20];
    
    // Format: "MATRICE|id|valeur,valeur,...;..."
    strcat(buffer, "MATRICE|");
    sprintf(temp, "%d|", id_processus);
    strcat(buffer, temp);
    
    for (int i = 0; i < NB_PROCESSUS; i++) {
        for (int j = 0; j < NB_PROCESSUS; j++) {
            sprintf(temp, "%d,", horloge[i][j]);
            strcat(buffer, temp);
        }
        strcat(buffer, ";");
    }
    
    if (send(sock, buffer, strlen(buffer), 0) < 0) {
        perror("Erreur envoi");
        exit(EXIT_FAILURE);
    }
    
    printf("\n[Processus %d] Horloge envoyée\n", id_processus);
    afficher_horloge();
}

/* Reçoit et traite une horloge */
void recevoir_horloge(int sock) {
    char buffer[TAILLE_BUFFER] = {0};
    int octets = recv(sock, buffer, TAILLE_BUFFER-1, 0);
    
    if (octets <= 0) {
        if (octets == 0) {
            printf("Connexion fermée par le serveur\n");
        } else {
            perror("Erreur réception");
        }
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    buffer[octets] = '\0';
    
    if (strncmp(buffer, "MATRICE|", 8) != 0) {
        fprintf(stderr, "Message invalide reçu\n");
        return;
    }
    
    int recue[NB_PROCESSUS][NB_PROCESSUS] = {0};
    char *ptr = buffer + 8;
    int expediteur = atoi(ptr);
    ptr = strchr(ptr, '|') + 1;
    
    printf("\nReçu du processus %d:\n", expediteur);
    
    for (int i = 0; i < NB_PROCESSUS; i++) {
        char *ligne = strtok_r(ptr, ";", &ptr);
        if (!ligne) break;
        
        for (int j = 0; j < NB_PROCESSUS; j++) {
            char *val = strtok_r(ligne, ",", &ligne);
            if (!val) break;
            recue[i][j] = atoi(val);
        }
    }
    
    mettre_a_jour(recue);
}

int main() {
    printf("Entrez l'ID du processus (0-%d): ", NB_PROCESSUS-1);
    scanf("%d", &id_processus);
    getchar();
    
    if (id_processus < 0 || id_processus >= NB_PROCESSUS) {
        fprintf(stderr, "ID invalide\n");
        return 1;
    }
    
    initialiser_horloge();
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveur_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };
    inet_pton(AF_INET, IP_SERVEUR, &serveur_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&serveur_addr, sizeof(serveur_addr)) < 0) {
        perror("Échec connexion");
        return 1;
    }
    
    printf("[Processus %d] Connecté au serveur\n", id_processus);
    
    /* Séquence de 5 événements locaux */
    for (int i = 1; i <= 5; i++) {
        sleep(1);
        evenement_local();
    }
    
    /* Séquence de 4 émissions/réceptions */
    for (int i = 1; i <= 4; i++) {
        sleep(1);
        printf("\n--- Échange %d ---\n", i);
        envoyer_horloge(sock);
        recevoir_horloge(sock);
    }
    
    close(sock);
    printf("\n[Processus %d] État final:\n", id_processus);
    afficher_horloge();
    
    return 0;
}
